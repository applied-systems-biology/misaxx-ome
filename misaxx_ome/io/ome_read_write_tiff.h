//
// Created by rgerst on 23.11.18.
//


#pragma once

#include <memory>
#include <shared_mutex>
#include <ome/files/out/OMETIFFWriter.h>
#include <ome/files/in/OMETIFFReader.h>
#include <ome/xml/meta/Convert.h>
#include <opencv2/opencv.hpp>
#include <boost/filesystem.hpp>
#include <cxxh/cache.h>
#include "ome_to_opencv.h"
#include "opencv_to_ome.h"

namespace misaxx_ome {
    /**
     * Allows thread-safe read and write access to an OME TIFF
     * This wrapper will automatically switch between an OME TIFF reader and an OME TIFF writer depending on what functionality
     * is currently being requested.
     */
    class ome_read_write_tiff {
    public:

        using locked_reader_type = cxxh::locked<std::shared_ptr<ome::files::in::OMETIFFReader>, std::shared_lock<std::shared_mutex>>;
        using locked_writer_type = cxxh::locked<std::shared_ptr<ome::files::out::OMETIFFWriter>, std::shared_lock<std::shared_mutex>>;

        ome_read_write_tiff() = default;

        /**
         * Opens an existing OME TIFF file
         * @param t_path
         */
        explicit ome_read_write_tiff(boost::filesystem::path t_path) : m_path(std::move(t_path)) {
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
        explicit ome_read_write_tiff(boost::filesystem::path t_path,
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
        explicit ome_read_write_tiff(boost::filesystem::path t_path, const ome_read_write_tiff &t_reference)
                : ome_read_write_tiff(std::move(t_path), t_reference.get_metadata()) {
        }

        void write_plane(const cv::Mat &image, const misa_ome_plane_location &index) {
            if(index.series != 0)
                throw std::runtime_error("Only series 0 is currently supported!");
            auto writer = get_writer();
//            writer.value->setSeries(index.series); // TODO: Mutex
            opencv_to_ome(image, *writer.value, index);
        }

        cv::Mat read_plane(const misa_ome_plane_location &index) const {
            if(index.series != 0)
                throw std::runtime_error("Only series 0 is currently supported!");
            auto reader = get_reader();
//            reader.value->setSeries(index.series); // TODO: Mutex
            return ome_to_opencv(*reader.value, index);
        }

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
                return get_reader();
            }
        }

        /**
         * Thread-safe access to the managed writer
         * @return
         */
        locked_writer_type get_writer() const {
            std::shared_lock<std::shared_mutex> lock(m_mutex, std::defer_lock);
            lock.lock();
            if(static_cast<bool>(m_writer)) {
                return locked_writer_type(m_writer, std::move(lock));
            }
            else {
                lock.unlock();
                std::unique_lock<std::shared_mutex> wlock(m_mutex, std::defer_lock);
                wlock.lock();
                if(static_cast<bool>(m_reader)) {
                    m_reader->close();
                    m_reader.reset();
                }
                open_writer();
                wlock.unlock();
                return get_writer();
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

    private:
        boost::filesystem::path m_path;
        mutable std::shared_ptr<ome::files::out::OMETIFFWriter> m_writer;
        mutable std::shared_ptr<ome::files::in::OMETIFFReader> m_reader;
        mutable std::shared_ptr<ome::xml::meta::OMEXMLMetadata> m_metadata;
        mutable std::shared_mutex m_mutex;

        /**
         * Opens the reader
         * NOT THREAD SAFE!
         */
        void open_reader() const {
            m_reader = std::make_shared<ome::files::in::OMETIFFReader>();
            m_reader->setMetadataFiltered(false);
            m_reader->setGroupFiles(true);
            m_reader->setId(m_path);
            if(!static_cast<bool>(m_metadata)) {
               m_metadata = ome::files::createOMEXMLMetadata(*m_reader);
            }
        }

        /**
         * Opens the writer
         * NOT THREAD SAFE!
         */
        void open_writer() const {
            if(!static_cast<bool>(m_metadata)) {
                throw std::runtime_error("Cannot write to OME TIFF " + m_path.string() + " without necessary metadata!");
            }
            auto metadata = std::static_pointer_cast<ome::xml::meta::MetadataRetrieve>(m_metadata);
            m_writer = std::make_shared<ome::files::out::OMETIFFWriter>();
            m_writer->setMetadataRetrieve(metadata);
            m_writer->setInterleaved(false);
            m_writer->setWriteSequentially(false);
            m_writer->setId(m_path);
        }

        /**
         * Gets a property that is only accessible via the reader / writer instance
         * @tparam Function
         * @param t_function
         * @return
         */
        template<typename T, typename Function> T get_reader_writer_property(const Function &t_function) const {
            std::shared_lock<std::shared_mutex> lock(m_mutex, std::defer_lock);
            lock.lock();
            if(static_cast<bool>(m_reader)) {
                return t_function(m_reader);
            }
            else if(static_cast<bool>(m_writer)) {
                return t_function(m_writer);
            }
            else if(boost::filesystem::exists(m_path)) {
                lock.unlock();
                open_reader();
            }
            else {
                lock.unlock();
                open_writer();
            }
            return get_reader_writer_property<T, Function>(t_function);
        }
    };
}
