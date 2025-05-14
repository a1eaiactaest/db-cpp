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
            case CommandType::ALTER:
                executeAlter(static_cast<const AlterCommand&>(*command));
                break;
            case CommandType::SAVE:
                executeSave(static_cast<const SaveCommand&>(*command));
                break;
            case CommandType::LOAD:
                executeLoad(static_cast<const LoadCommand&>(*command));
                break;
            case CommandType::SHOW:
                executeShow(static_cast<const ShowCommand&>(*command));
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
            const auto& value = row.getValue(col);
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
        if (value_set.size() != column_names.size()) {
            throw std::runtime_error(fmt::format("mismatch between number of columns ({}) and values ({})", 
                column_names.size(), value_set.size()));
        }

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
    if (table_name.empty()) {
        throw std::runtime_error("no table specified in UPDATE");
    }

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
    if (table_name.empty()) {
        throw std::runtime_error("no table specified in DELETE");
    }

    auto table = database_.getTable(table_name);
    if (!table) {
        throw std::runtime_error(fmt::format("table '{}' doesnt exist", table_name));
    }

    const auto& where_clause = c.getWhereClause();

    //TODO: also where shit
    table->clear();
    fmt::println("successfully deleted rows from '{}'", table_name);
}

auto Executor::executeAlter(const AlterCommand& c) -> void {
    const std::string& table_name = c.getTableName();
    auto table = database_.getTable(table_name);
    
    if (!table) {
        throw std::runtime_error(fmt::format("table '{}' doesn't exist", table_name));
    }

    switch (c.getAlterType()) {
        case AlterCommand::AlterType::ADD: {
            const auto& new_column = c.getNewColumn();
            if (table->hasColumn(new_column.getName())) {
                throw std::runtime_error(fmt::format("column '{}' already exists in table '{}'", 
                    new_column.getName(), table_name));
            }
            table->addColumn(new_column);
            fmt::println("successfully added column '{}' to table '{}'", 
                new_column.getName(), table_name);
            break;
        }
        case AlterCommand::AlterType::DROP: {
            const auto& column_name = c.getColumnName();
            if (!table->hasColumn(column_name)) {
                throw std::runtime_error(fmt::format("column '{}' doesn't exist in table '{}'", 
                    column_name, table_name));
            }
            table->dropColumn(column_name);
            fmt::println("successfully dropped column '{}' from table '{}'", 
                column_name, table_name);
            break;
        }
        case AlterCommand::AlterType::RENAME: {
            const auto& old_name = c.getColumnName();
            const auto& new_name = c.getNewColumnName();
            if (!table->hasColumn(old_name)) {
                throw std::runtime_error(fmt::format("column '{}' doesn't exist in table '{}'", 
                    old_name, table_name));
            }
            if (table->hasColumn(new_name)) {
                throw std::runtime_error(fmt::format("column '{}' already exists in table '{}'", 
                    new_name, table_name));
            }
            table->renameColumn(old_name, new_name);
            fmt::println("successfully renamed column '{}' to '{}' in table '{}'", 
                old_name, new_name, table_name);
            break;
        }
    }
}

auto Executor::executeSave(const SaveCommand& c) -> void {
    const std::string& filename = c.getFilename();
    // TODO: implement actual file saving logic
    // think of the best way to serialize db state
    fmt::println("saving database state to '{}'", filename);
}

auto Executor::executeLoad(const LoadCommand& c) -> void {
    const std::string& filename = c.getFilename();
    // TODO: Implement actual file loading logic
    // the same thing but deserialization
    fmt::println("loading database state from '{}'", filename);
}

auto Executor::executeShow(const ShowCommand& c) -> void {
    switch (c.getShowType()) {
        case ShowCommand::ShowType::TABLES: {
            auto table_names = database_.getTableNames();
            if (table_names.empty()) {
                fmt::println("no tables in database");
                return;
            }
            fmt::println("Tables in database:");
            for (const auto& name : table_names) {
                fmt::println("- {}", name);
            }
            break;
        }
        case ShowCommand::ShowType::COLUMNS: {
            const std::string& table_name = c.getTableName();
            auto table = database_.getTable(table_name);
            if (!table) {
                throw std::runtime_error(fmt::format("table '{}' doesn't exist", table_name));
            }
            const auto& columns = table->getColumns();
            fmt::println("Columns in table '{}':", table_name);
            for (const auto& column : columns) {
                fmt::println("- {} ({})", column.getName(), dataTypeToString(column.getType()));
            }
            break;
        }
    }
}




