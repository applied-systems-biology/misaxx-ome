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
#include <misaxx/misa_cache.h>
#include <boost/regex.hpp>
#include <misaxx-ome/ome_helpers.h>
#include <ome/files/MetadataTools.h>

namespace misaxx_ome {

    // Forward declare
    struct misa_ome_plane_description;

    /**
     * Exposes the internal OME XML, as the metadata maps do not contain all information for some reason
     */
    struct custom_ome_tiff_reader : public ome::files::in::OMETIFFReader {

        std::shared_ptr<ome::xml::meta::OMEXMLMetadata> get_xml_metadata() const {
            auto meta = cacheMetadata(*currentId); // Try the cached metadata of the current file
            boost::filesystem::path first_tiff_path = meta->getUUIDFileName(0, 0);
            first_tiff_path = boost::filesystem::canonical(first_tiff_path, currentId.value().parent_path());
            if(currentId != first_tiff_path) {
                meta = ome::files::createOMEXMLMetadata(first_tiff_path);
            }
            return meta;
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

        using locked_reader_type = misaxx::utils::locked<std::shared_ptr<ome::files::in::OMETIFFReader>, std::shared_lock<std::shared_mutex>>;
        using locked_writer_type = misaxx::utils::locked<std::shared_ptr<ome::files::out::OMETIFFWriter>, std::unique_lock<std::shared_mutex>>;

        ome_tiff_io() = default;

        /**
         * Opens an existing OME TIFF file
         * @param t_path
         */
        explicit ome_tiff_io(boost::filesystem::path t_path);

        /**
         *  Opens an existing OME TIFF file or creates a new one based on the metadata
         *  If the file already exists, the metadata is loaded from the file instead.
         * @param t_path
         * @param t_metadata
         */
        explicit ome_tiff_io(boost::filesystem::path t_path,
                                     std::shared_ptr<ome::xml::meta::OMEXMLMetadata> t_metadata);

        /**
         * Opens an existing OME TIFF file or creates a new one based on the reference
         * @param t_path
         * @param t_reference
         */
        explicit ome_tiff_io(boost::filesystem::path t_path, const ome_tiff_io &t_reference);

        void write_plane(const cv::Mat &image, const misa_ome_plane_description &index);

        cv::Mat read_plane(const misa_ome_plane_description &index) const;

        /**
         * Thread-safe access to the metadata
         * @return
         */
        std::shared_ptr<ome::xml::meta::OMEXMLMetadata> get_metadata() const;

        boost::filesystem::path get_path() const {
            return m_path;
        }

        /**
         * Closes any open reader and writer. This method is thread-safe.
         */
        void close();

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
        mutable std::map<misa_ome_plane_description, boost::filesystem::path> m_write_buffer;

        mutable std::shared_ptr<custom_ome_tiff_reader> m_reader;
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

        void open_reader() const;

        void close_reader() const;

        void close_writer() const;

        /**
         * Returns the write buffer path for a location
         * @param t_location
         * @return
         */
        boost::filesystem::path get_write_buffer_path(const misa_ome_plane_description &t_location) const;

        /**
         * Returns the OME TIFF reader for a read buffer
         * @param t_location
         * @return
         */
        std::shared_ptr<ome::files::in::OMETIFFReader> get_buffer_reader(const misa_ome_plane_description &t_location) const;

        /**
         * Returns the OME TIFF writer for a write buffer
         * @param t_location
         * @return
         */
        std::shared_ptr<ome::files::out::OMETIFFWriter> get_buffer_writer(const misa_ome_plane_description &t_location) const;

       /**
        * Copies all TIFF images stored inside the current reader into the buffer directory
        */
        void initialize_write_buffer_from_reader() const;

        /**
        * Thread-safe access to the managed reader
        * If applicable, returns a reader to a plane in the write buffer
        * @return
        */
        locked_reader_type get_reader(const misa_ome_plane_description &t_location) const;

        /**
        * Thread-safe access to the managed writer
        * The writer is unique-locked (sequential access!)
        * @return
        */
        locked_writer_type get_writer(const misa_ome_plane_description &t_location) const;
    };
}
