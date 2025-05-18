#pragma once

#include <memory>
#include <string>
#include <fstream>
#include "Command.hpp"
#include "Commands.hpp"
#include "Database.hpp"
#include "Parser.hpp"

class Executor {
private:
    Database& database_;

    void executeSelect(const SelectCommand& command);
    void executeCreate(const CreateCommand& command);
    void executeDrop(const DropCommand& command);
    void executeInsert(const InsertCommand& command);
    void executeUpdate(const UpdateCommand& command);
    void executeDelete(const DeleteCommand& command);
    void executeAlter(const AlterCommand& command);
    void executeSave(const SaveCommand& command);
    void executeLoad(const LoadCommand& command);
    void executeShow(const ShowCommand& command);
    void executeHelp(const HelpCommand& command);

    bool evaluateWhereCondition(const Row& row, const std::string& where_clause);

public:
    explicit Executor(Database& database) : database_(database) {}

    bool execute(const std::unique_ptr<Command>& command);
};
