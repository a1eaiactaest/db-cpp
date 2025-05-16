#pragma once

#include <string>
#include <fstream>

#include "Table.h"

class Serializer {
public: 
    static bool serializeDatabase(const Database& db, const std::string& filename);
    static bool deserializeDatabase(const Database& db, const std::string& filename);

private:
    static std::string serializeTable(const Table& table);
    static std::string serializeColumn(const Column& table);
    static std::string serializeRow(const Row& table);
    static std::string serializeValue(const Value& table);
    static std::string serializeConstraint(const Constraint& table);

    // (output structure, input file)
    static bool deserializeTable(Database& db, std::ifstream& file);
    static bool deserializeColumn(Table& table, std::ifstream& file);
    static bool deserializeRow(Table& table, std::ifstream& file);
    static bool deserializeValue(Row& row, std::ifstream& file);
    static bool deserializeConstraint(Table& table, std::ifstream& file);

    static std::string readLine(std::ifstream& file);
    static void writeLine(std::ofstream& file, const std::string& line);
};
