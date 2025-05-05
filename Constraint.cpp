//
// Created by Piotrek Rybiec on 04/05/2025.
//

#include <fmt/core.h>
#include <fmt/ranges.h>

#include "Constraint.h"

Constraint::~Constraint() = default;

auto Constraint::getType() const -> ConstraintType {
    return type;
}

auto Constraint::getName() const -> const std::string& {
    return name;
}

auto Constraint::toString() const -> std::string {
    return name;
}

auto PrimaryKeyConstraint::getColumnNames() const -> const std::vector<std::string>& {
    return column_names;
}

auto PrimaryKeyConstraint::toString() const -> std::string override {
    return fmt::format("PRIMARY KEY ({})", fmt::join(column_names, ", "));
}

auto ForeignKeyConstraint::getColumnName() const -> const std::string& {
    return column_name;
}

auto ForeignKeyConstraint::getRefTable() const -> const std::string& {
    return ref_table;
}

auto ForeignKeyConstraint::getRefColumn() const -> const std::string& {
    return ref_column;
}

auto ForeignKeyConstraint::toString() const -> std::string {
    return fmt::format("FOREIGN KEY ({}) REFERENCES {}({})", column_name, ref_table, ref_column);
}

auto UniqueConstraint::getColumnNames() const -> const std::vector<std::string>& {
    return column_names;
}

auto UniqueConstraint::toString() const -> std::string override {
    return fmt::format("UNIQUE ({})", fmt::join(column_names, ", "));
}

auto NotNullConstraint::getColumnName() const -> const std::string& {
    return column_name;
}

auto NotNullConstraint::toString() const -> std::string override {
    return fmt::format("NOT NULL ({})", column_name);
}

auto DefaultConstraint::getColumnName() const -> const std::string& {
    return column_name;
}

auto DefaultConstraint::getDefaultValue() const -> const Value& {
    return default_value;
}

auto DefaultConstraint::toString() const -> std::string override {
    return fmt::format("DEFAULT {} FOR {}", default_value.toString(), column_name);
}
