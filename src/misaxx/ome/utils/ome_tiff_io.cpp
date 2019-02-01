#include <src/misaxx/ome/utils/ome_tiff_io.h>
#include <misaxx/ome/descriptions/misa_ome_plane_description.h>
#include <misaxx/core/utils/string.h>
#include <misaxx/imaging/utils/tiffio.h>
#include <opencv2/opencv.hpp>
#include "ome_to_opencv.h"
#include "opencv_to_ome.h"
#include "ome_to_ome.h"


misaxx::ome::ome_tiff_io::ome_tiff_io(boost::filesystem::path t_path) : m_path(std::move(t_path)) {
    if (!boost::filesystem::exists(m_path)) {
        throw std::runtime_error("Cannot read from non-existing file " + m_path.string());
    }
}

misaxx::ome::ome_tiff_io::ome_tiff_io(boost::filesystem::path t_path,
                                     std::shared_ptr<::ome::xml::meta::OMEXMLMetadata> t_metadata) : m_path(
        std::move(t_path)), m_metadata(std::move(t_metadata)) {
    // We can load metadata from the file if it exists
    if (boost::filesystem::exists(m_path)) {
        m_metadata.reset();
    }
}

misaxx::ome::ome_tiff_io::ome_tiff_io(boost::filesystem::path t_path, const misaxx::ome::ome_tiff_io &t_reference)
        : ome_tiff_io(std::move(t_path), t_reference.get_metadata()) {
}

misaxx::ome::ome_tiff_io::locked_reader_type
misaxx::ome::ome_tiff_io::get_reader(const misaxx::ome::misa_ome_plane_description &t_location) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex, std::defer_lock);
    lock.lock();
    if(static_cast<bool>(m_reader)) {
        return locked_reader_type(m_reader, std::move(lock));
    }
    else {
        std::unique_lock<std::shared_mutex> wlock(m_mutex, std::defer_lock);

        // Create a global reader from the path. Go into exclusive thread mode for this
        lock.unlock();
        wlock.lock();
        if(!m_write_buffer.empty()) {
            close_writer(true);
        }
        open_reader();
        wlock.unlock();

        // Return the locked reader
        lock.lock();
        return locked_reader_type(m_reader, std::move(lock));
    }
}

void misaxx::ome::ome_tiff_io::initialize_write_buffer_from_reader() const {
    std::cout << "[MISA++ OME] Preparing write mode for existing OME TIFF " << m_path << " ... " << "\n";
    for(size_t series = 0; series < get_num_series(); ++series) {
        m_reader->setSeries(series);
        const auto size_Z = m_reader->getSizeZ();
        const auto size_C = m_reader->getEffectiveSizeC();
        const auto size_T = m_reader->getSizeT();

        for(size_t z = 0; z < size_Z; ++z) {
            for(size_t c = 0; c < size_C; ++c) {
                for (size_t t = 0; t < size_T; ++t) {
                    const misa_ome_plane_description location(series, z, c, t);
                    const auto location_name = misaxx::utils::to_string(location);
                    std::cout << "[MISA++ OME] Preparing write mode for existing OME TIFF " << m_path << " ... writing plane " << location_name << "\n";

                    const boost::filesystem::path buffer_path = get_write_buffer_path(location);
                    if(!boost::filesystem::is_directory(buffer_path.parent_path())) {
                        boost::filesystem::create_directories(buffer_path.parent_path());
                    }

                    cv::Mat tmp = ome_to_opencv(*m_reader, misa_ome_plane_description(series, z, c, t));
                    misaxx::imaging::utils::tiffwrite(tmp, buffer_path);
                }
            }
        }
    }
}

boost::filesystem::path
misaxx::ome::ome_tiff_io::get_write_buffer_path(const misaxx::ome::misa_ome_plane_description &t_location) const {
    return m_path.parent_path() / "__misa_ome_write_buffer__" / (m_path.filename().string() + "_" + misaxx::utils::to_string(t_location) + ".ome.tif");
}

void misaxx::ome::ome_tiff_io::close_writer(bool remove_write_buffer) const {
    std::cout << "[MISA++ OME] Writing results as OME TIFF " << m_path << " ... " << "\n";
    // Save the write buffer files into the path
    auto writer = std::make_shared<::ome::files::out::OMETIFFWriter>();
    auto metadata = std::static_pointer_cast<::ome::xml::meta::MetadataRetrieve>(m_metadata);
    writer->setMetadataRetrieve(metadata);
    writer->setBigTIFF(true);
    writer->setId(m_path);
    const auto compression_types = writer->getCompressionTypes();
    if(compression_is_enabled() && compression_types.find("LZW") != compression_types.end()) {
        writer->setCompression("LZW");
    }

    for(const auto &kv : m_write_buffer) {
        std::cout << "[MISA++ OME] Writing results as OME TIFF " << m_path << " ... " << kv.first << "\n";
        cv::Mat tmp = misaxx::imaging::utils::tiffread(kv.second);
        opencv_to_ome(tmp, *writer, kv.first);

        // Remove write buffer if requested
        if(remove_write_buffer) {
            boost::filesystem::remove(kv.second);
        }
    }

    writer->close();
    m_write_buffer.clear();
}

void misaxx::ome::ome_tiff_io::close_reader() const {
    m_reader->close();
    m_reader.reset();
}

void misaxx::ome::ome_tiff_io::open_reader() const {
    m_reader = std::make_shared<custom_ome_tiff_reader>();
    m_reader->setMetadataFiltered(false);
    m_reader->setGroupFiles(true);
    m_reader->setId(m_path);

    // INFO: This would be the proper way of loading the metadata, but it does not work
    // For example PhysicalSize properties are missing
//            if(!static_cast<bool>(m_metadata)) {
//               m_metadata = ::ome::files::createOMEXMLMetadata(*m_reader);
//               ::ome::files::fillOriginalMetadata(*m_metadata, m_reader->getGlobalMetadata());
//
//               for(size_t series = 0; series < m_reader->getSeriesCount(); ++series) {
//                   m_reader->setSeries(series);
//                   ::ome::files::fillOriginalMetadata(*m_metadata, m_reader->getSeriesMetadata());
//               }
//            }

    if(!static_cast<bool>(m_metadata)) {
        m_metadata = m_reader->get_xml_metadata(); // Copy the XML data to be sure
    }
}

void misaxx::ome::ome_tiff_io::close(bool remove_write_buffer) {
    std::unique_lock<std::shared_mutex> wlock(m_mutex, std::defer_lock);
    wlock.lock();
    if(static_cast<bool>(m_reader)) {
        close_reader();
    }
    if(!m_write_buffer.empty()) {
        close_writer(remove_write_buffer);
    }
}

std::shared_ptr<::ome::xml::meta::OMEXMLMetadata> misaxx::ome::ome_tiff_io::get_metadata() const {
    if(static_cast<bool>(m_metadata)) {
        return m_metadata;
    }
    else {
        // We are currently reading a file. Open it and fetch the metadata
        if(!m_write_buffer.empty())
            throw std::logic_error("Write buffer is active, but no metadata is set!");
        get_reader(misa_ome_plane_description(0, 0, 0, 0));
        return m_metadata;
    }
}

cv::Mat misaxx::ome::ome_tiff_io::read_plane(const misaxx::ome::misa_ome_plane_description &index) const {
    if(index.series != 0)
        throw std::runtime_error("Only series 0 is currently supported!");

    if(m_write_buffer.find(index) == m_write_buffer.end()) {
        auto reader = get_reader(index);
        return ome_to_opencv(*reader.value, index);
    } else {
        // The write buffer contains only standard TIFFs
        return misaxx::imaging::utils::tiffread(m_write_buffer.at(index));
    }
}

void misaxx::ome::ome_tiff_io::write_plane(const cv::Mat &image, const misaxx::ome::misa_ome_plane_description &index) {
    // Lock this IO to allow writing to the write buffer
    std::unique_lock<std::shared_mutex> lock(m_mutex, std::defer_lock);
    lock.lock();

    if(index.series != 0)
        throw std::runtime_error("Only series 0 is currently supported!");

    // If the file already exists, we have to create a write buffer
    if(m_write_buffer.empty() && boost::filesystem::exists(m_path)) {
        initialize_write_buffer_from_reader();
    }

    const boost::filesystem::path buffer_path = get_write_buffer_path(index);
    if(!boost::filesystem::is_directory(buffer_path.parent_path())) {
        boost::filesystem::create_directories(buffer_path.parent_path());
    }
    misaxx::imaging::utils::tiff_compression compression;
    if(compression_is_enabled())
        compression = misaxx::imaging::utils::tiff_compression::lzw;
    else
        compression = misaxx::imaging::utils::tiff_compression::none;
    misaxx::imaging::utils::tiffwrite(image, buffer_path, compression);
    m_write_buffer[index] = buffer_path;
}

boost::filesystem::path misaxx::ome::ome_tiff_io::get_base_filename() const {
    auto base_name = m_path.filename();
    for(int i = 0; i < 2 && base_name.has_extension(); ++i) {
        base_name.replace_extension();
    }
    return base_name;
}

::ome::files::dimension_size_type misaxx::ome::ome_tiff_io::get_num_series() const {
    return get_metadata()->getImageCount();
}

::ome::files::dimension_size_type misaxx::ome::ome_tiff_io::get_size_x(::ome::files::dimension_size_type series) const {
    return get_metadata()->getPixelsSizeX(series);
}

::ome::files::dimension_size_type misaxx::ome::ome_tiff_io::get_size_y(::ome::files::dimension_size_type series) const {
    return get_metadata()->getPixelsSizeY(series);
}

::ome::files::dimension_size_type misaxx::ome::ome_tiff_io::get_size_z(::ome::files::dimension_size_type series) const {
    return get_metadata()->getPixelsSizeZ(series);
}

::ome::files::dimension_size_type misaxx::ome::ome_tiff_io::get_size_t(::ome::files::dimension_size_type series) const {
    return get_metadata()->getPixelsSizeT(series);
}

::ome::files::dimension_size_type misaxx::ome::ome_tiff_io::get_size_c(::ome::files::dimension_size_type series) const {
    return get_metadata()->getChannelCount(series);
}

::ome::files::dimension_size_type
misaxx::ome::ome_tiff_io::get_num_planes(::ome::files::dimension_size_type series) const {
    return get_size_c(series) * get_size_t(series) * get_size_z(series);
}

boost::filesystem::path misaxx::ome::ome_tiff_io::get_path() const {
    return m_path;
}

bool misaxx::ome::ome_tiff_io::compression_is_enabled() const {
    return m_enable_compression;
}

void misaxx::ome::ome_tiff_io::set_compression(bool enabled) {
    m_enable_compression = enabled;
}

