#pragma once

#include <unordered_map>

#include "Column.hpp"
#include "Command.hpp"
#include "CommonTypes.hpp"
#include "Value.hpp"
#include "Database.hpp"

class Parser {
private:
    // https://stackoverflow.com/a/3114231
    std::unordered_map<std::string, void(Parser::*)()> handlers_;
    std::string query_;
    size_t pos_;
    Database& database_;

    struct ParseState {
        CommandType current_command;
        std::vector<std::string> current_columns_names;
        std::vector<std::string> current_tables_names;
        std::string current_table_name;
        std::unordered_map<std::string, Value> current_values;
        std::vector<std::vector<Value>> current_value_sets;
        std::string where_clause;
        std::vector<Column> current_columns_def;
        ConstraintList current_constraints;
        std::string filename; 
        std::string help_command; 

        auto reset () -> void {
            current_command = CommandType::UNKNOWN; // by default
            current_columns_names.clear();
            current_tables_names.clear();
            current_table_name.clear();
            current_values.clear();
            current_value_sets.clear();
            where_clause.clear();
            current_columns_def.clear();
            current_constraints.clear();
            filename.clear();
            help_command.clear();
        }
    } state_;

    void resetState();

    std::string findNextKeyword();
    std::string findNextToken();
    void skipWhitespace();
    bool isKeyword(const std::string& token) const;

    // all operations work on state, hence no return values
    // also, not sure if this is the correct form, kind of makes sense with out making an overkill
    void handleSelect();
    void handleFrom();
    void handleWhere();
    void handleCreate();
    void handleTable();
    void handleInsert();
    void handleInto();
    void handleValues();
    void handleUpdate();
    void handleSet();
    void handleDelete();
    void handleDrop();
    void handleAlter();
    void handleShow();
    void handleSave();
    void handleLoad();
    void handleHelp();

    std::unique_ptr<Command> buildCommand();
public:
    explicit Parser(Database& database) : database_(database) {
        handlers_["SELECT"] = &Parser::handleSelect;
        handlers_["FROM"] = &Parser::handleFrom;
        handlers_["WHERE"] = &Parser::handleWhere;
        handlers_["CREATE"] = &Parser::handleCreate;
        handlers_["TABLE"] = &Parser::handleTable;
        handlers_["INSERT"] = &Parser::handleInsert;
        handlers_["INTO"] = &Parser::handleInto;
        handlers_["VALUES"] = &Parser::handleValues;
        handlers_["UPDATE"] = &Parser::handleUpdate;
        handlers_["SET"] = &Parser::handleSet;
        handlers_["DELETE"] = &Parser::handleDelete;
        handlers_["DROP"] = &Parser::handleDrop;
        handlers_["ALTER"] = &Parser::handleAlter;
        handlers_["SHOW"] = &Parser::handleShow;
        handlers_["SAVE"] = &Parser::handleSave;
        handlers_["LOAD"] = &Parser::handleLoad;
        handlers_["HELP"] = &Parser::handleHelp;
    }
    std::unique_ptr<Command> parse(const std::string& query);
};

