//
// Created by Piotrek Rybiec on 04/05/2025.
//

#include "Constraint.h"

Constraint::~Constraint() = default;

ConstraintType Constraint::getType() const {
    return type;
}

std::string Constraint::toString() const {
    return name;
}




