#include "Parser.hpp"
#include "Commands.hpp"
#include "Value.h"
#include "Table.h"
#include <fmt/core.h>

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
auto Parser::findNextToken() -> std::string {
    skipWhitespace();

    if (pos_ >= query_.length()) {
        return ""; // maybe throw an error instead?
    }

    std::string token;
    bool in_quotes = false;
    while (pos_ < query_.length()) {
        char c = query_[pos_];

        if (c == '\'' || c == '"') {
            in_quotes = !in_quotes; // flip
            token += c;
        } else if (!in_quotes && (std::isspace(c) || delimiters.find(c) != std::string::npos)) { // maybe a cast on char?
            if (!token.empty()) return token;
            if (c != ' ' && c != '\t' && c != '\n') {
                token = c;
                pos_++;
                return token;
            }
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
            if (tok.empty() || tok == "WHERE" || tok == ";") break;
            if (tok != ",") {
                state_.current_tables_names.push_back(tok);
                state_.current_table_name = tok;
            }
        }
    }
}

auto Parser::handleWhere() -> void {
    std::ostringstream where_clause_os;
    while (pos_ < query_.length()) {
        std::string tok = findNextToken();
        if (tok.empty() || tok == ";") break;
        where_clause_os << tok << " ";
    }
    state_.where_clause = where_clause_os.str();
    if (state_.where_clause.empty()) {
        state_.where_clause.pop_back();
    }
}

auto Parser::handleCreate() -> void {
    state_.current_command = CommandType::CREATE;
}

auto Parser::handleTable() -> void {
    if (state_.current_command == CommandType::CREATE) {
        state_.current_table_name = findNextToken();
        
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

            // Create column and add it to the list
            Column col(col_name, type);
            state_.current_columns_def.push_back(col);

            // Check for next column or end of definition
            tok = findNextToken();
            if (tok == ")") break;
            if (tok != ",") {
                throw std::runtime_error("expected ',' or ')' after column definition");
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

// helpers, not sure if they will be sufficient
auto isString(std::string& value) -> bool {
    return value.front() == '\'' || value.front() == '"';
}

auto isBool(std::string& value) -> bool {
    return value == "true" || value == "false";
}

auto isDouble(std::string& value) -> bool {
    return value.find('.') != std::string::npos;
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
            // Skip commas, delete this later
        } else {
            // Parse the value
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

        // Check for end of statement
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
        default:
            return nullptr;
    }
}
