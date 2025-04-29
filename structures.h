//
// Created by Piotrek Rybiec on 28/04/2025.
//

#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <vector>
#include <string>
#include <unordered_map>

struct Column {
    std::string name;
    std::string type; // make it an enum
    bool is_nullable;
};

struct Row {
    // (k,v) -> (column_name, value)
    std::unordered_map<std::string, std::string> values;
};

class Table {
private:
    std::string table_name;
    std::vector<Column> columns;
    std::vector<Row> rows;
public:
    explicit Table(const std::string& name);
    std::string getName() const;
    const std::vector<Column>& getColumns() const;
    const std::vector<Row>& getRows() const;
    void addColumn(const Column& column);
    void addRow(const Row& row);
};

class Database {
private:
    std::string db_name;
    std::unordered_map<std::string, Table> tables; // (k,v) -> (table_name, table_obeject)
public:
    explicit Database(const std::string& name);
    std::string getName() const;
    Table& createTable(const std::string& name);
    Table& getTable(const std::string& name);
    bool tableExists(const std::string& name) const;
};

#endif //STRUCTURES_H
