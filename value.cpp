//
// Created by Piotrek Rybiec on 30/04/2025.
//

#include "value.h"

#include <iomanip>

Value Value::Null() {
    return Value();
}

bool Value::isNull() const {
    return std::holds_alternative<std::monostate>(value);
}

DataType Value::getType() const {
    if (isNull()) return DataType::NULL_VALUE;
    if (std::holds_alternative<int>(value)) return DataType::INTEGER;
    if (std::holds_alternative<double>(value)) return DataType::FLOAT;
    if (std::holds_alternative<bool>(value)) return DataType::BOOLEAN;
    if (std::holds_alternative<std::string>(value)) return DataType::STRING;
    if (std::holds_alternative<Date>(value)) return DataType::DATE;
    if (std::holds_alternative<DateTime>(value)) return DataType::DATETIME;
    throw std::runtime_error("unknown type in Value");
}

std::string Value::toString() const {
    if (isNull()) return "NULL";
    // apply lambda to the whatever type is in value
    return std::visit([](auto&& arg) -> std::string {
        // deduct the type, and remove references
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, int>) {
            return std::to_string(arg);
        } else if constexpr (std::is_same_v<T, double>) {
            std::ostringstream oss; // fixed (no scientific), prec 6 output stream
            oss << std::fixed << std::setprecision(6) << arg;
        } else if constexpr (std::is_same_v<T, bool>) {
            return arg ? "true" : "false";
        }
    }, value);
}
