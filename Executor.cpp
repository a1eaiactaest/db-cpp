#include <iostream>
#include <fmt/base.h>
#include <fmt/ostream.h>
#include <fmt/format.h>
#include <fstream>
#include <map>

#include "Executor.hpp"
#include "Table.h"
#include "Parser.hpp"
#include "data_types.h"

auto Executor::execute(const std::unique_ptr<Command>& command) -> bool {
    if (!command) {
        fmt::print(std::cerr, "err: null command recieved");
        return false;
    }

    try {
        switch (command->getType()) {
            case CommandType::SELECT:
                executeSelect(static_cast<const SelectCommand&>(*command));
                break;
            case CommandType::CREATE:
                executeCreate(static_cast<const CreateCommand&>(*command));
                break;
            case CommandType::DROP:
                executeDrop(static_cast<const DropCommand&>(*command)); break;
            case CommandType::INSERT:
                executeInsert(static_cast<const InsertCommand&>(*command));
                break;
            case CommandType::UPDATE:
                executeUpdate(static_cast<const UpdateCommand&>(*command));
                break;
            case CommandType::DELETE:
                executeDelete(static_cast<const DeleteCommand&>(*command));
                break;
            case CommandType::ALTER:
                executeAlter(static_cast<const AlterCommand&>(*command));
                break;
            case CommandType::SAVE:
                executeSave(static_cast<const SaveCommand&>(*command));
                break;
            case CommandType::LOAD:
                executeLoad(static_cast<const LoadCommand&>(*command));
                break;
            case CommandType::SHOW:
                executeShow(static_cast<const ShowCommand&>(*command));
                break;
            case CommandType::HELP:
                executeHelp(static_cast<const HelpCommand&>(*command));
                break;
            default:
                std::cerr << "err: unsupported command type" << std::endl;
                return false;
        }
        return true;
    } catch (const std::exception& e) {
        fmt::print(std::cerr, "error executing command: {}", e.what());
        return false;
    }
}

auto Executor::executeSelect(const SelectCommand& c) -> void {
    if (c.getTableNames().empty()) {
        throw std::runtime_error("no table specified in SELECT");
    }

    const std::string& table_name = c.getTableNames()[0];
    auto table = database_.getTable(table_name);
    if (!table) {
        throw std::runtime_error(fmt::format("table '{}' doesnt exist", table_name));
    }

    auto rows = table->getRows();
    const auto& columns = c.getColumnNames();
    const auto& where_clause = c.getWhereClause();

    // Print column headers
    for (const auto& col : columns) {
        fmt::print("{}\t", col);
    }
    fmt::print("\n");

    // Parse WHERE clause if present
    std::string column_name, operator_str, value_str;
    if (!where_clause.empty()) {
        std::istringstream iss(where_clause);
        iss >> column_name >> operator_str >> value_str;
        if (column_name.empty() || operator_str.empty() || value_str.empty()) {
            throw std::runtime_error("invalid WHERE clause format");
        }
    }

    // Print data rows
    for (const auto& row : rows) {
        // Apply WHERE clause filtering
        if (!where_clause.empty()) {
            if (!row.hasColumn(column_name)) {
                continue; // Skip rows that don't have the column we're filtering on
            }

            const auto& value = row.getValue(column_name);
            Value compare_value;
            
            // Parse the comparison value
            try {
                if (value_str.front() == '\'' || value_str.front() == '"') {
                    compare_value = Value(value_str.substr(1, value_str.length()-2));
                } else if (value_str == "true" || value_str == "false") {
                    compare_value = Value(value_str == "true");
                } else if (value_str.find('.') != std::string::npos) {
                    compare_value = Value(std::stod(value_str));
                } else {
                    compare_value = Value(std::stoi(value_str));
                }
            } catch (const std::exception& e) {
                throw std::runtime_error(fmt::format("error parsing value in WHERE clause: {}", e.what()));
            }

            // Apply the comparison
            bool matches = false;
            if (operator_str == "=") matches = value == compare_value;
            else if (operator_str == "!=") matches = value != compare_value;
            else if (operator_str == "<") matches = value < compare_value;
            else if (operator_str == ">") matches = value > compare_value;
            else if (operator_str == "<=") matches = value <= compare_value;
            else if (operator_str == ">=") matches = value >= compare_value;
            else throw std::runtime_error(fmt::format("unsupported operator in WHERE clause: {}", operator_str));

            if (!matches) continue;
        }

        // Print the row if it passes the WHERE clause
        for (const auto& col : columns) {
            if (row.hasColumn(col)) {
                const auto& value = row.getValue(col);
                if (!value.isNull()) {
                    fmt::print("{}\t", value.toString());
                } else {
                    fmt::print("NULL\t");
                }
            } else {
                fmt::print("NULL\t");
            }
        }
        fmt::print("\n");
    }
}

auto Executor::executeCreate(const CreateCommand& c) -> void {
    const std::string& table_name = c.getTableName();

    if (table_name.empty()) {
        throw std::runtime_error("table name cannot be empty");
    }

    if (database_.tableExists(table_name)) {
        throw std::runtime_error(fmt::format("table '{}' already exists", table_name));
    }

    auto table = std::make_shared<Table>(table_name, c.getColumns());
    
    // Add any table-level constraints
    for (const auto& constraint : c.getConstraints()) {
        table->addConstraint(constraint);
    }
    
    database_.addTable(table);
    fmt::println("table '{}' created successfully", table_name);
}

auto Executor::executeDrop(const DropCommand& c) -> void {
    const std::string& table_name = c.getTableName();

    if (!database_.tableExists(table_name)) {
        throw std::runtime_error(fmt::format("table '{}' doesnt exist", table_name));
    }
    if (database_.dropTable(table_name)) {
        fmt::println("table '{}' dropped successfully", table_name);
    } else {
        throw std::runtime_error(fmt::format("failed to drop table '{}'", table_name));
    }
}

auto Executor::executeInsert(const InsertCommand& c) -> void {
    const std::string& table_name = c.getTableName();
    auto table = database_.getTable(table_name);

    if (!table) {
        throw std::runtime_error(fmt::format("table '{}' doesnt exist", table_name));
    }

    const auto& column_names = c.getColumnNames();
    const auto& values = c.getValues();

    if (values.empty()) {
        throw std::runtime_error("no values provided for INSERT");
    }

    // for each set of values -> insert a row
    for (const auto& value_set : values) {
        if (value_set.size() != column_names.size()) {
            throw std::runtime_error(fmt::format("mismatch between number of columns ({}) and values ({})", 
                column_names.size(), value_set.size()));
        }

        Row row;
        for (int i = 0; i < column_names.size(); i++) {
            row.setValue(column_names[i], value_set[i]);
        }
        if (!database_.validateRow(table_name, row)) {
            throw std::runtime_error(fmt::format("row validation failed for table '{}'", table_name));
        }

        table->addRow(row);
    }

    fmt::println("successfully inserted ({}) row(s) into {}", values.size(), table_name);
}

auto Executor::executeUpdate(const UpdateCommand& c) -> void {
    const std::string& table_name = c.getTableName();
    if (table_name.empty()) {
        throw std::runtime_error("no table specified in UPDATE");
    }

    auto table = database_.getTable(table_name);
    if (!table) {
        throw std::runtime_error(fmt::format("table '{}' doesnt exist", table_name));
    }

    const auto& updates = c.getColumnValues();
    const auto& where_clause = c.getWhereClause();

    //TODO: where shit
    auto rows = table->getRows();
    for (auto& row : rows) {
        for (const auto& [col_name, value] : updates) {
            row.setValue(col_name, value);
        }
    }

    fmt::println("successfully updated ({}) row(s) in {}", rows.size(), table_name);
}

auto Executor::executeDelete(const DeleteCommand& c) -> void {
    const std::string& table_name = c.getTableName();
    if (table_name.empty()) {
        throw std::runtime_error("no table specified in DELETE");
    }

    auto table = database_.getTable(table_name);
    if (!table) {
        throw std::runtime_error(fmt::format("table '{}' doesnt exist", table_name));
    }

    const auto& where_clause = c.getWhereClause();

    //TODO: also where shit
    table->clear();
    fmt::println("successfully deleted rows from '{}'", table_name);
}

auto Executor::executeAlter(const AlterCommand& c) -> void {
    const std::string& table_name = c.getTableName();
    auto table = database_.getTable(table_name);
    
    if (!table) {
        throw std::runtime_error(fmt::format("table '{}' doesn't exist", table_name));
    }

    switch (c.getAlterType()) {
        case AlterCommand::AlterType::ADD: {
            const auto& new_column = c.getNewColumn();
            if (table->hasColumn(new_column.getName())) {
                throw std::runtime_error(fmt::format("column '{}' already exists in table '{}'", 
                    new_column.getName(), table_name));
            }
            table->addColumn(new_column);
            fmt::println("successfully added column '{}' to table '{}'", 
                new_column.getName(), table_name);
            break;
        }
        case AlterCommand::AlterType::DROP: {
            const auto& column_name = c.getColumnName();
            if (!table->hasColumn(column_name)) {
                throw std::runtime_error(fmt::format("column '{}' doesn't exist in table '{}'", 
                    column_name, table_name));
            }
            table->dropColumn(column_name);
            fmt::println("successfully dropped column '{}' from table '{}'", 
                column_name, table_name);
            break;
        }
        case AlterCommand::AlterType::RENAME: {
            const auto& old_name = c.getColumnName();
            const auto& new_name = c.getNewColumnName();
            if (!table->hasColumn(old_name)) {
                throw std::runtime_error(fmt::format("column '{}' doesn't exist in table '{}'", 
                    old_name, table_name));
            }
            if (table->hasColumn(new_name)) {
                throw std::runtime_error(fmt::format("column '{}' already exists in table '{}'", 
                    new_name, table_name));
            }
            table->renameColumn(old_name, new_name);
            fmt::println("successfully renamed column '{}' to '{}' in table '{}'", 
                old_name, new_name, table_name);
            break;
        }
    }
}

auto Executor::executeSave(const SaveCommand& c) -> void {
    const std::string& filename = c.getFilename();
    
    if (filename.empty()) {
        throw std::runtime_error("filename cannot be empty");
    }
    
    // Save all commands that would rebuild the current database state
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error(fmt::format("failed to open file '{}' for saving", filename));
    }
    
    // First write the database creation commands (tables, columns)
    for (const auto& tableName : database_.getTableNames()) {
        auto table = database_.getTable(tableName);
        
        // Generate CREATE TABLE command
        std::string createCmd = fmt::format("CREATE TABLE {} (", tableName);
        const auto& columns = table->getColumns();
        
        for (size_t i = 0; i < columns.size(); ++i) {
            const auto& col = columns[i];
            std::string typeStr;
            
            switch (col.getType()) {
                case DataType::INTEGER:
                    typeStr = "INTEGER";
                    break;
                case DataType::FLOAT:
                    typeStr = "FLOAT";
                    break;
                case DataType::STRING:
                    typeStr = "STRING";
                    break;
                case DataType::BOOLEAN:
                    typeStr = "BOOLEAN";
                    break;
                default:
                    typeStr = "INTEGER"; // Default fallback
            }
            
            createCmd += fmt::format("{} {}", col.getName(), typeStr);
            
            // Add column constraints
            bool isPrimaryKey = false;
            bool isUnique = false;
            bool isNotNull = false;
            Value defaultValue;
            bool hasDefault = false;
            
            for (const auto& constraint : col.getConstraints()) {
                if (constraint->getType() == ConstraintType::PRIMARY_KEY) {
                    isPrimaryKey = true;
                } else if (constraint->getType() == ConstraintType::UNIQUE) {
                    isUnique = true;
                } else if (constraint->getType() == ConstraintType::NOT_NULL) {
                    isNotNull = true;
                } else if (constraint->getType() == ConstraintType::DEFAULT) {
                    auto defConstraint = std::dynamic_pointer_cast<DefaultConstraint>(constraint);
                    if (defConstraint) {
                        defaultValue = defConstraint->getDefaultValue();
                        hasDefault = true;
                    }
                }
            }

            // add constraints in order: PRIMARY KEY, UNIQUE, NOT NULL, DEFAULT
            if (isPrimaryKey) {
                createCmd += " PRIMARY KEY";
            } 
            if (isUnique && !isPrimaryKey) { // PRIMARY KEY implies UNIQUE
                createCmd += " UNIQUE";
            }
            if (isNotNull) {
                createCmd += " NOT NULL";
            }
            if (hasDefault) {
                if (defaultValue.getType() == DataType::STRING) {
                    createCmd += fmt::format(" DEFAULT '{}'", defaultValue.toString());
                } else {
                    createCmd += fmt::format(" DEFAULT {}", defaultValue.toString());
                }
            }
            
            if (i < columns.size() - 1) {
                createCmd += ", ";
            }
        }
        createCmd += ")";
        file << createCmd << std::endl;

        // generate inserts for all rows
        const auto& rows = table->getRows();
        for (const auto& row : rows) {
            const auto& values = row.getValues();
            if (values.empty()) continue;

            std::string insert_cmd = fmt::format("INSERT INTO {} VALUES (", tableName);

            size_t i = 0;
            for (const auto& [colName, val] : values) {
                if (val.getType() == DataType::STRING) {
                    insert_cmd += fmt::format("'{}'", val.toString());
                } else {
                    insert_cmd+= val.toString();
                }

                if (i < values.size() - 1) {
                    insert_cmd+= ", ";
                }
                ++i;
            }
            insert_cmd += ")";
            file << insert_cmd << std::endl;
        }
    }
    
    file.close();
    fmt::println("database state saved as commands to '{}'", filename);
}

auto Executor::executeLoad(const LoadCommand& c) -> void {
    const std::string& filename = c.getFilename();

    if (filename.empty()) {
        throw std::runtime_error("filename cannot be empty");
    }

    // read commands from file and execute them
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error(fmt::format("failed to open file '{}' for loading", filename));
    }
    database_.clear();

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            try {
                Parser parser(database_);
                auto command = parser.parse(line);
                if (command) {
                    // skip SAVE and LOAD commands, avoids recursion
                    if (command->getType() != CommandType::SAVE && 
                        command->getType() != CommandType::LOAD) {
                        execute(command);
                    }
                } else {
                    fmt::print(std::cerr, "warning: failed to parse command: {}\n", line);
                }
            } catch (const std::exception& e) {
                fmt::print(std::cerr, "warning: error executing command '{}': {}\n", line, e.what());
            }
        }
    }

    file.close();
    fmt::println("database state loaded from '{}'", filename);
}

auto Executor::executeShow(const ShowCommand& c) -> void {
    switch (c.getShowType()) {
        case ShowCommand::ShowType::TABLES: {
            auto table_names = database_.getTableNames();
            if (table_names.empty()) {
                fmt::println("no tables in database");
                return;
            }
            fmt::println("Tables in database:");
            for (const auto& name : table_names) {
                fmt::println("- {}", name);
            }
            break;
        }
        case ShowCommand::ShowType::COLUMNS: {
            const std::string& table_name = c.getTableName();
            auto table = database_.getTable(table_name);
            if (!table) {
                throw std::runtime_error(fmt::format("table '{}' doesn't exist", table_name));
            }
            const auto& columns = table->getColumns();
            fmt::println("Columns in table '{}':", table_name);
            for (const auto& column : columns) {
                fmt::println("- {} ({})", column.getName(), dataTypeToString(column.getType()));
            }
            break;
        }
    }
}

auto Executor::executeHelp(const HelpCommand& c) -> void {
    std::map<std::string, std::string> commands = {
        {"SELECT", "SELECT column1, column2, ... FROM table_name [WHERE condition]\n"
                  "  - Retrieves data from a table\n"
                  "  - Use * to select all columns\n"
                  "  - Example: SELECT * FROM employees WHERE salary > 50000"},
                  
        {"CREATE", "CREATE TABLE table_name (column1 TYPE, column2 TYPE, ...)\n"
                  "  - Creates a new table with specified columns\n"
                  "  - Supported types: INTEGER, STRING, DOUBLE, BOOLEAN\n"
                  "  - Example: CREATE TABLE employees (id INTEGER, name STRING, salary DOUBLE)"},
                  
        {"INSERT", "INSERT INTO table_name [(column1, column2, ...)] VALUES (value1, value2, ...), ...\n"
                  "  - Inserts new rows into a table\n"
                  "  - Can insert multiple rows with comma separation\n"
                  "  - Example: INSERT INTO employees VALUES (1, 'John Doe', 75000)"},
                  
        {"UPDATE", "UPDATE table_name SET column1 = value1, column2 = value2, ... [WHERE condition]\n"
                  "  - Updates existing rows in a table\n"
                  "  - Example: UPDATE employees SET salary = 80000 WHERE id = 1"},
                  
        {"DELETE", "DELETE FROM table_name [WHERE condition]\n"
                  "  - Removes rows from a table\n"
                  "  - Example: DELETE FROM employees WHERE id = 1"},
                  
        {"DROP", "DROP TABLE table_name\n"
                "  - Deletes an entire table\n"
                "  - Example: DROP TABLE employees"},
                
        {"ALTER", "ALTER TABLE table_name ADD column_name TYPE\n"
                 "ALTER TABLE table_name DROP COLUMN column_name\n"
                 "ALTER TABLE table_name RENAME COLUMN old_name TO new_name\n"
                 "  - Modifies the structure of a table\n"
                 "  - Example: ALTER TABLE employees ADD department STRING"},
                 
        {"SHOW", "SHOW TABLES\n"
               "SHOW COLUMNS FROM table_name\n"
               "  - Lists tables in the database or columns in a table\n"
               "  - Example: SHOW COLUMNS FROM employees"},
               
        {"SAVE", "SAVE TO 'filename'\n"
               "  - Saves the database to a file\n"
               "  - Example: SAVE TO 'my_database.db'"},
               
        {"LOAD", "LOAD FROM 'filename'\n"
               "  - Loads a database from a file\n"
               "  - Example: LOAD FROM 'my_database.db'"},
               
        {"HELP", "HELP [command_name]\n"
               "  - Displays information about commands\n"
               "  - Example: HELP CREATE"},
               
        {"EXIT", "exit\n"
               "  - Exits the SQL interface"}
    };

    if (c.hasSpecificCommand()) {
        const std::string& command_name = c.getCommandName();
        auto it = commands.find(command_name);
        if (it != commands.end()) {
            fmt::println("Help for {} command:", command_name);
            fmt::println("{}", it->second);
        } else {
            fmt::println("Unknown command: {}", command_name);
            fmt::println("Type HELP to see all available commands.");
        }
    } else {
        fmt::println("Available commands:");
        fmt::println("-------------------");
        for (const auto& [cmd, _] : commands) {
            fmt::println("{}", cmd);
        }
        fmt::println("\nType HELP command_name for detailed information on a specific command.");
    }
}




