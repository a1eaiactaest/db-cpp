//
// Created by Piotrek Rybiec on 04/05/2025.
//

#include <fmt/core.h>
#include <fmt/ranges.h>

#include "Table.h"
#include "Constraint.h"
#include "Database.h"

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
    return validate(row, table);
}

auto PrimaryKeyConstraint::validate(const Row& row, const Table& table) const -> bool {
    // all columns from pk must exist in row and can't be null
    for (const auto& col_name : column_names) {
        if (!row.hasColumn(col_name)) {
            return false;
        }
        if (row.getValue(col_name).isNull()) {
            return false;  // primary key columns cannot be null
        }
    }

    for (const auto& existing_row : table.getRows()) {
        bool matches = true;
        for (const auto& col_name : column_names) {
            if (!existing_row.hasColumn(col_name) || 
                existing_row.getValue(col_name) != row.getValue(col_name)) {
                matches = false;
                break;
            }
        }
        // if all columns match, we have a duplicate - primary key constraint violated
        if (matches) {
            return false;
        }
    }

    return true;  
}

auto ForeignKeyConstraint::validate(const Row& row, const Table& table) const -> bool {
    if (!row.hasColumn(column_name)) return false;
    const auto& value = row.getValue(column_name);
    if (value.isNull()) return true; // allow nulls
    return false; // without database context, we can't validate foreign key references
}

auto ForeignKeyConstraint::validate(const Row& row, const Table& table, const Database& base) const -> bool {
    if (!row.hasColumn(column_name)) return false;
    if (!base.tableExists(ref_table)) return false;

    auto ref_table_obj = base.getTable(ref_table);
    if (!ref_table_obj) return false;

    const auto& value = row.getValue(column_name);
    if (value.isNull()) return true; // allow nulls

    // check if value exists
    for (const auto& ref_row : ref_table_obj->getRows()) {
        if (ref_row.hasColumn(ref_column) && (ref_row.getValue(ref_column) == value)) {
            return true;
        }
    }
    return false;
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

auto UniqueConstraint::validate(const Row& row, const Table& table) const -> bool {
    for (const auto& col_name : column_names) {
        if (!row.hasColumn(col_name)) {
            return false;
        }
        if (row.getValue(col_name).isNull()) {
            return true;  // null values don't violate uniqueness
        }
    }

    for (const auto& existing_row : table.getRows()) {
        bool matches = true;

        for (const auto& col_name : column_names) {
            if (!existing_row.hasColumn(col_name) || 
                existing_row.getValue(col_name).isNull() ||
                existing_row.getValue(col_name) != row.getValue(col_name)) {
                matches = false;
                break;
            }
        }
        // if all columns match, we have a duplicate
        if (matches) {
            return false;  
        }
    }
    return true;  // no dups
}

auto UniqueConstraint::validate(const Row& row, const Table& table, const Database& base) const -> bool {
    return validate(row, table);
}

auto NotNullConstraint::getColumnName() const -> const std::string& {
    return column_name;
}

auto NotNullConstraint::toString() const -> std::string {
    return fmt::format("NOT NULL ({})", column_name);
}

auto NotNullConstraint::validate(const Row& row, const Table& table) const -> bool {
    return !row.getValue(column_name).isNull();
}

auto NotNullConstraint::validate(const Row& row, const Table& table, const Database& database) const -> bool {
    return validate(row, table);
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
