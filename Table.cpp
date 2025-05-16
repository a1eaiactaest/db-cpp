//
// Created by Piotrek Rybiec on 05/05/2025.
//
//

#include <string>
#include <vector>
#include <fmt/format.h>


#include "Constraint.h"
#include "Row.h"
#include "Table.h"
#include "Column.h"

Table::Table(std::string name) : name_(std::move(name)) {}
Table::Table(const std::string& name, const std::vector<Column>& columns) : name_(name), columns_(columns) {
    for (int i = 0; i < columns_.size(); i++) {
        column_index_map_[columns_[i].getName()] = i;
    }
}

auto Table::addColumn(const Column& column) -> void {
    if (hasColumn(column.getName())) {
        throw std::runtime_error(
            fmt::format("column already exists: {}", column.getName())
        );
    }
    columns_.push_back(std::move(column));
    column_index_map_[columns_.back().getName()] = columns_.size() - 1;
}

auto Table::getColumn(const std::string& name) const -> const Column& {
    if (!hasColumn(name)) {
        throw std::runtime_error(
            fmt::format("column not found: {}", name)
        );
    }
    return columns_[column_index_map_.at(name)];
}

auto Table::getColumns() const -> const ColumnList& { return columns_; }

auto Table::hasColumn(const std::string& name) const -> bool {
    return column_index_map_.find(name) != column_index_map_.end();
}

auto Table::getColumnIndex(const std::string& name) const -> size_t {
    if (!hasColumn(name)) {
        throw std::runtime_error(
            fmt::format("column not found: {}", name)
        );
    }
    return column_index_map_.at(name);
}

auto Table::addRow(const Row& row) -> void {
    if (!validateRow(row)) {
        throw std::runtime_error("row validation failed");
    }
    rows_.push_back(std::move(row));
}

auto Table::getRows() const -> const RowList& { return rows_; }

auto Table::getRow(size_t index) -> Row& {
    if (index >= rows_.size()) {
        throw std::out_of_range(
            std::format("row index out of range, exists: {} accessing: {}", rows_.size(), index)
        );
    }
    return rows_[index];
}

auto Table::rowCount() const -> size_t { return rows_.size(); }


auto Table::addConstraint(ConstraintPtr c) -> void {
    constraints_.push_back(std::move(c));
}

auto Table::getConstraints() const -> const ConstraintList& { return constraints_; }

auto Table::getConstraintsOfType(ConstraintType t) const -> ConstraintList {
    ConstraintList res;
    for (const auto& constraint : constraints_) {
        if (constraint->getType() == t) {
            res.push_back(constraint);
        }
    }
    return res;
}

auto Table::getName() const -> const std::string& { return name_; }

auto Table::setName(std::string new_name) -> void {
    name_ = std::move(new_name);
}

auto Table::validateRow(const Row& row) const -> bool {
    // all columns in the row are present
    for (const auto& col : columns_) {
        if (!col.isNullable() && !row.hasColumn(col.getName())) {
            return false;
        }
    }

    for (const auto& constraint : constraints_) {
        //throw std::logic_error("function Table::validateRow not implemented");
        if (!constraint->validate(row, *this)) {
            return false;
        }
    }
    return true;
}

auto Table::clear() -> void {
    columns_.clear();
    rows_.clear();
    constraints_.clear();
    column_index_map_.clear();
}

auto Table::clearRows() -> void { rows_.clear(); }

auto Table::getPrimaryKeyConstraint() const -> ConstraintPtr {
    for (const auto& constraint : constraints_) {
        if (constraint->getType() == ConstraintType::PRIMARY_KEY) {
            return constraint;
        }
    }
    return nullptr;
}

auto Table::getPrimaryKeyColumns() const -> std::vector<std::string> {
    auto pk_columns = std::vector<std::string>();
    if (auto pk = getPrimaryKeyConstraint()) {
        if (auto primary_key = dynamic_cast<PrimaryKeyConstraint*>(pk.get())) {
            return primary_key->getColumnNames();
        }
    }
    return pk_columns;;
}

auto Table::dropColumn(const std::string& name) -> void {
    auto it = std::find_if(columns_.begin(), columns_.end(), 
        [&name](const Column& col) { return col.getName() == name; });
    if (it != columns_.end()) {
        columns_.erase(it);
        column_index_map_.erase(name);
        // update indices for remaining columns, it may be a little overhead?
        for (size_t i = 0; i < columns_.size(); i++) {
            column_index_map_[columns_[i].getName()] = i;
        }
    }
}

auto Table::renameColumn(const std::string& old_name, const std::string& new_name) -> void {
    auto it = std::find_if(columns_.begin(), columns_.end(), 
        [&old_name](const Column& col) { return col.getName() == old_name; });
    if (it != columns_.end()) {
        it->setName(new_name);
        column_index_map_.erase(old_name);
        column_index_map_[new_name] = std::distance(columns_.begin(), it);
    }
}
