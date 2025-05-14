#include <iostream>
#include <fmt/base.h>
#include <fmt/ostream.h>
#include <fmt/format.h>

#include "Executor.hpp"
#include "Table.h"

auto Executor::execute(const std::unique_ptr<Command>& command) -> void {
    if (!command) {
        fmt::print(std::cerr, "err: null command recieved");
        return;
    }

    try {
        switch (command->getType()) {
            case CommandType::SELECT:
                executeSelect(static_cast<const SelectCommand&>(*command));
                break;
            case CommandType::CREATE:
                executeCreate(static_cast<const CreateCommand&>(*command));
                break;
            case CommandType::DROP:
                executeDrop(static_cast<const DropCommand&>(*command)); break;
            case CommandType::INSERT:
                executeInsert(static_cast<const InsertCommand&>(*command));
                break;
            case CommandType::UPDATE:
                executeUpdate(static_cast<const UpdateCommand&>(*command));
                break;
            case CommandType::DELETE:
                executeDelete(static_cast<const DeleteCommand&>(*command));
                break;
            default:
                std::cerr << "err: unsupported command type" << std::endl;
                break;
        }
    } catch (const std::exception& e) {
        fmt::print(std::cerr, "error executing command: {}", e.what());
    }
}

auto Executor::executeSelect(const SelectCommand& c) -> void {
    if (c.getTableNames().empty()) {
        throw std::runtime_error("no table specified in SELECT");
    }

    const std::string& table_name = c.getTableNames()[0];
    auto table = database_.getTable(table_name);
    if (!table) {
        throw std::runtime_error(fmt::format("table '{}' doesnt exist", table_name));
    }

    auto rows = table->getRows();
    const auto& columns = c.getColumnNames();
    for (const auto& col : columns) {
        fmt::print("{}\t", col);
    }
    fmt::print("\n");

    for (const auto& row : rows) {
        for (const auto& col : columns) {
            auto value = row.getValue(col);
            if (!value.isNull()) {
                fmt::print("{}\t", value.toString());
            } else {
                fmt::print("NULL\t");
            }
        }
        fmt::print("\n");
    }
}

auto Executor::executeCreate(const CreateCommand& c) -> void {
    const std::string& table_name = c.getTableName();

    if (database_.tableExists(table_name)) {
        throw std::runtime_error(fmt::format("table '{}' already exists", table_name));
    }

    auto table = std::make_shared<Table>(table_name, c.getColumns());
    database_.addTable(table);
    fmt::println("table '{}' created successfully", table_name);
}

auto Executor::executeDrop(const DropCommand& c) -> void {
    const std::string& table_name = c.getTableName();

    if (!database_.tableExists(table_name)) {
        throw std::runtime_error(fmt::format("table '{}' doesnt exist", table_name));
    }
    if (database_.dropTable(table_name)) {
        fmt::println("table '{}' dropped successfully", table_name);
    } else {
        throw std::runtime_error(fmt::format("failed to drop table '{}'", table_name));
    }
}

auto Executor::executeInsert(const InsertCommand& c) -> void {
    const std::string& table_name = c.getTableName();
    auto table = database_.getTable(table_name);

    if (!table) {
        throw std::runtime_error(fmt::format("table '{}' doesnt exist", table_name));
    }

    const auto& column_names = c.getColumnNames();
    const auto& values = c.getValues();

    if (values.empty()) {
        throw std::runtime_error("no values provided for INSERT");
    }

    // for each set of values -> insert a row
    for (const auto& value_set : values) {
        Row row;
        for (int i = 0; i < column_names.size(); i++) {
            row.setValue(column_names[i], value_set[i]);
        }
        if (!database_.validateRow(table_name, row)) {
            throw std::runtime_error(fmt::format("row validation failed for table '{}'", table_name));
        }

        table->addRow(row);
    }

    fmt::println("successfully inserted ({}) row(s) into {}", values.size(), table_name);
}

auto Executor::executeUpdate(const UpdateCommand& c) -> void {
    const std::string& table_name = c.getTableName();
    auto table = database_.getTable(table_name);

    if (!table) {
        throw std::runtime_error(fmt::format("table '{}' doesnt exist", table_name));
    }

    const auto& updates = c.getColumnValues();
    const auto& where_clause = c.getWhereClause();

    //TODO: where shit
    auto rows = table->getRows();
    for (auto& row : rows) {
        for (const auto& [col_name, value] : updates) {
            row.setValue(col_name, value);
        }
    }

    fmt::println("successfully updated ({}) row(s) in {}", rows.size(), table_name);
}

auto Executor::executeDelete(const DeleteCommand& c) -> void {
    const std::string& table_name = c.getTableName();
    auto table = database_.getTable(table_name);

    if (!table) {
        throw std::runtime_error(fmt::format("table '{}' doesnt exist", table_name));
    }
}




