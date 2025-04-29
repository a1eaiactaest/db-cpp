//
// Created by Piotrek Rybiec on 29/04/2025.
//

#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <string>
#include <unordered_map>

enum class DataType {
    INTEGER,
    FLOAT,
    BOOLEAN,
    STRING,
    DATE,
    DATETIME,
};

inline auto dataTypeToString(const DataType& t) -> std::string {
    const std::unordered_map<DataType, std::string> tmap = {
        {DataType::INTEGER, "INTEGER"},
        {DataType::FLOAT, "FLOAT"},
        {DataType::BOOLEAN, "BOOLEAN"},
        {DataType::STRING, "STRING"},
        {DataType::DATE, "DATE"},
        {DataType::DATETIME, "DATETIME"},
    };
    return tmap.at(t);
}

inline auto stringToDataType(const std::string& s) -> DataType {
    const std::unordered_map<std::string, DataType> smap = {
        {"INTEGER", DataType::INTEGER},
        {"FLOAT", DataType::FLOAT},
        {"BOOLEAN", DataType::BOOLEAN},
        {"STRING", DataType::STRING},
        {"DATE", DataType::DATE},
        {"DATETIME", DataType::DATETIME},
    };
    return smap.at(s);
}


#endif //DATA_TYPES_H
