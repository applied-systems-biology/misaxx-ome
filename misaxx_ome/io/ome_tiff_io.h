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

namespace misaxx_ome {

    /**
    * Custom OMETIFFWriter that removes the sequential write check
    */
    struct non_sequential_ome_tiff_writer : public ome::files::out::OMETIFFWriter {
        using OMETIFFWriter::OMETIFFWriter;

        void setPlane(ome::files::dimension_size_type plane) const override {
            const ome::files::dimension_size_type currentPlane = getPlane();

            this->plane = plane;

            if (currentPlane != plane)
            {
                nextIFD();
                setupIFD();
            }
        }
    };

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
            m_buffer_path = m_path.parent_path() / (get_base_filename().string() + "_wbuffer.ome.tif");
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
            m_buffer_path = m_path.parent_path() / (get_base_filename().string() + "_wbuffer.ome.tif");
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
            auto writer = get_writer();
//            writer.value->setSeries(index.series); // TODO: Mutex
            const auto writer_path = m_path.parent_path() / (get_base_filename().string() + "_" + cxxh::to_string(index) + ".ome.tif");
            writer.value->changeOutputFile(writer_path);
            opencv_to_ome(image, *writer.value, index);
        }

        cv::Mat read_plane(const misa_ome_plane_location &index) const {
            if(index.series != 0)
                throw std::runtime_error("Only series 0 is currently supported!");
            auto reader = get_reader();
//            reader.value->setSeries(index.series); // TODO: Mutex
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
                if(static_cast<bool>(m_writer)) {
                    m_writer->close();
                    m_writer.reset();
                }
                open_reader();
                wlock.unlock();

                // Return the locked reader
                lock.lock();
                return locked_reader_type(m_reader, std::move(lock));
            }
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
            if(static_cast<bool>(m_writer)) {
                close_writer();
            }
        }

        ome::files::dimension_size_type get_num_series() const {
            return get_metadata()->getImageCount();
        }

        ome::files::dimension_size_type get_size_x(ome::files::dimension_size_type series) const {
            return get_metadata()->getPixelsSizeX(series);
        }

        ome::files::dimension_size_type get_size_y(ome::files::dimension_size_type series) const {
            return get_metadata()->getPixelsSizeY(series);
        }

        ome::files::dimension_size_type get_size_z(ome::files::dimension_size_type series) const {
            return get_metadata()->getPixelsSizeZ(series);
        }

        ome::files::dimension_size_type get_size_t(ome::files::dimension_size_type series) const {
            return get_metadata()->getPixelsSizeT(series);
        }

        ome::files::dimension_size_type get_size_c(ome::files::dimension_size_type series) const {
            return get_metadata()->getPixelsSizeC(series);
        }

        ome::files::dimension_size_type get_num_planes(ome::files::dimension_size_type series) const {
            return get_size_c(series) * get_size_t(series) * get_size_z(series);
        }

    protected:

        /**
        * Thread-safe access to the managed writer
        * The writer is unique-locked (sequential access!)
        * @return
        */
        locked_writer_type get_writer() const {
            std::unique_lock<std::shared_mutex> lock(m_mutex, std::defer_lock);
            lock.lock();
            if(static_cast<bool>(m_writer)) {
                return locked_writer_type(m_writer, std::move(lock));
            }
            else {

                // If the current TIFF file exists, set it to a reader->writer connection
                // This will preserve the TIFF data that is already written
                if(boost::filesystem::is_regular_file(m_path)) {
                    if(!static_cast<bool>(m_reader)) {
                        open_reader();
                    }
                }

                open_writer();
                return locked_writer_type(m_writer, std::move(lock));
            }
        }

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

    private:
        /**
         * Path of the TIFF that is read / written
         */
        mutable boost::filesystem::path m_path;
        /**
         * Buffer TIFF file if a reader is switched to a writer
         */
        mutable boost::filesystem::path m_buffer_path;
        mutable std::shared_ptr<ome::files::out::OMETIFFWriter> m_writer;
        mutable std::shared_ptr<ome::files::in::OMETIFFReader> m_reader;
        mutable std::shared_ptr<ome::xml::meta::OMEXMLMetadata> m_metadata;
        mutable std::shared_mutex m_mutex;

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

        void open_writer() const {
            if(!static_cast<bool>(m_metadata)) {
                throw std::runtime_error("Cannot write to OME TIFF " + m_path.string() + " without necessary metadata!");
            }
            auto metadata = std::static_pointer_cast<ome::xml::meta::MetadataRetrieve>(m_metadata);
            m_writer = std::make_shared<non_sequential_ome_tiff_writer>(); // Initialize a non-sequential TIFF writer
            m_writer->setMetadataRetrieve(metadata);
            m_writer->setInterleaved(false);
            m_writer->setWriteSequentially(false);

            // If the existing write target already exists, use the buffer path instead
            if(boost::filesystem::is_regular_file(m_path)) {
                std::swap(m_path, m_buffer_path);
            }

            m_writer->setId(m_path);

            if(static_cast<bool>(m_reader)) {
                // We need to copy the data from reader to writer (to preserve it)
                write_all_tiffs();
                close_reader();
            }
        }

        void close_writer() const {
            if(boost::filesystem::is_regular_file(m_buffer_path)) {
                // Switch to the write buffer
                std::swap(m_path, m_buffer_path);
            }
        }

        /**
         * Copies all TIFF data from the reader to the writer
         */
        void write_all_tiffs() const {
            std::cout << "[MISA++ OME] Switching " << m_path << " from read to write mode ... " << std::endl;

            for(size_t series = 0; series < get_num_series(); ++series) {
                m_reader->setSeries(series);
                m_writer->setSeries(series);

                const auto size_Z = m_reader->getSizeZ();
                const auto size_C = m_reader->getSizeC();
                const auto size_T = m_reader->getSizeT();

                for(size_t z = 0; z < size_Z; ++z) {
                    for(size_t c = 0; c < size_C; ++c) {
                        for (size_t t = 0; t < size_T; ++t) {
                            const auto location_name = cxxh::to_string(misa_ome_plane_location(series, z, c, t));
                            std::cout << "[MISA++ OME] Switching " << m_path << " from read to write mode ... " << location_name << std::endl;

                            // Change the writer path
                            const auto writer_path = m_path.parent_path() / (get_base_filename().string() + "_" + location_name + ".ome.tif");
                            m_writer->changeOutputFile(writer_path);

                            // Copy via variant pixel buffer
                            ome_to_ome(*m_reader, misa_ome_plane_location(series, z, c, t), *m_writer, misa_ome_plane_location(series, z, c, t));
                        }
                    }
                }
            }
        }
    };
}
