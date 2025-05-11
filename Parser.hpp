#pragma once


#include <string>
#include <vector>

#include "Command.hpp"
#include "CommonTypes.h"

// forward declarations
class Command;
class CreateCommand;
class AlterCommand;
class DropCommand;
class InsertCommand;
class UpdateCommand;
class DeleteCommand;
class SelectCommand;
class SaveCommand;
class LoadCommand;
class ShowCommand;

class Parser {
private:

    CommandPtr parseCreate(const std::vector<std::string>& tokens);
    CommandPtr parseAlter(const std::vector<std::string>& tokens);
    CommandPtr parseDrop(const std::vector<std::string>& tokens);
    CommandPtr parseInsert(const std::vector<std::string>& tokens);
    CommandPtr parseUpdate(const std::vector<std::string>& tokens);
    CommandPtr parseDelete(const std::vector<std::string>& tokens);
    CommandPtr parseSelect(const std::vector<std::string>& tokens);
    CommandPtr parseSave(const std::vector<std::string>& tokens);
    CommandPtr parseLoad(const std::vector<std::string>& tokens);
    CommandPtr parseShow(const std::vector<std::string>& tokens);

    std::vector<std::string> tokenize(const std::string& sql);


public:
    Parser() = default;
    //~Parser() = default;
    CommandPtr parse(const std::string& sql);
};
