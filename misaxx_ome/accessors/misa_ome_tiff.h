//
// Created by rgerst on 28.11.18.
//

#pragma once
#include <misaxx_ome/caches/misa_ome_tiff_cache.h>
#include <misaxx/misa_cached_data.h>

namespace misaxx_ome {
    /**
     * An OME TIFF file that contains a list of 2D image planes.
     * The planes can be accessed via their index within this structure or using a misa_ome_plane_location
     * that also encodes semantic location within a time/depth/channel space.
     */
    struct misa_ome_tiff : public misaxx::misa_cached_data<misa_ome_tiff_cache> {

        using iterator = typename misa_ome_tiff_planes_t ::iterator;
        using const_iterator = typename misa_ome_tiff_planes_t::const_iterator;

        iterator begin() {
            return this->data->get().begin();
        }

        iterator end() {
            return this->data->get().end();
        }

        const_iterator begin() const {
            return this->data->get().begin();
        }

        const_iterator end() const {
            return this->data->get().end();
        }

        size_t size() const {
            return this->data->get().size();
        }

        bool empty() const {
            return this->data->get().empty();
        }

        misa_ome_plane at(size_t index) const {
            return this->data->get().at(index);
        }

        misa_ome_plane at(const misa_ome_plane_location &index) {
            return this->data->get_plane(index);
        }

    };
}




