#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/ranges.h>
#include <fmt/core.h>
#include <stdexcept>

#include "data_types.h"
#include "CommonTypes.h"
#include "Commands.hpp"

auto CreateCommand::getTableName() const -> const std::string& {
    return table_name_;
}

auto CreateCommand::getColumns() const -> const std::vector<Column>& {
    return columns_;
}

auto CreateCommand::getConstraints() const -> const ConstraintList& {
    return constraints_;
}

auto CreateCommand::toString() const -> std::string {
    std::vector<std::string> columnDefs;
    for (const auto& column : columns_) {
        columnDefs.push_back(fmt::format("{} {}", column.getName(), dataTypeToString(column.getType())));
    }

    return fmt::format("CREATE TABLE {} ({})", table_name_, fmt::join(columnDefs, ", "));
}

auto AlterCommand::getTableName() const -> const std::string& {
    return table_name_;
}

auto AlterCommand::getAlterType() const -> AlterType {
    return alter_type_;
}

auto AlterCommand::getColumnName() const -> const std::string& {
    return column_name_;
}

auto AlterCommand::getNewColumnName() const -> const std::string& {
    return new_column_name_;
}

auto AlterCommand::getNewColumn() const -> const Column& {
    return new_column_;
}

auto AlterCommand::toString() const -> std::string {
    std::string result;

    switch (alter_type_) {
        case AlterType::ADD:
            result = fmt::format(
                "ALTER TABLE {} ADD COLUMN {} {}",
                table_name_,
                new_column_.getName(),
                dataTypeToString(new_column_.getType())
            );
            break;
        case AlterType::DROP:
            result = fmt::format(
                "ALTER TABLE {} DROP COLUMN {}",
                table_name_,
                column_name_ 
            );
            break;
        case AlterType::RENAME:
            result = fmt::format(
                "ALTER TABLE {} RENAME COLUMN {} TO {}",
                table_name_,
                column_name_,
                new_column_name_
            );
            break;
    }

    return result;
}


auto DropCommand::getTableName() const -> const std::string& {
    return table_name_;
}

auto DropCommand::toString() const -> std::string {
    return fmt::format("DROP TABLE {}", table_name_);
}


auto InsertCommand::getTableName() const -> const std::string& {
    return table_name_;
}

auto InsertCommand::getColumnNames() const -> const std::vector<std::string>& {
    return column_names_;
}

auto InsertCommand::getValues() const -> const std::vector<std::vector<Value>>& {
    return values_;
}

auto InsertCommand::toString() const -> std::string {
    std::string result = fmt::format("INSERT INTO {}", table_name_);

    // add column names if specified
    if (!column_names_.empty()) {
        result += fmt::format(" ({})", fmt::join(column_names_, ", "));
    }

    result += " VALUES ";

    std::vector<std::string> row_strings;
    for (const auto& row : values_) {
        std::vector<std::string> val_strings;
        for (const auto& val : row) {
            val_strings.push_back(val.toString()); // TODO: fix literals
        }
        row_strings.push_back(fmt::format("({})", fmt::join(val_strings, ", ")));
    }

    result += fmt::format("{}", fmt::join(row_strings, ", "));
    return result;
}


auto UpdateCommand::getTableName() const -> const std::string& {
    return table_name_;
}

auto UpdateCommand::getColumnValues() const -> const std::unordered_map<std::string, Value>& {
    return column_values_;
}

auto UpdateCommand::getWhereClause() const -> const std::string& {
    return where_clause_;
}

auto UpdateCommand::toString() const -> std::string {
    std::vector<std::string> assignments;
    for (const auto& [col, val] : column_values_) {
        assignments.push_back(fmt::format("{} = {}", col, val.toString()));
    }

    std::string result = fmt::format("UPDATE {} SET {}", table_name_, fmt::join(assignments, ", "));

    if (!where_clause_.empty()) {
        result += fmt::format(" WHERE {}", where_clause_);
    }

    return result;
}

auto DeleteCommand::getTableName() const -> const std::string& {
    return table_name_;
}

auto DeleteCommand::getWhereClause() const -> const std::string& {
    return where_clause_;
}

auto DeleteCommand::toString() const -> std::string {
    std::string result = fmt::format("DELETE FROM {}", table_name_);
    if (!where_clause_.empty()) {
        result += fmt::format(" WHERE {}", where_clause_);
    }
    return result;
}

auto SelectCommand::getColumnNames() const -> const std::vector<std::string>& {
    return column_names_;
}

auto SelectCommand::getTableNames() const -> const std::vector<std::string>& {
    return table_names_;
}

auto SelectCommand::getWhereClause() const -> const std::string& {
    return where_clause_;
}

auto SelectCommand::toString() const -> std::string {
    std::string column_part = column_names_.empty()
        ? "*"
        : fmt::format("{}", fmt::join(column_names_, ", "));

    std::string table_part = fmt::format("{}", fmt::join(table_names_, ", "));

    std::string result = fmt::format("SELECT {} FROM {}", column_part, table_part);

    if (!where_clause_.empty()) {
        result += fmt::format(" WHERE {}", where_clause_);
    }
    return result;
}

auto SaveCommand::getFilename() const -> const std::string& {
    return filename_;
};

auto SaveCommand::toString() const -> std::string {
    return fmt::format("SAVE TO '{}'", filename_);
}

auto LoadCommand::getFilename() const -> const std::string& {
    return filename_;
};

auto LoadCommand::toString() const -> std::string {
    return fmt::format("LOAD TO '{}'", filename_);
}

auto ShowCommand::getShowType() const -> ShowType {
    return show_type_;
}

auto ShowCommand::getTableName() const -> const std::string& {
    return table_name_;
}

auto ShowCommand::toString() const -> std::string {
    switch (show_type_) {
        case ShowType::TABLES:
            return "SHOW TABLES";
        case ShowType::COLUMNS:
            return fmt::format("SHOW COLUMNS FROM {}", table_name_);
        default:
            throw std::runtime_error("unknown SHOW command");
    }
}

