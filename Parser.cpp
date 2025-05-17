#include "Parser.hpp"
#include "Commands.hpp"
#include "Value.h"
#include "Table.h"
#include <fmt/core.h>

static auto isString(const std::string& value) -> bool {
    return !value.empty() && (value.front() == '\'' || value.front() == '"');
}

static auto isBool(const std::string& value) -> bool {
    return value == "true" || value == "false";
}

static auto isDouble(const std::string& value) -> bool {
    return value.find('.') != std::string::npos;
}

auto Parser::parse(const std::string& query) -> std::unique_ptr<Command> {
    query_ = query;
    pos_ = 0;
    resetState();

    while (pos_ < query_.length()) {
        auto keyword = findNextKeyword();
        if (keyword.empty()) break;

        auto handler = handlers_.find(keyword);
        if (handler != handlers_.end()) {
            //handler->second(*this, keyword);
            //https://stackoverflow.com/a/3114231
            (this->*handler->second)();
        }
    }
    return buildCommand();
}

auto Parser::skipWhitespace() -> void {
    while (pos_ < query_.length() && std::isspace(query_[pos_])) {
        pos_++;
    }
}

static constexpr std::string delimiters = ",();";
static constexpr std::string operators = "=<>!";

auto Parser::findNextToken() -> std::string {
    skipWhitespace();

    if (pos_ >= query_.length()) {
        return ""; // maybe throw an error instead?
    }

    std::string token;
    bool in_quotes = false;

    // Special case for operators
    char c = query_[pos_];
    if (c == '=' || c == '<' || c == '>' || c == '!') {
        token += c;
        pos_++;

        if (pos_ < query_.length() && query_[pos_] == '=') {
            token += '=';
            pos_++;
        }
        return token;
    }
    
    // Normal token parsing
    while (pos_ < query_.length()) {
        c = query_[pos_];

        if (c == '\'' || c == '"') {
            in_quotes = !in_quotes; // flip
            token += c;
        } else if (!in_quotes && (std::isspace(c) || delimiters.find(c) != std::string::npos)) {
            if (!token.empty()) return token;
            if (c != ' ' && c != '\t' && c != '\n') {
                token = c;
                pos_++;
                return token;
            }
        } else if (!in_quotes && (c == '=' || c == '<' || c == '>' || c == '!') && !token.empty()) {
            // If we encounter an operator and we already have a token, return the token first
            return token;
        } else {
            token += c;
        }
        pos_++;
    }
    return token;
}

// this will always return capitalized
auto Parser::findNextKeyword() -> std::string {
    std::string tok = findNextToken();
    if (tok.empty()) return "";
    std::transform(tok.begin(), tok.end(), tok.begin(), [](unsigned char c){
        return std::toupper(c);
    });
    return isKeyword(tok) ? tok : "";
}

auto Parser::isKeyword(const std::string& token) const -> bool {
    return handlers_.contains(token);
}

auto Parser::handleSelect() -> void {
    state_.current_command = CommandType::SELECT;
    state_.current_tables_names.clear(); 
    state_.current_columns_names.clear(); 

    while (pos_ < query_.length()) {
        auto tok = findNextToken();
        if (tok.empty()) break;
        if (tok == "FROM") {
            handleFrom();
            break;
        }
        if (tok != ",") {
            state_.current_columns_names.push_back(tok);
        }
    }
}

auto Parser::handleFrom() -> void {
    // only process table names if we're in a SELECT command, otherwise won't work lmao
    if (state_.current_command == CommandType::SELECT) {
        while (pos_ < query_.length()) {
            auto tok = findNextToken();
            if (tok.empty() || tok == ";") break;
            // if WHERE is found, process the WHERE clause and then break
            if (tok == "WHERE") {
                handleWhere();
                break;
            }
            if (tok != ",") {
                state_.current_tables_names.push_back(tok);
                state_.current_table_name = tok;
            }
        }

        // if we have a * in the column list, replace it with all column names
        if (!state_.current_tables_names.empty() && 
            state_.current_columns_names.size() == 1 && 
            state_.current_columns_names[0] == "*") {
            fmt::print("Looking up table: {}\n", state_.current_table_name);
            auto table = database_.getTable(state_.current_table_name);
            if (table) {
                fmt::print("Found table, getting columns\n");
                state_.current_columns_names.clear();
                for (const auto& col : table->getColumns()) {
                    fmt::print("Adding column: {}\n", col.getName());
                    state_.current_columns_names.push_back(col.getName());
                }
            } else {
                throw std::runtime_error(fmt::format("table '{}' not found", state_.current_table_name));
            }
        }
    }
}

auto Parser::handleWhere() -> void {
    std::string column_name = findNextToken();
    if (column_name.empty()) {
        throw std::runtime_error("missing column name in WHERE clause");
    }

    std::string operator_str = findNextToken();
    if (operator_str.empty()) {
        throw std::runtime_error("missing operator in WHERE clause");
    }

    std::string value_str = findNextToken();
    if (value_str.empty()) {
        throw std::runtime_error("missing value in WHERE clause");
    }

    // Validate operator
    if (operator_str != "=" && operator_str != "!=" && 
        operator_str != "<" && operator_str != ">" && 
        operator_str != "<=" && operator_str != ">=") {
        throw std::runtime_error(fmt::format("unsupported operator in WHERE clause: {}", operator_str));
    }

    state_.where_clause = fmt::format("{} {} {}", column_name, operator_str, value_str);
}

auto Parser::handleCreate() -> void {
    state_.current_command = CommandType::CREATE;
}

auto Parser::handleTable() -> void {
    if (state_.current_command == CommandType::CREATE) {
        state_.current_table_name = findNextToken();
        
        // Check if table name is empty
        if (state_.current_table_name.empty()) {
            throw std::runtime_error("table name cannot be empty");
        }
        
        // Expect opening parenthesis for column definitions
        auto tok = findNextToken();
        if (tok != "(") {
            throw std::runtime_error("expected '(' after table name");
        }

        // Parse column definitions
        while (pos_ < query_.length()) {
            auto col_name = findNextToken();
            if (col_name.empty()) break;
            if (col_name == ")") break;

            auto type_name = findNextToken();
            if (type_name.empty()) {
                throw std::runtime_error("expected data type after column name");
            }

            // Convert type name to DataType
            DataType type;
            if (type_name == "INT" || type_name == "INTEGER") {
                type = DataType::INTEGER;
            } else if (type_name == "VARCHAR" || type_name == "STRING") {
                type = DataType::STRING;
            } else if (type_name == "BOOLEAN") {
                type = DataType::BOOLEAN;
            } else if (type_name == "FLOAT" || type_name == "DOUBLE") {
                type = DataType::FLOAT;
            } else {
                throw std::runtime_error(fmt::format("unsupported data type: {}", type_name));
            }

            Column col(col_name, type);
            bool done = false;
            while (!done) {
                auto constraint_tok = findNextToken();
                if (constraint_tok.empty()) {
                    done = true;
                } else if (constraint_tok == ",") {
                    done = true;
                } else if (constraint_tok == ")") {
                    // end of all column definitions
                    pos_--; // push back the token so it's read again in the outer loop
                    done = true;
                } else {
                    // parse constraint
                    std::string upper_constraint = constraint_tok;
                    std::transform(upper_constraint.begin(), upper_constraint.end(), 
                                  upper_constraint.begin(), ::toupper);

                    if (upper_constraint == "NOT" || upper_constraint == "NOT NULL") {
                        // handle NOT NULL constraint
                        if (upper_constraint == "NOT") {
                            // look for NULL keyword
                            auto null_tok = findNextToken();
                            std::string upper_null = null_tok;
                            std::transform(upper_null.begin(), upper_null.end(), 
                                          upper_null.begin(), ::toupper);
                            if (upper_null != "NULL") {
                                throw std::runtime_error("expected NULL after NOT");
                            }
                        }
                        auto constraint = std::make_shared<NotNullConstraint>(
                            ConstraintType::NOT_NULL, 
                            col_name + "_not_null", 
                            col_name);
                        col.addConstraint(constraint);
                        state_.current_constraints.push_back(constraint);
                    } else if (upper_constraint == "UNIQUE") {
                        // handle UNIQUE constraint
                        auto constraint = std::make_shared<UniqueConstraint>(
                            col_name + "_unique", 
                            std::vector<std::string>{col_name});
                        col.addConstraint(constraint);
                        state_.current_constraints.push_back(constraint);
                    } else if (upper_constraint == "PRIMARY" || upper_constraint == "PRIMARY KEY" || upper_constraint == "PK") {
                        // handle PRIMARY KEY constraint
                        if (upper_constraint == "PRIMARY") {
                            // look for KEY keyword
                            auto key_tok = findNextToken();
                            std::string upper_key = key_tok;
                            std::transform(upper_key.begin(), upper_key.end(), 
                                         upper_key.begin(), ::toupper);

                            if (upper_key != "KEY") {
                                throw std::runtime_error("expected KEY after PRIMARY");
                            }
                        }

                        auto constraint = std::make_shared<PrimaryKeyConstraint>(
                            col_name + "_pk", 
                            std::vector<std::string>{col_name});
                        col.addConstraint(constraint);
                        state_.current_constraints.push_back(constraint);
                    } else if (upper_constraint == "DEFAULT") {
                        // handle DEFAULT value constraint
                        auto value_tok = findNextToken();
                        Value default_value;
                        if (isString(value_tok)) {
                            default_value = Value(value_tok.substr(1, value_tok.length() - 2));
                        } else if (isBool(value_tok)) {
                            default_value = Value(value_tok == "true");
                        } else if (isDouble(value_tok)) {
                            default_value = Value(std::stod(value_tok));
                        } else {
                            // assume integer
                            default_value = Value(std::stoi(value_tok));
                        }
                        auto constraint = std::make_shared<DefaultConstraint>(
                            col_name + "_default", 
                            col_name, 
                            default_value);
                        col.addConstraint(constraint);
                        state_.current_constraints.push_back(constraint);
                    } else {
                        throw std::runtime_error(fmt::format("unknown constraint: {}", constraint_tok));
                    }
                }
            }
            state_.current_columns_def.push_back(col);

            // if we ended on a ')', were doooone heeer
            if (pos_ < query_.length() && query_[pos_] == ')') {
                findNextToken(); // eat the ')' and break
                break;
            }
        }
    }
}

auto Parser::handleInsert() -> void {
    state_.current_command = CommandType::INSERT;
}

auto Parser::handleInto() -> void {
    if (state_.current_command == CommandType::INSERT) {
        state_.current_table_name = findNextToken();

        // column names in parentheses
        auto saved_pos = pos_;
        auto tok = findNextToken();
        if (tok != "(") {
            pos_ = saved_pos; // backtrack if no column list
        } else {
            while (pos_ < query_.length()) {
                tok = findNextToken();
                if (tok == ")") break;
                if (tok != ",") {
                    state_.current_columns_names.push_back(tok);
                }
            }
        }
    }
}

auto Parser::handleValues() -> void{
    if (state_.current_command != CommandType::INSERT) {
        throw std::runtime_error("VALUES keyword found outside INSERT statement!");
    }

    // Skip the VALUES keyword
    auto tok = findNextToken();
    if (tok != "(") {
        throw std::runtime_error("expected '(' after VALUES");
    }

    auto value_sets = std::vector<std::vector<Value>>();
    auto current_set = std::vector<Value>();

    // if no column names were specified, use all columns from the table
    if (state_.current_columns_names.empty()) {
        fmt::print("Looking up table: {}\n", state_.current_table_name);
        auto table = database_.getTable(state_.current_table_name);
        if (table) {
            fmt::print("Found table, getting columns\n");
            for (const auto& col : table->getColumns()) {
                fmt::print("Adding column: {}\n", col.getName());
                state_.current_columns_names.push_back(col.getName());
            }
            fmt::print("Total columns added: {}\n", state_.current_columns_names.size());
        } else {
            fmt::print("Table not found!\n");
        }
    }

    while (pos_ < query_.length()) {
        tok = findNextToken();
        if (tok.empty()) break;

        if (tok == "(") {
            current_set.clear();
        } else if (tok == ")") {
            if (!current_set.empty()) {
                value_sets.push_back(current_set);
            }
        } else if (tok == ",") {
            // skip commas, delete this later
        } else {
            // parse the value
            if (isString(tok)) {
                current_set.push_back(Value(tok.substr(1, tok.length()-2)));
            } else if (isBool(tok)) {
                current_set.push_back(Value(tok == "true"));
            } else if (isDouble(tok)) {
                current_set.push_back(Value(std::stod(tok)));
            } else {
                current_set.push_back(Value(std::stoi(tok)));
            }
        }

        if (tok == ";" || pos_ >= query_.length()) {
            break;
        }
    }

    state_.current_value_sets = value_sets;
}

auto Parser::handleUpdate() -> void {
    state_.current_command = CommandType::UPDATE;
    state_.current_table_name = findNextToken(); // table name after UPDATE
}

auto Parser::handleSet() -> void {
    if (state_.current_command != CommandType::UPDATE) {
        throw std::runtime_error("SET found outside UPDATE statement!");
    }

    auto in_quotes = false;
    auto cur_val_s = std::string();
    auto cur_col = std::string();
    auto expecting_value = false;

    auto append_val = [&](){
        if (!cur_val_s.empty()) {
            Value val;
            if (isString(cur_val_s)) {
                val = Value(cur_val_s.substr(1, cur_val_s.length()-2));
            } else if (isBool(cur_val_s)) {
                val = Value(cur_val_s == "true");
            } else if (isDouble(cur_val_s)) {
                val = Value(std::stod(cur_val_s));
            } else {
                val = Value(std::stoi(cur_val_s));
            }
            state_.current_values[cur_col] = val;
            cur_val_s.clear();
            cur_col.clear();
            expecting_value = false;
        }
    };

    while (pos_ < query_.length()) {
        char c = query_[pos_];
        if (c == '\'' || c == '"') {
            in_quotes = !in_quotes;
            cur_val_s += c;
        } else if (c == '=' && !in_quotes && !expecting_value) {
            expecting_value = true;
        } else if (c == ',' && !in_quotes) {
            append_val();
        } else if (!std::isspace(c) || in_quotes) {
            if (expecting_value) {
                cur_val_s += c;
            } else {
                cur_col += c;
            }
        }
        pos_++;
        // 'W' for 'W'HERE
        if (!in_quotes && (pos_ >= query_.length() || query_[pos_] == ';' || query_[pos_] == 'W')) {
            append_val();
            break;
        }
    }
}

auto Parser::handleDelete() -> void {
    state_.current_command = CommandType::DELETE;
    auto tok = findNextToken(); // skip FROM
    if (tok == "FROM") {
        state_.current_table_name = findNextToken(); // get table name after FROM
    }
}

auto Parser::handleDrop() -> void {
    state_.current_command = CommandType::DROP;
    auto tok = findNextToken();
    if (tok == "TABLE") {
        state_.current_table_name = findNextToken();
    } else {
        // If "TABLE" keyword is omitted, use the token as table name
        state_.current_table_name = tok;
    }
}

auto Parser::handleShow() -> void {
    state_.current_command = CommandType::SHOW;
    auto tok = findNextToken();
    if (tok == "TABLES") {
        state_.current_command = CommandType::SHOW;
    } else if (tok == "COLUMNS") {
        tok = findNextToken(); // skip FROM
        if (tok == "FROM") {
            state_.current_table_name = findNextToken();
        }
    }
}

auto Parser::handleSave() -> void {
    state_.current_command = CommandType::SAVE;
    std::string filename = findNextToken();

    if (!filename.empty() && (filename.front() == '\'' || filename.front() == '"')) {
        if (filename.size() >= 2 && (filename.back() == '\'' || filename.back() == '"')) {
            // remove surrounding quotes
            filename = filename.substr(1, filename.size() - 2);
        }
    }
    state_.filename = filename;
}

auto Parser::handleLoad() -> void {
    state_.current_command = CommandType::LOAD;
    std::string filename = findNextToken();

    if (!filename.empty() && (filename.front() == '\'' || filename.front() == '"')) {
        if (filename.size() >= 2 && (filename.back() == '\'' || filename.back() == '"')) {
            filename = filename.substr(1, filename.size() - 2);
        }
    }
    
    state_.filename = filename;
}

auto Parser::handleHelp() -> void {
    state_.current_command = CommandType::HELP;
    // check for specific command to get help for
    skipWhitespace();
    if (pos_ < query_.length() && query_[pos_] != ';') {
        state_.help_command = findNextToken();
        std::transform(state_.help_command.begin(), state_.help_command.end(), 
                      state_.help_command.begin(), ::toupper);
    }
}

auto Parser::resetState() -> void {
    state_ = ParseState();
}

auto Parser::buildCommand() -> std::unique_ptr<Command> {
    switch (state_.current_command) {
        case CommandType::SELECT:
            if (state_.current_tables_names.empty()) {
                throw std::runtime_error("no table specified in SELECT");
            }
            if (state_.current_columns_names.empty()) {
                state_.current_columns_names.push_back("*");  // Default to all columns if none specified
            }
            return std::make_unique<SelectCommand>(
                state_.current_columns_names,
                state_.current_tables_names,
                state_.where_clause
            );
        case CommandType::CREATE:
            return std::make_unique<CreateCommand>(
                state_.current_table_name,
                state_.current_columns_def,
                state_.current_constraints
            );
        case CommandType::DROP:
            return std::make_unique<DropCommand>(state_.current_table_name);
        case CommandType::INSERT:
            return std::make_unique<InsertCommand>(
                state_.current_table_name,
                state_.current_columns_names,
                state_.current_value_sets
            );
        case CommandType::UPDATE:
            return std::make_unique<UpdateCommand>(
                state_.current_table_name,
                state_.current_values,
                state_.where_clause
            );
        case CommandType::DELETE:
            return std::make_unique<DeleteCommand>(
                state_.current_table_name,
                state_.where_clause
            );
        case CommandType::SAVE:
            return std::make_unique<SaveCommand>(state_.filename);
        case CommandType::LOAD:
            return std::make_unique<LoadCommand>(state_.filename);
        case CommandType::SHOW:
            if (state_.current_table_name.empty()) {
                return std::make_unique<ShowCommand>(ShowCommand::ShowType::TABLES);
            } else {
                return std::make_unique<ShowCommand>(state_.current_table_name);
            }
        case CommandType::HELP:
            return std::make_unique<HelpCommand>(state_.help_command);
        default:
            return nullptr;
    }
}
