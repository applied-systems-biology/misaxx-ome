//
// Created by rgerst on 27.11.18.
//


#pragma once

#include <ome/files/Types.h>
#include <ome/files/detail/FormatReader.h>
#include <ome/files/FormatWriter.h>
#include <ome/files/out/OMETIFFWriter.h>
#include <misaxx/misa_data_description.h>
#include <ostream>

namespace misaxx_ome {

    /**
     * Describes the location of an image plane (2D image) within an OME TIFF
     */
    struct misa_ome_plane_description : public misaxx::misa_data_description {

        /**
         * The series of images the target image belongs to.
         * All planes (2D images) within a series have the same X-Y size.
         * There are |Z| * |C| * |T| planes within a series.
         */
        ome::files::dimension_size_type series = 0;
        /**
         * The Z location (depth) the target plane (2D image) is assigned to
         */
        ome::files::dimension_size_type z = 0;
        /**
         * The Channel the target plane (2D image) is assigned to
         */
        ome::files::dimension_size_type c = 0;
        /**
         * The time location the target plane (2D image) is assigned to
         */
        ome::files::dimension_size_type t = 0;

        misa_ome_plane_description() = default;

        explicit misa_ome_plane_description(ome::files::dimension_size_type t_series) :
                series(t_series), z(0), c(0), t(0) {

        }

        explicit misa_ome_plane_description(ome::files::dimension_size_type t_series, ome::files::dimension_size_type t_z, ome::files::dimension_size_type t_c, ome::files::dimension_size_type t_t) :
            series(t_series), z(t_z), c(t_c), t(t_t) {

        }

        /**
         * Converts the data into a OME native ZCT coordinate.
         * The information about the series is not used!
         * @return
         */
        std::array<ome::files::dimension_size_type, 3> as_zct() const {
            return { z, c, t };
        }

        /**
         * Converts the data into a plane index within an OME Format reader
         * @param reader
         * @return
         */
        ome::files::dimension_size_type index_within(const ome::files::FormatReader &reader) const {
            if(reader.getSeries() != series)
                throw std::runtime_error("The reader must be located at the same series as the location description!");
            return reader.getIndex(z, c, t);
        }

        ome::files::dimension_size_type index_within(const ome::files::out::OMETIFFWriter &writer) const {
            if(writer.getSeries() != series)
                throw std::runtime_error("The reader must be located at the same series as the location description!");
            return writer.getIndex(z, c, t);
        }

        void from_json(const nlohmann::json &t_json) override {
            series = t_json["series"];
            z = t_json["z"];
            c = t_json["c"];
            t = t_json["t"];
        }

        void to_json(nlohmann::json &t_json) const override {
            t_json["series"] = series;
            t_json["z"] = z;
            t_json["c"] = c;
            t_json["t"] = t;
        }

        void to_json_schema(const misaxx::misa_json_schema &t_schema) const override {
            t_schema.resolve("series").declare_required<ome::files::dimension_size_type>();
            t_schema.resolve("z").declare_required<ome::files::dimension_size_type>();
            t_schema.resolve("c").declare_required<ome::files::dimension_size_type>();
            t_schema.resolve("t").declare_required<ome::files::dimension_size_type>();
        }

        misaxx::misa_serialization_id get_serialization_id() const override {
            return misaxx::misa_serialization_id("misa_ome", "ome-image-location");
        }

        bool operator==(const misa_ome_plane_description &rhs) const {
            return series == rhs.series &&
                   z == rhs.z &&
                   c == rhs.c &&
                   t == rhs.t;
        }

        bool operator!=(const misa_ome_plane_description &rhs) const {
            return !(rhs == *this);
        }

        bool operator<(const misa_ome_plane_description &rhs) const {
            return series < rhs.series || z < rhs.z || c < rhs.c || t < rhs.t;
        }

        bool operator>(const misa_ome_plane_description &rhs) const {
            return rhs < *this;
        }

        bool operator<=(const misa_ome_plane_description &rhs) const {
            return !(rhs < *this);
        }

        bool operator>=(const misa_ome_plane_description &rhs) const {
            return !(*this < rhs);
        }

        friend std::ostream &operator<<(std::ostream &os, const misa_ome_plane_description &location) {
            os << "S" << location.series << "_Z" << location.z << "_C" << location.c << "_T" << location.t;
            return os;
        }
    };

    inline void to_json(nlohmann::json& j, const misa_ome_plane_description& p) {
        p.to_json(j);
    }

    inline void from_json(const nlohmann::json& j, misa_ome_plane_description& p) {
        p.from_json(j);
    }
}

namespace std {
    template <> struct hash<misaxx_ome::misa_ome_plane_description>
    {
        size_t operator()(const misaxx_ome::misa_ome_plane_description & x) const
        {
            const auto H = hash<ome::files::dimension_size_type>();
            return H(x.series) + H(x.z) + H(x.c) + H(x.t);
        }
    };
}
