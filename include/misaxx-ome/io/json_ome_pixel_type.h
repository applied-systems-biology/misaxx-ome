//
// Created by rgerst on 26.11.18.
//


#pragma once
#include <ome/xml/model/enums.h>
#include <nlohmann/json.hpp>

namespace nlohmann {
    template <> struct adl_serializer<ome::xml::model::enums::PixelType> {
        static void to_json(json& j, const ome::xml::model::enums::PixelType& value) {
            j = std::string(value);
        }

        static ome::xml::model::enums::PixelType from_json(const json& j) {
           return ome::xml::model::enums::PixelType::strings().at(j.get<std::string>());
        }
    };
}