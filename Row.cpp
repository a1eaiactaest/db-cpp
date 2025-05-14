//
// Created by Piotrek Rybiec on 05/05/2025.
//

#include "Row.h"
#include "Column.h"

auto Row::getValue(const std::string& column_name) const -> const Value& {
    return values.at(column_name);
}

auto Row::getValue(const Column& col) const -> const Value& {
    return values.at(col.getName());
}

void Row::setValue(const std::string& column_name, const Value& val) {
    values[column_name] = val;
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

