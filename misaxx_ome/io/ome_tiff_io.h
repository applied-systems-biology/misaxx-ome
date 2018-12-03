//
// Created by rgerst on 23.11.18.
//


#pragma once

#include <memory>
#include <shared_mutex>
#include <unordered_set>
#include <ome/files/out/OMETIFFWriter.h>
#include <ome/files/in/OMETIFFReader.h>
#include <ome/xml/meta/Convert.h>
#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include <cxxh/cache.h>
#include <cxxh/string.h>
#include <boost/regex.hpp>
#include "ome_to_opencv.h"
#include "opencv_to_ome.h"
#include "ome_to_ome.h"
#include <misaxx_ome/helpers/ome_helpers.h>

namespace misaxx_ome {

    /**
     * Allows thread-safe read and write access to an OME TIFF
     * This wrapper will automatically switch between an OME TIFF reader and an OME TIFF writer depending on what functionality
     * is currently being requested.
     *
     * Please note that this IO, similar to ome::files TIFF reader & writer needs to be closed manually
     */
    class ome_tiff_io {
    public:

        using locked_reader_type = cxxh::locked<std::shared_ptr<ome::files::in::OMETIFFReader>, std::shared_lock<std::shared_mutex>>;
        using locked_writer_type = cxxh::locked<std::shared_ptr<ome::files::out::OMETIFFWriter>, std::unique_lock<std::shared_mutex>>;

        ome_tiff_io() = default;

        /**
         * Opens an existing OME TIFF file
         * @param t_path
         */
        explicit ome_tiff_io(boost::filesystem::path t_path) : m_path(std::move(t_path)) {
            if (!boost::filesystem::is_regular_file(m_path)) {
                throw std::runtime_error("Cannot read from non-existing file " + m_path.string());
            }
        }

        /**
         *  Opens an existing OME TIFF file or creates a new one based on the metadata
         *  If the file already exists, the metadata is loaded from the file instead.
         * @param t_path
         * @param t_metadata
         */
        explicit ome_tiff_io(boost::filesystem::path t_path,
                                     std::shared_ptr<ome::xml::meta::OMEXMLMetadata> t_metadata) : m_path(
                std::move(t_path)), m_metadata(std::move(t_metadata)) {
            // We can load metadata from the file if it exists
            if (boost::filesystem::is_regular_file(m_path)) {
                m_metadata.reset();
            }
        }

        /**
         * Opens an existing OME TIFF file or creates a new one based on the reference
         * @param t_path
         * @param t_reference
         */
        explicit ome_tiff_io(boost::filesystem::path t_path, const ome_tiff_io &t_reference)
                : ome_tiff_io(std::move(t_path), t_reference.get_metadata()) {
        }

        void write_plane(const cv::Mat &image, const misa_ome_plane_location &index) {
            if(index.series != 0)
                throw std::runtime_error("Only series 0 is currently supported!");
            auto writer = get_writer(index);
            opencv_to_ome(image, *writer.value, misa_ome_plane_location(0, 0, 0, 0));
            writer.value->close();
        }

        cv::Mat read_plane(const misa_ome_plane_location &index) const {
            if(index.series != 0)
                throw std::runtime_error("Only series 0 is currently supported!");
            auto reader = get_reader();
            return ome_to_opencv(*reader.value, index);
        }

        /**
         * Thread-safe access to the metadata
         * @return
         */
        std::shared_ptr<ome::xml::meta::OMEXMLMetadata> get_metadata() const {
            if(static_cast<bool>(m_metadata)) {
                return m_metadata;
            }
            else {
                // We are currently reading a file. Open it and fetch the metadata
                get_reader();
                return m_metadata;
            }
        }

        boost::filesystem::path get_path() const {
            return m_path;
        }

        /**
         * Thread-safe access to the managed reader
         * @return
         */
        locked_reader_type get_reader() const {
            std::shared_lock<std::shared_mutex> lock(m_mutex, std::defer_lock);
            lock.lock();
            if(static_cast<bool>(m_reader)) {
                return locked_reader_type(m_reader, std::move(lock));
            }
            else {
                lock.unlock();
                std::unique_lock<std::shared_mutex> wlock(m_mutex, std::defer_lock);
                wlock.lock();
                if(!m_write_buffer.empty()) {
                    close_writer();
                }
                open_reader();
                wlock.unlock();

                // Return the locked reader
                lock.lock();
                return locked_reader_type(m_reader, std::move(lock));
            }
        }

        /**
        * Thread-safe access to the managed writer
        * The writer is unique-locked (sequential access!)
        * @return
        */
        locked_writer_type get_writer(const misa_ome_plane_location &t_location) const {
            std::unique_lock<std::shared_mutex> lock(m_mutex, std::defer_lock);
            lock.lock();

            // If the current TIFF file exists, set it to a reader->writer connection
            // This will preserve the TIFF data that is already written
            if(boost::filesystem::is_regular_file(m_path) && m_write_buffer.empty()) {
                if(!static_cast<bool>(m_reader)) {
                    open_reader();
                }
            }

            if(static_cast<bool>(m_reader)) {
                 initialize_write_buffer_from_reader();
                 close_reader();
            }

            return locked_writer_type(get_buffer_writer(t_location), std::move(lock));
        }

        /**
         * Closes any open reader and writer. This method is thread-safe.
         */
        void close() {
            std::unique_lock<std::shared_mutex> wlock(m_mutex, std::defer_lock);
            wlock.lock();
            if(static_cast<bool>(m_reader)) {
                close_reader();
            }
            if(!m_write_buffer.empty()) {
                close_writer();
            }
        }

        /**
         * The number of image series
         * @return
         */
        ome::files::dimension_size_type get_num_series() const {
            return get_metadata()->getImageCount();
        }

        /**
         * The width of each plane
         * @param series
         * @return
         */
        ome::files::dimension_size_type get_size_x(ome::files::dimension_size_type series) const {
            return get_metadata()->getPixelsSizeX(series);
        }

        /**
         * The height of each plane
         * @param series
         * @return
         */
        ome::files::dimension_size_type get_size_y(ome::files::dimension_size_type series) const {
            return get_metadata()->getPixelsSizeY(series);
        }

        /**
         * Planes located in depth axis
         * @param series
         * @return
         */
        ome::files::dimension_size_type get_size_z(ome::files::dimension_size_type series) const {
            return get_metadata()->getPixelsSizeZ(series);
        }

        /**
         * Planes located in time axis
         * @param series
         * @return
         */
        ome::files::dimension_size_type get_size_t(ome::files::dimension_size_type series) const {
            return get_metadata()->getPixelsSizeT(series);
        }

        /**
         * Planes located in channel axis (this is the same as OME's effectiveSizeC)
         * @param series
         * @return
         */
        ome::files::dimension_size_type get_size_c(ome::files::dimension_size_type series) const {
            return get_metadata()->getChannelCount(series);
        }

        /**
         * Number of planes (Z * C * T)
         * @param series
         * @return
         */
        ome::files::dimension_size_type get_num_planes(ome::files::dimension_size_type series) const {
            return get_size_c(series) * get_size_t(series) * get_size_z(series);
        }

    private:

        /**
         * Path of the TIFF that is read / written
         */
        mutable boost::filesystem::path m_path;

        /**
         * Because of limitations to OMETIFFWriter, we buffer any output TIFF in a separate directory
         */
        mutable std::map<misa_ome_plane_location, boost::filesystem::path> m_write_buffer;

        mutable std::shared_ptr<ome::files::in::OMETIFFReader> m_reader;
        mutable std::shared_ptr<ome::xml::meta::OMEXMLMetadata> m_metadata;
        mutable std::shared_mutex m_mutex;

        /**
         * Returns the filename of the path without .ome.tif extension
         * @return
         */
        boost::filesystem::path get_base_filename() const {
            auto base_name = m_path.filename();
            for(int i = 0; i < 2 && base_name.has_extension(); ++i) {
                base_name.replace_extension();
            }
            return base_name;
        }

        void open_reader() const {
            m_reader = std::make_shared<ome::files::in::OMETIFFReader>();
            m_reader->setMetadataFiltered(false);
            m_reader->setGroupFiles(true);
            m_reader->setId(m_path);

            if(!static_cast<bool>(m_metadata)) {
               m_metadata = ome::files::createOMEXMLMetadata(*m_reader);
            }
        }

        void close_reader() const {
            m_reader->close();
            m_reader.reset();
        }

        void close_writer() const {
            std::cout << "[MISA++ OME] Writing results as OME TIFF " << m_path << " ... " << std::endl;
            // Save the write buffer files into the path
            auto writer = std::make_shared<ome::files::out::OMETIFFWriter>();
            auto metadata = std::static_pointer_cast<ome::xml::meta::MetadataRetrieve>(m_metadata);
            writer->setMetadataRetrieve(metadata);
            writer->setInterleaved(false);
            writer->setId(m_path);

            auto reader = std::make_shared<ome::files::in::OMETIFFReader>();
            reader->setMetadataFiltered(false);
            reader->setGroupFiles(true);

            for(const auto &kv : m_write_buffer) {
                std::cout << "[MISA++ OME] Writing results as OME TIFF " << m_path << " ... " << kv.first << std::endl;
                reader->setId(kv.second);
                ome_to_ome(*reader, misa_ome_plane_location(0, 0, 0, 0), *writer, kv.first);
            }

            reader->close();
            writer->close();
            m_write_buffer.clear();
        }

        /**
         * Returns the write buffer path for a location
         * @param t_location
         * @return
         */
        boost::filesystem::path get_write_buffer_path(const misa_ome_plane_location &t_location) const {
            return m_path.parent_path() / "__misa_ome_write_buffer__" / (m_path.filename().string() + "_" + cxxh::to_string(t_location) + ".ome.tif");
        }

        /**
         * Returns the OME TIFF writer for a write buffer
         * @param t_location
         * @return
         */
        std::shared_ptr<ome::files::out::OMETIFFWriter> get_buffer_writer(const misa_ome_plane_location &t_location) const {
            const auto size_X = m_metadata->getPixelsSizeX(t_location.series);
            const auto size_Y = m_metadata->getPixelsSizeY(t_location.series);
            const std::vector<size_t> channels = { static_cast<size_t>(m_metadata->getChannelSamplesPerPixel(t_location.series, t_location.c)) };

            // Create a new writer with only 1 plane
            auto writer = std::make_shared<ome::files::out::OMETIFFWriter>();
            auto metadata = helpers::create_ome_xml_metadata(size_X, size_Y, 1, 1, channels, m_metadata->getPixelsType(t_location.series));
            auto metadata_retrieve = std::static_pointer_cast<ome::xml::meta::MetadataRetrieve>(metadata);
            writer->setMetadataRetrieve(metadata_retrieve);
            writer->setInterleaved(false);

            // Change the writer path
            const auto buffer_path = get_write_buffer_path(t_location);
            if(!boost::filesystem::is_directory(buffer_path.parent_path())) {
                boost::filesystem::create_directories(buffer_path.parent_path());
            }
            writer->setId(buffer_path);

            // Store it into the buffer
            m_write_buffer.insert({ t_location, buffer_path});

            return writer;
        }

       /**
        * Copies all TIFF images stored inside the current reader into the buffer directory
        */
        void initialize_write_buffer_from_reader() const {
            std::cout << "[MISA++ OME] Preparing write mode for existing OME TIFF " << m_path << " ... " << std::endl;
            for(size_t series = 0; series < get_num_series(); ++series) {
                m_reader->setSeries(series);
                const auto size_Z = m_reader->getSizeZ();
                const auto size_C = m_reader->getSizeC();
                const auto size_T = m_reader->getSizeT();

                for(size_t z = 0; z < size_Z; ++z) {
                    for(size_t c = 0; c < size_C; ++c) {
                        for (size_t t = 0; t < size_T; ++t) {
                            const misa_ome_plane_location location(series, z, c, t);
                            const auto location_name = cxxh::to_string(location);
                            std::cout << "[MISA++ OME] Preparing write mode for existing OME TIFF " << m_path << " ... writing plane " << location_name << std::endl;

                            auto writer = get_buffer_writer(location);
                            ome_to_ome(*m_reader, misa_ome_plane_location(series, z, c, t), *writer, misa_ome_plane_location(0, 0, 0, 0));
                            writer->close();
                        }
                    }
                }
            }
        }
    };
}
