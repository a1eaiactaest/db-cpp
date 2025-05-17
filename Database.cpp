#include "Constraint.h"
#include "fmt/format.h"

#include "Database.h"
#include "CommonTypes.h"
#include "Table.h"
#include <memory>

auto Database::addTable(TablePtr table) -> void {
    auto table_name = table->getName();
    std::transform(table_name.begin(), table_name.end(), table_name.begin(), ::tolower);
    if (tableExists(table_name)) {
        throw std::runtime_error(
            fmt::format("table already exists: {}", table->getName())
        );
    }
    tables_[table_name] = std::move(table);
}

auto Database::dropTable(const std::string& name) -> bool {
    auto table_name = name;
    std::transform(table_name.begin(), table_name.end(), table_name.begin(), ::tolower);
    if (!tableExists(table_name)) {
        return false;
    }
    tables_.erase(table_name);
    return true;
}

auto Database::getTable(const std::string& name) const -> TablePtr {
    auto table_name = name;
    std::transform(table_name.begin(), table_name.end(), table_name.begin(), ::tolower);
    auto it = tables_.find(table_name);
    if (it != tables_.end()) return it->second;
    return nullptr;
}

auto Database::tableExists(const std::string& name) const -> bool {
    auto table_name = name;
    std::transform(table_name.begin(), table_name.end(), table_name.begin(), ::tolower);
    return tables_.find(table_name) != tables_.end();
}

auto Database::getName() const -> const std::string& {
    return name_;
}

auto Database::setName(std::string new_name) -> void {
    name_ = std::move(new_name);
}

auto Database::getTableNames() const -> std::vector<std::string> {
    auto names = std::vector<std::string>();
    names.reserve(tables_.size());
    for (const auto& [name, _] : tables_) {
        names.push_back(name);
    }
    return names;
}

auto Database::validateRow(const std::string& table_name, const Row& row) -> bool {
    auto table = getTable(table_name);

    if (!table) return false;
    if (!table->validateRow(row)) return false; // not sure if this is neccessary but lets leave it like that for now.
    
    // check db-level constraints
    for (const auto& constraint : table->getConstraints()) {
        if (constraint->getType() == ConstraintType::FOREIGN_KEY) {
            auto fk = std::dynamic_pointer_cast<ForeignKeyConstraint>(constraint);
            if (fk && !fk->validate(row, *table, *this)) {
                return false;
            }
        }
    }
    return true;
}

auto Database::clear() -> void {
    tables_.clear();
}
