//
// Created by rgerst on 05.12.18.
//

#pragma once

#include <ome/xml/model/primitives/Quantity.h>
#include <misaxx/misa_serializeable.h>
#include <misaxx/attachments/misa_scalar.h>
#include <ome/xml/model/enums/UnitsLength.h>
#include <ome/xml/model/enums/UnitsElectricPotential.h>
#include <ome/xml/model/enums/UnitsFrequency.h>
#include <ome/xml/model/enums/UnitsPower.h>
#include <ome/xml/model/enums/UnitsPressure.h>
#include <ome/xml/model/enums/UnitsTime.h>
#include <ome/xml/model/enums/UnitsTemperature.h>

namespace misaxx_ome {
    /**
    * MISA++ wrapper around a ome::xml::model::primitives::Quantity
    * @tparam T the type that measures the unit
    * @tparam Unit the unit type
    */
template<typename T, class Unit> struct misa_ome_quantity : public misaxx::misa_scalar<ome::xml::model::primitives::Quantity<Unit, T>>,
        misaxx::scalar_operators<misa_ome_quantity<T, Unit>, T> {

        /**
         * OME quantity that is stored inside this object
         */
        using value_type = ome::xml::model::primitives::Quantity<Unit, T>;

        /**
         * Raw value type that the OME quantity is storing
         */
        using raw_value_type = T;

        /**
         * Unit enum
         */
        using unit_type = Unit;

        misa_ome_quantity() = default;

        explicit misa_ome_quantity(value_type value) : misaxx::misa_scalar<value_type>(std::move(value)) {

        }

        explicit misa_ome_quantity(T v, Unit u) : misaxx::misa_scalar<value_type>(value_type(v, u)) {

        }

        void from_json(const nlohmann::json &t_json) override {
            const auto u = Unit::values().at(t_json["unit"]);
            const T v = t_json["value"].get<T>();
            this->value = value_type(v, u);
        }

        void to_json(nlohmann::json &t_json) const override {
            t_json["unit"] = static_cast<std::string>(get_unit());
            t_json["value"] = get_value();
        }

        void to_json_schema(const misaxx::misa_json_schema &t_schema) const override {
            t_schema.resolve("value").declare_required<T>();
            std::vector<std::string> units;
            for (const auto &kv : Unit::strings()) {
                units.push_back(kv.first);
            }
            t_schema.resolve("unit").declare_required<std::string>(
                    misaxx::misa_json_property<std::string>().make_enum(std::move(units)));
        }

        Unit get_unit() const {
            return this->value.getUnit();
        }

        T get_value() const {
            return this->value.getValue();
        }

        /**
         * Converts the value to the unit
         * @param unit
         */
        void convert_to(Unit unit) {
            this->value = ome::xml::model::primitives::convert(get_value(), unit);
        }

        /**
         * Sets the value
         * @param value
         */
        void set_value(T value) {
            value = value_type (std::move(value), get_unit());
        }

        bool operator==(const misa_ome_quantity<T, Unit> &rhs) const {
            return this->value == rhs.value;
        }

        bool operator<(const misa_ome_quantity<T, Unit> &rhs) const {
            return this->value < rhs.value;
        }

        misa_ome_quantity<T, Unit>& operator+=(const misa_ome_quantity<T, Unit>& rhs) {
            this->value += rhs.value;
            return *this;
        }

        misa_ome_quantity<T, Unit>& operator-=(const misa_ome_quantity<T, Unit>& rhs) {
            this->value -= rhs.value;
            return *this;
        }

        misa_ome_quantity<T, Unit>& operator*=(const misa_ome_quantity<T, Unit>& rhs) {
            this->value *= rhs.value;
            return *this;
        }

        misa_ome_quantity<T, Unit>& operator/=(const misa_ome_quantity<T, Unit>& rhs) {
            this->value /= rhs.value;
            return *this;
        }
    };

    template<typename T = double> using misa_ome_length = misa_ome_quantity<T, ome::xml::model::enums::UnitsLength>;
    template<typename T = double> using misa_ome_electric_potential = misa_ome_quantity<T, ome::xml::model::enums::UnitsElectricPotential>;
    template<typename T = double> using misa_ome_frequency = misa_ome_quantity<T, ome::xml::model::enums::UnitsFrequency>;
    template<typename T = double> using misa_ome_power = misa_ome_quantity<T, ome::xml::model::enums::UnitsPower>;
    template<typename T = double> using misa_ome_pressure = misa_ome_quantity<T, ome::xml::model::enums::UnitsPressure>;
    template<typename T = double> using misa_ome_temperature = misa_ome_quantity<T, ome::xml::model::enums::UnitsTemperature>;
    template<typename T = double> using misa_ome_time = misa_ome_quantity<T, ome::xml::model::enums::UnitsTime>;
}

namespace nlohmann {
    template <typename T, class Unit>
    struct adl_serializer<misaxx_ome::misa_ome_quantity<T, Unit>> {
        static void to_json(json& j, const misaxx_ome::misa_ome_quantity<T, Unit>& opt) {
            opt.to_json(j);
        }

        static void from_json(const json& j, misaxx_ome::misa_ome_quantity<T, Unit>& opt) {
            opt.from_json(j);
        }
    };
}




