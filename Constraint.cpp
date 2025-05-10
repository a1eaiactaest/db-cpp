//
// Created by Piotrek Rybiec on 04/05/2025.
//

#include <fmt/core.h>
#include <fmt/ranges.h>

#include "Table.h"
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

auto Constraint::validate(const Row& row, const Table& table, const Database& base) const -> bool {
    //throw std::runtime_error("base Constraint::validate has been called, which is incorrect.");
    return validate(row, table);
}

auto PrimaryKeyConstraint::getColumnNames() const -> const std::vector<std::string>& {
    return column_names;
}

auto PrimaryKeyConstraint::toString() const -> std::string  {
    return fmt::format("PRIMARY KEY ({})", fmt::join(column_names, ", "));
}

auto PrimaryKeyConstraint::validate(const Row& row, const Table& table, const Database& base) const -> bool {
    // all columns from pk exist in row
    for (const auto& col_name : column_names) {
        if (!row.hasColumn(col_name)) {
            return false;
        }
    }

    // values in the columns are unique across all rows in the table
    for (const auto& existing_row : table.getRows()) {
        bool all_equal = true;
        for (const auto& col_name : column_names) {
            if (existing_row.getValue(col_name) != row.getValue(col_name)) {
                all_equal = false;
                break;
            }
        }
        if (all_equal) return false;
    }
    return true;
}

auto ForeignKeyConstraint::validate(const Row& row, const Table& table) const -> bool {
    if (!row.hasColumn(column_name)) return false;
    // TODO: finish after implementing Database class
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

auto UniqueConstraint::toString() const -> std::string {
    return fmt::format("UNIQUE ({})", fmt::join(column_names, ", "));
}

auto NotNullConstraint::getColumnName() const -> const std::string& {
    return column_name;
}

auto NotNullConstraint::toString() const -> std::string {
    return fmt::format("NOT NULL ({})", column_name);
}

auto DefaultConstraint::getColumnName() const -> const std::string& {
    return column_name;
}

auto DefaultConstraint::getDefaultValue() const -> const Value& {
    return default_value;
}

auto DefaultConstraint::toString() const -> std::string {
    return fmt::format("DEFAULT {} FOR {}", default_value.toString(), column_name);
}
