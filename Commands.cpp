#include <fmt/format.h>
#include <fmt/ranges.h>
#include <fmt/ranges.h>
#include <fmt/core.h>

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


