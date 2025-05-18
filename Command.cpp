#include "Command.hpp"
#include "CommonTypes.hpp"

auto Command::getType() const -> CommandType {
    return type_;
}

// no toString because every command child class will implement it's own
