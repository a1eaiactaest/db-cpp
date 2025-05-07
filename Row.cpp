//
// Created by Piotrek Rybiec on 05/05/2025.
//

#include "Row.h"

auto Row::getValue(const std::string& column_name) const -> const Value& {
    return values.at(column_name);
}

auto Row::setValue(const std::string& column_name, Value val) -> void {
    values[column_name] = std::move(val);
}

auto Row::hasColumn(const std::string& column_name) const -> bool {
    return values.find(column_name) != values.end();
}

auto Row::hasColumn(const Column& col) const -> bool {
    return values.find(col.getName()) != values.end();
}

auto Row::getValues() const -> const ValueMap& {
    return values;
}

auto Row::empty() const -> bool {
    return values.empty();
}

auto Row::removeColumn(const std::string& column_name) -> bool {
    // if column existed and was removed returns true
    return values.erase(column_name) > 0; 
}

