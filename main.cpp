#include <iostream>
#include <fmt/base.h>

#include "Row.h"
#include "Value.h"
#include "Parser.hpp"
#include "Executor.hpp"
#include "Database.h"

void executeQuery(Database& db, const std::string& query) {
    Parser parser;
    auto command = parser.parse(query);
    if (command) {
        fmt::println("successfully parsed query: {}", query);
        Executor executor(db);
        executor.execute(command);
    } else {
        fmt::println("failed to parse query: {}", query);
    }
    fmt::println("---");
}

int main() {
    Database db("test_db");

    executeQuery(db, "CREATE TABLE employees (id INT, name VARCHAR(100), salary DOUBLE)");

    executeQuery(db, "INSERT INTO employees (id, name, salary) VALUES (1, 'John Doe', 75000)");
    executeQuery(db, "INSERT INTO employees (id, name, salary) VALUES (2, 'Jane Smith', 85000)");

    executeQuery(db, "SELECT id, name, salary FROM employees");

    executeQuery(db, "UPDATE employees SET salary = 80000 WHERE id = 1");

    executeQuery(db, "SELECT id, name, salary FROM employees");

    executeQuery(db, "DELETE FROM employees WHERE id = 2");

    executeQuery(db, "SELECT id, name, salary FROM employees");

    return 0;
}
