#pragma once

#include <string>

enum class CommandType {
    CREATE,
    ALTER,
    DROP,
    INSERT,
    UPDATE,
    DELETE,
    SELECT,
    SAVE,
    LOAD,
    SHOW,
    HELP,
    UNKNOWN
};


class Command {
private:
    CommandType type_;

public:
    explicit Command(CommandType type) : type_(type) {}
    virtual ~Command() = default;

    CommandType getType() const;
    virtual std::string toString() const = 0;
};

