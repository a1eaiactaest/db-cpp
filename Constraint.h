//
// Created by Piotrek Rybiec on 04/05/2025.
//

#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include "data_types.h"

class Constraint {
protected:
    ConstraintType type;
    std::string name;
public:
    Constraint(ConstraintType type, std::string name) : type(type), name(std::move(name)) {}
    virtual ~Constraint();
    ConstraintType getType() const;
    const std::string& getName() const { return name; }
    virtual std::string toString() const;
};

class PrimaryKeyConstraint : public Constraint {
private:
    std::vector<std::string> column_names;
public:
    PrimaryKeyConstraint(std::string name, std::vector<std::string> columns)
        : Constraint(ConstraintType::PRIMARY_KEY, std::move(name)),
        column_names(std::move(columns)) {}

    const std::vector<std::string>& getColumnNames() const;
    std::string toString() const override;
};



#endif //CONSTRAINT_H
