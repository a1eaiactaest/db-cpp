#ifndef DATABASE_H
#define DATABASE_H

#include <string>

#include "CommonTypes.h"

class Database {
private: 
    std::string name_;
    TablePtrMap tables_;
public:
    Database(std::string name) : name_(name) {
        if (name_.empty()) {
            throw std::runtime_error("database name cannot be empty");
        }
    };
    Database(std::string& name) : name_(std::move(name)) {
        if (name_.empty()) {
            throw std::runtime_error("database name cannot be empty");
        }
    };
    ~Database() = default;

    void addTable(TablePtr table);
    bool dropTable(const std::string& name);
    TablePtr getTable(const std::string& name) const;
    bool tableExists(const std::string& name) const;

    const std::string& getName() const;
    void setName(std::string new_name);
    std::vector<std::string> getTableNames() const;

    // adds db context
    bool validateRow(const std::string& table_name, const Row& row);

    void clear();
};

#endif //DATABASE_H
