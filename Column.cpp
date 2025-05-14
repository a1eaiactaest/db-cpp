//
// Created by Piotrek Rybiec on 05/05/2025.
//

#include <algorithm>

#include "Column.h"
#include "Constraint.h"

auto Column::addConstraint(const std::shared_ptr<Constraint>& constraint) -> void {
    constraints.push_back(constraint);
}

auto Column::isPrimaryKey() const -> bool {
    /*
    for (const auto& c : constraints) {
        if (c->getType() == ConstraintType::PRIMARY_KEY) {
            return true;
        }
    }
    return false;
    */
    return std::ranges::any_of(constraints, [](const auto& c) {
        return c->getType() == ConstraintType::UNIQUE;
    });
}

auto Column::isUnique() const -> bool {
    return std::ranges::any_of(constraints, [](const auto& c) {
        return c->getType() == ConstraintType::UNIQUE;
    });
}

auto Column::isNullable() const -> bool {
    return std::ranges::any_of(constraints, [](const auto& c) {
        return c->getType() == ConstraintType::NOT_NULL;
    });
}

auto Column::getName() const -> const std::string& {
    return name;
}

auto Column::getType() const -> DataType {
    return type;
}

auto Column::getConstraints() const -> const std::vector<std::shared_ptr<Constraint>>& {
    return constraints;
}

auto Column::setName(std::string new_name) -> void {
    name = std::move(new_name);
}
