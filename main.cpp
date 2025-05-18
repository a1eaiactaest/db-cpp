#include <iostream>
#include <fmt/base.h>
#include <string>
#include <signal.h>
#include <fstream>

#include "Row.h"
#include "Value.h"
#include "Parser.hpp"
#include "Executor.hpp"
#include "Database.h"

const std::string COMMAND_LOG_FILE = "db_commands.log";
Database* globalDb = nullptr;

void logCommand(const std::string& command) {
    std::ofstream logFile(COMMAND_LOG_FILE, std::ios::app);
    if (logFile.is_open()) {
        logFile << command << std::endl;
        logFile.close();
    } else {
        fmt::println("Error: Could not open log file for writing");
    }
}

void signalHandler(int signum) {
    if (globalDb) {
        fmt::println("\nExiting database...");
    }
    exit(signum);
}

void executeQuery(Database& db, const std::string& query, bool logToFile = true) {
    Parser parser(db);
    auto command = parser.parse(query);

    if (command) {
        Executor executor(db);
        bool success = executor.execute(command);

        // only log commands that execute successfully and aren't read-only operations
        if (success && logToFile && command->getType() != CommandType::SELECT && 
            command->getType() != CommandType::SHOW && command->getType() != CommandType::HELP) {
            logCommand(query);
        }
    } else {
        fmt::println("failed to parse query: {}", query);
    }
    fmt::println("---");
}

bool rebuildDatabaseFromLog(Database& db) {
    std::ifstream logFile(COMMAND_LOG_FILE);
    if (!logFile.is_open()) {
        return false;
    }

    std::string command;
    while (std::getline(logFile, command)) {
        if (!command.empty()) {
            executeQuery(db, command, false); // don't log these commands again
        }
    }

    logFile.close();
    return true;
}

/*
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
*/

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    Database db("test_db");
    globalDb = &db;

    if (rebuildDatabaseFromLog(db)) {
        fmt::println("Successfully rebuilt database from command log.");
    } else {
        fmt::println("No existing command log found or error reading log. Starting with fresh database.");
    }

    fmt::println("\nWelcome to SQL REPL. Type your SQL commands or 'exit' to quit.");
    fmt::println("Example commands:");
    fmt::println("\tCREATE TABLE employees (id INTEGER, name STRING, salary DOUBLE)");
    fmt::println("\tINSERT INTO employees VALUES (1, 'John Doe', 75000)");
    fmt::println("\tSELECT * FROM employees");
    fmt::println("\tHELP - Show available commands");
    fmt::println("\tHELP SELECT - Show help for a specific command");
    fmt::println("\texit");

    std::string input;
    while (true) {
        fmt::print("sql> ");
        std::getline(std::cin, input);

        if (input == "exit") {
            break;
        }

        if (!input.empty()) {
            executeQuery(db, input);
        }
    }

    return 0;
}
