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
            return oss.str();
        } else if constexpr (std::is_same_v<T, bool>) {
            return arg ? "true" : "false";
        } else if constexpr (std::is_same_v<T, std::string>) {
            return arg;
        } else if constexpr (std::is_same_v<T, Date>) {
            std::ostringstream oss;
            // yyyy-mm-dd format
            oss << std::setfill('0')
                << static_cast<int>(arg.year()) << "-"
                << std::setw(2) << static_cast<unsigned>(arg.month()) << "-"
                << std::setw(2) << static_cast<unsigned>(arg.day());
            return oss.str();
        } else if constexpr (std::is_same_v<T, DateTime>) {
            auto in_time_t = std::chrono::system_clock::to_time_t(arg);
            std::ostringstream oss;
            oss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %H:%M:%S");
            return oss.str();
        } else {
            throw std::runtime_error("unsupported type in Value::toString");
        }
    }, value);
}
