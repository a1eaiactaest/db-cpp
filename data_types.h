//
// Created by Piotrek Rybiec on 29/04/2025.
//

#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <string>
#include <unordered_map>
#include <chrono>

enum class ConstraintType {
    PRIMARY_KEY,
    FOREIGN_KEY,
    UNIQUE,
    NOT_NULL,
    DEFAULT,
};

enum class DataType {
    INTEGER,
    FLOAT,
    BOOLEAN,
    STRING,
    DATE,
    DATETIME,
    NULL_VALUE,
};

class Constraint;
class ForeignKeyConstraint;
class Value;

using Date = std::chrono::year_month_day;
using DateTime = std::chrono::sys_time<std::chrono::milliseconds>;

// type safety, corresponding types are self-explanatory
using VariantType = std::variant<
    int,
    double,
    bool,
    std::string,
    Date,
    DateTime,
    std::monostate // for NULL
>;

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
