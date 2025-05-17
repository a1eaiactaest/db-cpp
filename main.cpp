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

// Filename for the command log
const std::string COMMAND_LOG_FILE = "db_commands.log";
Database* globalDb = nullptr;

// Function to save a command to the log file
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
        fmt::println("successfully parsed query: {}", query);
        Executor executor(db);
        executor.execute(command);

        if (logToFile && command->getType() != CommandType::SELECT && 
            command->getType() != CommandType::SHOW) {
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
