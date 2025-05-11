#pragma once

#include <vector>
#include <string>

#include "Command.hpp"
#include "CommonTypes.h"
#include "Column.h"

/*
    *
    *
    *  ===== DDL =====
    *
    *
*/
class CreateCommand : public Command {
private:
    std::string table_name_;
    std::vector<Column> columns_;
    std::vector<ConstraintPtr> constraints_;

public:
    CreateCommand(std::string table_name,
                  std::vector<Column> columns,
                  std::vector<ConstraintPtr> constraints = {}) // constraints empty by default
        : Command(CommandType::CREATE),
          table_name_(std::move(table_name)),
          columns_(std::move(columns)),
          constraints_(std::move(constraints)) {}

    const std::string& getTableName() const;
    const std::vector<Column>& getColumns() const;
    const std::vector<ConstraintPtr>& getConstraints() const;

    std::string toString() const override;
};


class AlterCommand : public Command {
public:
    enum class AlterType {
        ADD,
        DROP,
        RENAME,
    };

private:
    std::string table_name_;
    AlterType alter_type_;
    std::string column_name_;
    // fields for specific alter types 
    // (drop doesnt need any as it takes from column_name_)
    std::string new_column_name_; // for rename
    Column new_column_; // for adding

public:
    // here there will be many constructors for different alters

    // for ADD
    AlterCommand(std::string table_name, Column new_column) : Command(CommandType::ALTER),
        table_name_(std::move(table_name)),
        alter_type_(AlterType::ADD),
        new_column_(std::move(new_column)) {}

    // for DROP
    //AlterCommand(std::string table_name, std::string column_name, AlterType type = AlterType::DROP) : Command(CommandType::ALTER),
    AlterCommand(std::string table_name, std::string column_name) : Command(CommandType::ALTER),
        table_name_(std::move(table_name)),
        alter_type_(AlterType::DROP),
        column_name_(std::move(column_name)),
        new_column_name_(), // for neovim lsp to leave me alone, maybe could use std::optional?
        new_column_() {} // same
    
    // for RENAME
    AlterCommand(std::string table_name, std::string column_name, std::string new_column_name) : Command(CommandType::ALTER),
        table_name_(std::move(table_name)),
        alter_type_(AlterType::RENAME),
        column_name_(std::move(column_name)),
        new_column_name_(std::move(new_column_name)),
        new_column_() {} 

    // rest
    const std::string& getTable() const;
    AlterType getAlterType() const;
    const std::string& getColumnName() const;
    const std::string& getNewColumnName() const;
    const Column& getNewColumn() const;

    std::string toString() const override;
};


class DropCommand : public Command {
private:
    std::string table_name_;

public:
    explicit DropCommand(std::string table_name) : Command(CommandType::DROP), table_name_(std::move(table_name)) {}

    const std::string& getTableName() const;
    std::string toString() const override;
};
