#include <misaxx/ome/caches/misa_ome_tiff_cache.h>
#include <misaxx/ome/attachments/misa_ome_planes_location.h>

void misaxx::ome::misa_ome_tiff_cache::do_link(const misaxx::ome::misa_ome_tiff_description &t_description) {

    if(t_description.filename.empty())
        throw std::runtime_error("Cannot link to file description with empty file name!");

    // We do cache initialization during linkage
    this->set_unique_location(this->get_location() / t_description.filename);

    if(boost::filesystem::exists(this->get_unique_location())) {
        std::cout << "[Cache] Opening OME TIFF " << this->get_unique_location() << std::endl;
        m_tiff = std::make_shared<ome_tiff_io>(this->get_unique_location());

        // Put the loaded metadata into the description
        this->describe()->template get<misa_ome_tiff_description>().metadata = m_tiff->get_metadata();
    }
    else {
        std::cout << "[Cache] Creating OME TIFF " << this->get_unique_location() << std::endl;

        // Create the TIFF and generate the image caches
        m_tiff = std::make_shared<ome_tiff_io>(this->get_unique_location(), t_description.metadata);
    }

    // Create the plane caches
    for(size_t series = 0; series < m_tiff->get_num_series(); ++series) {
        const auto size_Z = m_tiff->get_size_z(series);
        const auto size_C = m_tiff->get_size_c(series);
        const auto size_T = m_tiff->get_size_t(series);

        for(size_t z = 0; z < size_Z; ++z) {
            for(size_t c = 0; c < size_C; ++c) {
                for(size_t t = 0; t < size_T; ++t) {
                    misa_ome_plane cache;
                    cache.data = std::make_shared<misa_ome_plane_cache>();
                    cache.data->set_tiff_io(m_tiff);
                    cache.force_link(this->get_location(), misaxx::misa_description_storage::with(misa_ome_plane_description(series, z, c, t)));
                    this->get().emplace_back(std::move(cache));
                }
            }
        }
    }
}

bool misaxx::ome::misa_ome_tiff_cache::has() const {
    return static_cast<bool>(m_tiff);
}

std::shared_ptr<misaxx::ome::ome_tiff_io> misaxx::ome::misa_ome_tiff_cache::get_tiff_io() const {
    return m_tiff;
}

misaxx::ome::misa_ome_plane
misaxx::ome::misa_ome_tiff_cache::get_plane(const misaxx::ome::misa_ome_plane_description &t_location) const {
//            const auto num_series = m_tiff->get_num_series();
//            const auto size_Z = m_tiff->get_size_z(t_location.series);
    const auto size_C = m_tiff->get_size_c(t_location.series);
    const auto size_T = m_tiff->get_size_t(t_location.series);

    // Calculate the plane index
    size_t start_index = 0;
    for(size_t series = 0; series < t_location.series; ++series) {
        start_index += m_tiff->get_num_planes(series);
    }

    size_t index = start_index + t_location.t + t_location.c * size_T + t_location.z * size_T * size_C;
    return this->get().at(index);
}

void misaxx::ome::misa_ome_tiff_cache::postprocess() {
    misaxx::misa_default_cache<misaxx::utils::memory_cache<std::vector<misa_ome_plane>>,
            misa_ome_tiff_pattern, misa_ome_tiff_description>::postprocess();
    // Close the TIFF
    m_tiff->close();
}

misaxx::ome::misa_ome_tiff_description
misaxx::ome::misa_ome_tiff_cache::produce_description(const boost::filesystem::path &t_location,
                                                      const misaxx::ome::misa_ome_tiff_pattern &t_pattern) {
    auto file_description = t_pattern.produce(t_location);
    misa_ome_tiff_description result;
    result.filename = file_description.filename;
    return result;
}

std::shared_ptr<misaxx::misa_location> misaxx::ome::misa_ome_tiff_cache::create_location_interface() const {
    auto result = std::make_shared<misaxx::ome::misa_ome_planes_location>();
    result->filesystem_location = get_location();
    result->filesystem_unique_location = get_unique_location();

    for(const auto &plane : this->get()) {
        result->planes.push_back(plane.get_plane_location());
    }

    return result;
}
