#pragma once

#include <unordered_map>

#include "Column.h"
#include "Command.hpp"
#include "CommonTypes.h"

class Parser {
private:
    std::unordered_map<std::string, std::function<void(Parser&, const std::string&)>> handlers_;
    std::string query_;
    size_t pos_;

    struct ParseState {
        CommandType current_command;
        std::vector<std::string> current_columns_names;
        std::vector<std::string> current_tables_names;
        std::string current_table_name;
        std::unordered_map<std::string, Value> current_values;
        std::string where_clause;
        std::vector<Column> current_columns_def;
        ConstraintList current_constraints;

        auto reset () -> void {
            current_command = CommandType::UNKNOWN; // by default
            current_columns_names.clear();
            current_tables_names.clear();
            current_table_name.clear();
            current_values.clear();
            where_clause.clear();
            current_columns_def.clear();
            current_constraints.clear();
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

    std::unique_ptr<Command> buildCommand();
public:
    Parser();
    std::unique_ptr<Command> parse(const std::string& query);
};

