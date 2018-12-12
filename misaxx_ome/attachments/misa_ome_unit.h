//
// Created by rgerst on 11.12.18.
//

#pragma once

#include <nlohmann/json.hpp>
#include <misaxx/attachments/misa_unit.h>
#include <ome/xml/model/enums/UnitsLength.h>
#include <ome/xml/model/enums/UnitsElectricPotential.h>
#include <ome/xml/model/enums/UnitsFrequency.h>
#include <ome/xml/model/enums/UnitsPower.h>
#include <ome/xml/model/enums/UnitsPressure.h>
#include <ome/xml/model/enums/UnitsTemperature.h>
#include <ome/xml/model/enums/UnitsTime.h>
#include <ome/xml/model/primitives/Quantity.h>
#include <misaxx/attachments/misa_unit_numeric.h>

namespace misaxx_ome {
    template<size_t Order, class OMEUnit> struct misa_ome_unit : public misaxx::misa_unit<Order>,
                                          public boost::equality_comparable<misa_ome_unit<Order, OMEUnit>> {

        using ome_unit_type = OMEUnit;
        template<size_t O> using select_order_type = typename std::conditional<O < 1, misaxx::misa_unit_numeric, misa_ome_unit<O, OMEUnit>>::type;

        misa_ome_unit() = default;

        misa_ome_unit(const ome_unit_type &t_value) : m_value(t_value) {
        }

        misa_ome_unit(const typename ome_unit_type::enum_value &t_value) : m_value(t_value) {
        }

        template<class Unit, typename = typename std::enable_if<std::is_base_of<misaxx::misa_unit_base, Unit>::value>::type>
        explicit misa_ome_unit(const Unit &t_src) : m_value(t_src.get_ome_unit()) {

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

        bool operator==(const misa_ome_unit &rhs) const {
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
        template<typename T> static T convert(T t_value, const misa_ome_unit<Order, OMEUnit> &t_src, const misa_ome_unit<Order, OMEUnit> &t_dst) {
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

    // Convenience types for all OME unit groups
    template<size_t Order> using misa_ome_unit_length = misa_ome_unit<Order, ome::xml::model::enums::UnitsLength>;
    template<size_t Order> using misa_ome_unit_electric_potential = misa_ome_unit<Order, ome::xml::model::enums::UnitsElectricPotential>;
    template<size_t Order> using misa_ome_unit_frequency = misa_ome_unit<Order, ome::xml::model::enums::UnitsFrequency>;
    template<size_t Order> using misa_ome_unit_power = misa_ome_unit<Order, ome::xml::model::enums::UnitsPower>;
    template<size_t Order> using misa_ome_unit_pressure = misa_ome_unit<Order, ome::xml::model::enums::UnitsPressure>;
    template<size_t Order> using misa_ome_unit_temperature = misa_ome_unit<Order, ome::xml::model::enums::UnitsTemperature>;
    template<size_t Order> using misa_ome_unit_time = misa_ome_unit<Order, ome::xml::model::enums::UnitsTime>;

    /**
     * Allows conversion of a misa_quantity with a misa_ome_unit to an OME Quantity
     * @tparam Quantity
     * @tparam Value
     * @tparam Unit
     * @tparam OMEUnit
     * @param t_quantity
     * @return
     */
    template<class Quantity,
            typename Value = typename Quantity::value_type,
            class Unit = typename Quantity::unit_type,
            class OMEUnit = typename Unit::ome_unit_type>
    inline ome::xml::model::primitives::Quantity<Unit, Value> misa_to_ome(const Quantity &t_quantity) {
        return ome::xml::model::primitives::Quantity<Unit, Value>(t_quantity.get_value(),
                t_quantity.get_unit().get_ome_unit());
    }
}

namespace nlohmann {
    template <size_t Order, class OMEUnit>
    struct adl_serializer<misaxx_ome::misa_ome_unit<Order, OMEUnit>> {
        static void to_json(json& j, const misaxx_ome::misa_ome_unit<Order, OMEUnit>& value) {
            value.to_json(j);
        }

        static void from_json(const json& j, misaxx_ome::misa_ome_unit<Order, OMEUnit>& value) {
            value.from_json(j);
        }
    };
}



