//
// Created by Piotrek Rybiec on 04/05/2025.
//

#ifndef COLUMN_H
#define COLUMN_H
#include <utility>

#include "data_types.h"

class Column {
private:
    std::string name;
    DataType type;
    std::vector<std::shared_ptr<Constraint>> constraints;
public:
    Column(std::string name, DataType type)
        : name(std::move(name)),
          type(type) {}

    void addConstraint(const std::shared_ptr<Constraint>& constraint);

    bool isPrimaryKey() const;
    bool isUnique() const;
    bool isNullable() const;

    const std::string& getName() const;
    DataType getType() const;
    const std::vector<std::shared_ptr<Constraint>>& getConstraints() const;
};

#endif //COLUMN_H
