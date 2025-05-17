#pragma once

#include <string>
#include <fstream>
#include "Commands.hpp"
#include "Database.h"
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

public:
    explicit Executor(Database& database) : database_(database) {}

    bool execute(const std::unique_ptr<Command>& command);
};
