#include "Parser.hpp"

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
            handler->second(*this, keyword);
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
        return ""; // maybe throw an error?
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
