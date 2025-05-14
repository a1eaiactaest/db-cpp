#include "Parser.hpp"
#include "Commands.hpp"
#include "Value.h"

Parser::Parser() {
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
            // Process the FROM part immediately
            while (pos_ < query_.length()) {
                auto table_tok = findNextToken();
                if (table_tok.empty() || table_tok == "WHERE" || table_tok == ";") break;
                if (table_tok != ",") {
                    state_.current_tables_names.push_back(table_tok);
                    state_.current_table_name = table_tok;
                }
            }
            break;
        }
        if (tok != ",") {
            state_.current_columns_names.push_back(tok);
        }
    }
}

auto Parser::handleFrom() -> void {
    // FROM is now handled in handleSelect
    return;
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
    }
}

auto Parser::handleInsert() -> void {
    state_.current_command = CommandType::INSERT;
}

auto Parser::handleInto() -> void {
    if (state_.current_command == CommandType::INSERT) {
        state_.current_table_name = findNextToken();
        
        // column names in parentheses
        auto tok = findNextToken();
        if (tok == "(") {
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

    auto value_sets = std::vector<std::vector<Value>>();
    auto current_set = std::vector<Value>();
    auto in_parentheses = false;
    auto in_quotes = false;
    auto cur_val_s = std::string();

    auto append_val = [&](){
        if (!cur_val_s.empty()) {
            if (isString(cur_val_s)) {
                current_set.push_back(
                    Value(cur_val_s.substr(1, cur_val_s.length()-2))
                );
            } else if (isBool(cur_val_s)) {
                current_set.push_back(Value(cur_val_s == "true"));
            } else if (isDouble(cur_val_s)) {
                current_set.push_back(Value(std::stod(cur_val_s)));
            } else {
                current_set.push_back(Value(std::stoi(cur_val_s)));
            } // TODO: maybe check for date, and datetime altough they can be treated as strings for now
            cur_val_s.clear();
        }
    };

    while (pos_ < query_.length()) {
        char c = query_[pos_];

        if (c == '(') {
            if (!in_parentheses) {
                in_parentheses = true;
                current_set.clear();
            } else {
                throw std::runtime_error("nested parentheses in VALUES!");
            }
        } else if (c == ')') {
            if (in_parentheses) {
                in_parentheses = false;
                append_val();
                value_sets.push_back(current_set);
            } else {
                throw std::runtime_error("unexpected closing parenthesis in VALUES!");
            }
        } else if (c == '\'' || c == '"') {
            in_quotes = !in_quotes;
            cur_val_s += c;
        } else if (c == ',' && !in_quotes) {
            if (in_parentheses) {
                append_val();
            }
        } else if (!std::isspace(c) || in_quotes) {
            cur_val_s += c;
        }

        pos_++;

        // end of VALUES 
        if (!in_parentheses && !in_quotes && (pos_ >= query_.length() || query_[pos_] == ';')) {
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
