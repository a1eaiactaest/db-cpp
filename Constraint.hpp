//
// Created by Piotrek Rybiec on 04/05/2025.
//

#ifndef CONSTRAINT_H
#define CONSTRAINT_H

#include "data_types.hpp"
#include "Value.hpp"
#include "Row.hpp"

class Database; // forward declaration, remvoe later

class Constraint {
protected:
    ConstraintType type;
    std::string name;
public:
    virtual ~Constraint() = default;
    Constraint(ConstraintType type, std::string name) : type(type), name(std::move(name)) {}
    ConstraintType getType() const;
    const std::string& getName() const;
    virtual std::string toString() const;
    virtual bool validate(const Row& row, const Table& table) const = 0;
    virtual bool validate(const Row& row, const Table& table, const Database& base) const = 0;
};

class PrimaryKeyConstraint : public Constraint {
private:
    std::vector<std::string> column_names;
public:
    PrimaryKeyConstraint(const std::string name, const std::vector<std::string>& columns)
        : Constraint(ConstraintType::PRIMARY_KEY, std::move(name)),
          column_names(std::move(columns)) {}

    const std::vector<std::string>& getColumnNames() const;
    std::string toString() const override;
    bool validate(const Row& row, const Table& table) const override;
    bool validate(const Row& row, const Table& table, const Database& base) const override;
};

class ForeignKeyConstraint : public Constraint {
private:
    std::string column_name;
    std::string ref_table;
    std::string ref_column;
public:
    ForeignKeyConstraint(const std::string name, const std::string& column_name,
        const std::string& ref_table, const std::string& ref_column)
        : Constraint(ConstraintType::FOREIGN_KEY, name),
          column_name(column_name),
          ref_table(ref_table),
          ref_column(ref_column) {}

    const std::string& getColumnName() const;
    const std::string& getRefTable() const;
    const std::string& getRefColumn() const;
    std::string toString() const override;
    bool validate(const Row& row, const Table& table) const override;
    bool validate(const Row& row, const Table& table, const Database& base) const override;
};

class UniqueConstraint : public Constraint {
private:
    std::vector<std::string> column_names;
public:
    UniqueConstraint(const std::string name, const std::vector<std::string> &column_names)
        : Constraint(ConstraintType::UNIQUE, name),
          column_names(column_names) {}

    const std::vector<std::string>& getColumnNames() const;
    std::string toString() const override;
    bool validate(const Row& row, const Table& table) const override;
    bool validate(const Row& row, const Table& table, const Database& base) const override;
};

class NotNullConstraint : public Constraint {
private:
    std::string column_name;
public:
    NotNullConstraint(ConstraintType type, const std::string name, const std::string& column_name)
        : Constraint(ConstraintType::NOT_NULL, name),
          column_name(column_name) {}

    const std::string& getColumnName() const;
    std::string toString() const override;
    bool validate(const Row& row, const Table& table) const override;
    bool validate(const Row& row, const Table& table, const Database& base) const override;
};

class DefaultConstraint : public Constraint {
private:
    std::string column_name;
    Value default_value;
public:
    DefaultConstraint(const std::string name, const std::string &column_name,
        const Value &default_value)
        : Constraint(ConstraintType::DEFAULT, name),
          column_name(column_name),
          default_value(default_value) {}

    const std::string& getColumnName() const;
    const Value& getDefaultValue() const;
    std::string toString() const override;
    bool validate(const Row& row, const Table& table) const override {
        // Default constraints are always valid as they only provide default values
        return true;
    }
    bool validate(const Row& row, const Table& table, const Database& base) const override {
        // Default constraints are always valid as they only provide default values
        return true;
    }
};



#endif //CONSTRAINT_H
