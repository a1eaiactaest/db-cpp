#pragma once

#include <vector>
#include <string>

#include "Command.hpp"
#include "CommonTypes.h"
#include "Column.h"
#include "Value.h"

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

/*
    *
    *
    *  ===== DML =====
    *
    *
*/
class InsertCommand : public Command {
private:
    std::string table_name_;
    std::vector<std::string> column_names_;
    std::vector<std::vector<Value>> values_;

public:
    InsertCommand(std::string table_name,
                  std::vector<std::string> column_names,
                  std::vector<std::vector<Value>> values)
            : Command(CommandType::INSERT),
              table_name_(std::move(table_name)),
              column_names_(std::move(column_names)),
              values_(std::move(values)) {}

    const std::string& getTableName() const;
    const std::vector<std::string> getColumnNames() const;
    const std::vector<std::vector<Value>>& getValues() const;

    std::string toString() const override;
};


class UpdateCommand : public Command {
private:
    std::string table_name_;
    std::unordered_map<std::string, Value> column_values_;
    std::string where_clause_; // if where is not specified then all records in table are updated

public:
    UpdateCommand(std::string table_name,
                  std::unordered_map<std::string, Value> column_values,
                  std::string where_clause = "")
            : Command(CommandType::UPDATE),
              table_name_(std::move(table_name)),
              column_values_(std::move(column_values)),
              where_clause_(std::move(where_clause)) {}

    const std::string& getTableName() const;
    const std::unordered_map<std::string, Value>& getColumnValues() const;
    const std::string& getWhereClause() const;

    std::string toString() const override;
};

class DeleteCommand : public Command {
private:
    std::string table_name_;
    std::string where_clause_; // same as where clause in update command

public:
    DeleteCommand(std::string table_name, std::string where_clause = "")
         : Command(CommandType::DELETE),
           table_name_(std::move(table_name)),
           where_clause_(std::move(where_clause)) {}

    const std::string getTableName() const;
    const std::string getWhereClause() const;

    std::string toString() const override;
};


/*
    *
    *
    *  ===== DQL =====
    *
    *
*/
class SelectCommand : public Command {
private:
    std::vector<std::string> column_names_;
    std::vector<std::string> table_names_;
    std::string where_clause_;

public:
    SelectCommand(std::vector<std::string> column_names, 
                  std::vector<std::string> table_names,
                  std::string where_clause = "")
        : Command(CommandType::SELECT),
          column_names_(std::move(column_names)),
          table_names_(std::move(table_names)),
          where_clause_(std::move(where_clause)) {}

    const std::vector<std::string>& getColumnNames() const;
    const std::vector<std::string>& getTableNames() const;
    const std::string& getWhereClause() const;

    std::string toString() const override;
};

/*
    *
    *
    *  ===== File operations =====
    *
    *
*/

class SaveCommand : public Command {
private:
    std::string filename_;

public:
    explicit SaveCommand(std::string filename) : Command(CommandType::SAVE), filename_(std::move(filename)) {}

    const std::string& getFilename() const;
    std::string toString() const override;
};

// TODO: maybe add the destination table name as a parameter?
class LoadCommand : public Command {
private:
    std::string filename_;

public:
    explicit LoadCommand(std::string filename) : Command(CommandType::LOAD), filename_(std::move(filename)) {}

    const std::string& getFilename() const;
    std::string toString() const override;
};

class ShowCommand : public Command {
public:
    enum class ShowType {
        TABLES,
        COLUMNS
    };

private:
    ShowType show_type_;
    std::string table_name_;

public:
    explicit ShowCommand(ShowType show_type = ShowType::TABLES) : Command(CommandType::SHOW), show_type_(show_type) {}

    ShowCommand(std::string table_name) 
        : Command(CommandType::SHOW),
          show_type_(ShowType::COLUMNS),
          table_name_(std::move(table_name)) {}

    ShowType getShowType() const;
    const std::string& getTableName() const;

    std::string toString() const override;
};
