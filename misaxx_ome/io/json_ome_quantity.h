//
// Created by rgerst on 06.12.18.
//

#pragma once
#include <ome/xml/model/primitives/Quantity.h>
#include <nlohmann/json.hpp>

namespace nlohmann {

    /**
     * ADL serializer for ome quantities
     * @tparam T
     * @tparam Unit
     */
    template<class Unit, typename T>
    struct adl_serializer<ome::xml::model::primitives::Quantity<Unit, T>> {
    static void to_json(json &j, const ome::xml::model::primitives::Quantity<Unit, T> &opt) {
        j["value"] = opt.getValue();
        j["unit"] = static_cast<std::string>(opt.getUnit());
    }

    static void from_json(const json &j,ome::xml::model::primitives::Quantity<Unit, T> &opt) {
        T v = j["value"].get<T>();
        Unit u = Unit::strings().at(j["unit"].get<std::string>());
        auto result = ome::xml::model::primitives::Quantity<Unit, T>(std::move(v), std::move(u));
        opt = std::move(result);
    }
};
}


