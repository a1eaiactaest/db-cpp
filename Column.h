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
    Column() = default;
    Column(std::string name, DataType type)
        : name(std::move(name)),
          type(type) {}
    Column(std::string name, DataType type, bool nullable) : name(std::move(name)), type(type) {
        if (!nullable) {
            addConstraint(std::make_shared<NotNullConstraint>(ConstraintType::NOT_NULL, name + "_not_null", name));
        }
    }

    void addConstraint(const std::shared_ptr<Constraint>& constraint);

    bool isPrimaryKey() const;
    bool isUnique() const;
    bool isNullable() const;

    const std::string& getName() const;
    DataType getType() const;
    const std::vector<std::shared_ptr<Constraint>>& getConstraints() const;
    void setName(std::string new_name);
};

#endif //COLUMN_H
