#include <iostream>
#include <fmt/base.h>
#include <string>
#include <signal.h>

#include "Row.h"
#include "Value.h"
#include "Parser.hpp"
#include "Executor.hpp"
#include "Database.h"
#include "Serializer.hpp"

Database* globalDb = nullptr;

void signalHandler(int signum) {
    if (globalDb) {
        fmt::println("\nSaving database state before exit...");
        Serializer::serializeDatabase(*globalDb, "database_backup.db");
        fmt::println("Database state saved successfully.");
    }
    exit(signum);
}

void executeQuery(Database& db, const std::string& query) {
    Parser parser(db);
    auto command = parser.parse(query);
    // command is returned good except that column_names_ are not present
    if (command) {
        fmt::println("successfully parsed query: {}", query);
        Executor executor(db);
        executor.execute(command); // fucks up here i think
    } else {
        fmt::println("failed to parse query: {}", query);
    }
    fmt::println("---");
}

int main() {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    Database db("test_db");
    globalDb = &db;

    if (Serializer::deserializeDatabase(db, "database_backup.db")) {
        fmt::println("Successfully loaded database state from backup.");
    } else {
        fmt::println("No existing database state found or error loading backup. Starting with fresh database.");
    }

    fmt::println("Welcome to SQL REPL. Type your SQL commands or 'exit' to quit.");
    fmt::println("Example commands:");
    fmt::println("  CREATE TABLE employees (id INT, name VARCHAR(100), salary DOUBLE)");
    fmt::println("  INSERT INTO employees VALUES (1, 'John Doe', 75000)");
    fmt::println("  SELECT * FROM employees");
    fmt::println("  exit");

    std::string input;
    while (true) {
        fmt::print("sql> ");
        std::getline(std::cin, input);

        if (input == "exit") {
            fmt::println("Saving database state before exit...");
            Serializer::serializeDatabase(db, "database_backup.db");
            fmt::println("Database state saved successfully.");
            break;
        }

        if (!input.empty()) {
            executeQuery(db, input);
        }
    }

    return 0;
}
