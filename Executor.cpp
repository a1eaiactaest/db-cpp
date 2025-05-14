#include <iostream>
#include <fmt/base.h>
#include <fmt/ostream.h>

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
