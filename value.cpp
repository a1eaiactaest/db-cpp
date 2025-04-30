//
// Created by Piotrek Rybiec on 30/04/2025.
//

#include "value.h"

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
    // TODO:
}
