#include <misaxx_ome/io/ome_tiff_io.h>


misaxx_ome::ome_tiff_io::ome_tiff_io(boost::filesystem::path t_path) : m_path(std::move(t_path)) {
    if (!boost::filesystem::is_regular_file(m_path)) {
        throw std::runtime_error("Cannot read from non-existing file " + m_path.string());
    }
}

misaxx_ome::ome_tiff_io::ome_tiff_io(boost::filesystem::path t_path,
                                     std::shared_ptr<ome::xml::meta::OMEXMLMetadata> t_metadata) : m_path(
        std::move(t_path)), m_metadata(std::move(t_metadata)) {
    // We can load metadata from the file if it exists
    if (boost::filesystem::is_regular_file(m_path)) {
        m_metadata.reset();
    }
}

misaxx_ome::ome_tiff_io::ome_tiff_io(boost::filesystem::path t_path, const misaxx_ome::ome_tiff_io &t_reference)
        : ome_tiff_io(std::move(t_path), t_reference.get_metadata()) {
}


misaxx_ome::ome_tiff_io::locked_writer_type
misaxx_ome::ome_tiff_io::get_writer(const misaxx_ome::misa_ome_plane_description &t_location) const {
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

misaxx_ome::ome_tiff_io::locked_reader_type
misaxx_ome::ome_tiff_io::get_reader(const misaxx_ome::misa_ome_plane_description &t_location) const {
    std::shared_lock<std::shared_mutex> lock(m_mutex, std::defer_lock);
    lock.lock();
    if(static_cast<bool>(m_reader)) {
        return locked_reader_type(m_reader, std::move(lock));
    }
    else {
        std::unique_lock<std::shared_mutex> wlock(m_mutex, std::defer_lock);

        // Check if the write buffer contains the location
        // If this is the case, just read the write buffer
        if(m_write_buffer.find(t_location) != m_write_buffer.end()) {
            return locked_reader_type(get_buffer_reader(t_location), std::move(lock));
        }

        // Create a global reader from the path. Go into exclusive thread mode for this
        lock.unlock();
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

void misaxx_ome::ome_tiff_io::initialize_write_buffer_from_reader() const {
    std::cout << "[MISA++ OME] Preparing write mode for existing OME TIFF " << m_path << " ... " << std::endl;
    for(size_t series = 0; series < get_num_series(); ++series) {
        m_reader->setSeries(series);
        const auto size_Z = m_reader->getSizeZ();
        const auto size_C = m_reader->getEffectiveSizeC();
        const auto size_T = m_reader->getSizeT();

        for(size_t z = 0; z < size_Z; ++z) {
            for(size_t c = 0; c < size_C; ++c) {
                for (size_t t = 0; t < size_T; ++t) {
                    const misa_ome_plane_description location(series, z, c, t);
                    const auto location_name = cxxh::to_string(location);
                    std::cout << "[MISA++ OME] Preparing write mode for existing OME TIFF " << m_path << " ... writing plane " << location_name << std::endl;

                    auto writer = get_buffer_writer(location);
                    ome_to_ome(*m_reader, misa_ome_plane_description(series, z, c, t), *writer, misa_ome_plane_description(0, 0, 0, 0));
                    writer->close();
                }
            }
        }
    }
}

std::shared_ptr<ome::files::out::OMETIFFWriter>
misaxx_ome::ome_tiff_io::get_buffer_writer(const misaxx_ome::misa_ome_plane_description &t_location) const {
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

std::shared_ptr<ome::files::in::OMETIFFReader>
misaxx_ome::ome_tiff_io::get_buffer_reader(const misaxx_ome::misa_ome_plane_description &t_location) const {
    auto reader = std::make_shared<ome::files::in::OMETIFFReader>();
    reader->setMetadataFiltered(false);
    reader->setGroupFiles(true);
    const auto buffer_path = get_write_buffer_path(t_location);
    reader->setId(buffer_path);
    return reader;
}

boost::filesystem::path
misaxx_ome::ome_tiff_io::get_write_buffer_path(const misaxx_ome::misa_ome_plane_description &t_location) const {
    return m_path.parent_path() / "__misa_ome_write_buffer__" / (m_path.filename().string() + "_" + cxxh::to_string(t_location) + ".ome.tif");
}

void misaxx_ome::ome_tiff_io::close_writer() const {
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
        ome_to_ome(*reader, misa_ome_plane_description(0, 0, 0, 0), *writer, kv.first);
    }

    reader->close();
    writer->close();
    m_write_buffer.clear();
}

void misaxx_ome::ome_tiff_io::close_reader() const {
    m_reader->close();
    m_reader.reset();
}

void misaxx_ome::ome_tiff_io::open_reader() const {
    m_reader = std::make_shared<custom_ome_tiff_reader>();
    m_reader->setMetadataFiltered(false);
    m_reader->setGroupFiles(true);
    m_reader->setId(m_path);

    // INFO: This would be the proper way of loading the metadata, but it does not work
    // For example PhysicalSize properties are missing
//            if(!static_cast<bool>(m_metadata)) {
//               m_metadata = ome::files::createOMEXMLMetadata(*m_reader);
//               ome::files::fillOriginalMetadata(*m_metadata, m_reader->getGlobalMetadata());
//
//               for(size_t series = 0; series < m_reader->getSeriesCount(); ++series) {
//                   m_reader->setSeries(series);
//                   ome::files::fillOriginalMetadata(*m_metadata, m_reader->getSeriesMetadata());
//               }
//            }

    if(!static_cast<bool>(m_metadata)) {
        m_metadata = m_reader->get_xml_metadata(); // Copy the XML data to be sure
    }
}

void misaxx_ome::ome_tiff_io::close() {
    std::unique_lock<std::shared_mutex> wlock(m_mutex, std::defer_lock);
    wlock.lock();
    if(static_cast<bool>(m_reader)) {
        close_reader();
    }
    if(!m_write_buffer.empty()) {
        close_writer();
    }
}

std::shared_ptr<ome::xml::meta::OMEXMLMetadata> misaxx_ome::ome_tiff_io::get_metadata() const {
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

cv::Mat misaxx_ome::ome_tiff_io::read_plane(const misaxx_ome::misa_ome_plane_description &index) const {
    if(index.series != 0)
        throw std::runtime_error("Only series 0 is currently supported!");

    auto reader = get_reader(index);
    if(m_write_buffer.find(index) == m_write_buffer.end())
        return ome_to_opencv(*reader.value, index);
    else
        return ome_to_opencv(*reader.value, misaxx_ome::misa_ome_plane_description(0, 0, 0 ,0));
}

void misaxx_ome::ome_tiff_io::write_plane(const cv::Mat &image, const misaxx_ome::misa_ome_plane_description &index) {
    if(index.series != 0)
        throw std::runtime_error("Only series 0 is currently supported!");
    auto writer = get_writer(index);
    opencv_to_ome(image, *writer.value, misa_ome_plane_description(0, 0, 0, 0));
    writer.value->close();
}

