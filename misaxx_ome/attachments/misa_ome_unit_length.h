//
// Created by rgerst on 11.12.18.
//

#pragma once

#include <nlohmann/json.hpp>
#include <misaxx/attachments/misa_unit.h>
#include <ome/xml/model/enums/UnitsLength.h>
#include <ome/xml/model/primitives/Quantity.h>
#include <misaxx/attachments/misa_unit_numeric.h>

namespace misaxx_ome {
    template<size_t Order> struct misa_ome_unit_length : public misaxx::misa_unit<Order>,
                                                 public boost::equality_comparable<misa_ome_unit_length<Order>> {

        using ome_unit_type = ome::xml::model::enums::UnitsLength;
        template<size_t O> using select_order_type = typename std::conditional<O < 1, misaxx::misa_unit_numeric, misa_ome_unit_length<O>>::type;

        misa_ome_unit_length() = default;

        misa_ome_unit_length(const ome_unit_type &t_value) : m_value(t_value) {
        }

        template<class Unit, typename = typename std::enable_if<std::is_base_of<misaxx::misa_unit_base, Unit>::value>::type>
        explicit misa_ome_unit_length(const misa_ome_unit_length<Order> &t_src) : m_value(t_src.m_value) {

        }

        explicit operator ome_unit_type() const {
            return m_value;
        }

        std::string get_literal() const override {
            return ome_unit_type::values().at(m_value);
        }

        ome_unit_type get_ome_unit() const {
            return m_value;
        }

        void from_json(const nlohmann::json &t_json) override {
            m_value = ome_unit_type::strings().at(t_json["unit"].get<std::string>());
        }

        void to_json(nlohmann::json &t_json) const override {
            misaxx::misa_serializeable::to_json(t_json);
            t_json["unit"] = static_cast<std::string>(m_value);
        }

        void to_json_schema(const misaxx::misa_json_schema &t_schema) const override {
            misaxx::misa_json_property<std::string> meta;
            std::vector<std::string> units;
            for(const auto &kv : ome_unit_type::values()) {
                units.push_back(kv.second);
            }
            meta.make_enum(std::move(units));
            t_schema.resolve("unit").declare_required(std::move(meta));
        }

        std::vector<misaxx::misa_serialization_id> get_serialization_id_hierarchy() const override {
            return misaxx::misa_serializeable::create_serialization_id_hierarchy(misaxx::misa_serialization_id("misa_ome", "attachments/units/length"), {
                misaxx::misa_serializeable::get_serialization_id_hierarchy()
            });
        }

        bool operator==(const misa_ome_unit_length &rhs) const {
            return m_value == rhs.m_value;
        }

        /**
        * Allows intra-unit conversion
        * Automatically called by misa_quantity
        * @tparam T
        * @param t_value
        * @param t_src
        * @param t_dst
        * @return
        */
        template<typename T> static T convert(T t_value, const misa_ome_unit_length<Order> &t_src, const misa_ome_unit_length<Order> &t_dst) {
            if(t_src == t_dst)
                return t_value;
            else {
                for(size_t i = 0; i < Order; ++i) {
                    ome::xml::model::primitives::Quantity<ome_unit_type, T> s(t_value, t_src.m_value);
                    auto u = ome::xml::model::primitives::convert(s, t_dst.m_value);
                    t_value = u.getValue();
                }
                return t_value;
            }
        }

    private:
        ome_unit_type m_value = typename ome_unit_type::enum_value(0);
    };
}

namespace nlohmann {
    template <size_t Order>
    struct adl_serializer<misaxx_ome::misa_ome_unit_length<Order>> {
        static void to_json(json& j, const misaxx_ome::misa_ome_unit_length<Order>& value) {
            value.to_json(j);
        }

        static void from_json(const json& j, misaxx_ome::misa_ome_unit_length<Order>& value) {
            value.from_json(j);
        }
    };
}



