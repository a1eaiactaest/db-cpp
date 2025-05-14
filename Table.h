//
// Created by Piotrek Rybiec on 05/05/2025.
//

#ifndef TABLE_H
#define TABLE_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Row.h"
#include "Column.h"
#include "Constraint.h"
#include "CommonTypes.h"

class Table {
private:
    std::string name_;
    ColumnList columns_;
    RowList rows_;
    ConstraintList constraints_;
    std::unordered_map<std::string, size_t> column_index_map_;

public:
    explicit Table(std::string& name);
    explicit Table(std::string name);
    Table(const std::string& name, const std::vector<Column>& columns);

    void addColumn(const Column& c);
    const Column& getColumn(const std::string& name) const;
    const ColumnList& getColumns() const;
    bool hasColumn(const std::string& name) const;
    size_t getColumnIndex(const std::string& name) const;

    void addRow(const Row& r);
    const RowList& getRows() const;
    Row& getRow(size_t index);
    size_t rowCount() const;

    void addConstraint(ConstraintPtr c);
    const ConstraintList& getConstraints() const;
    ConstraintList getConstraintsOfType(ConstraintType t) const;

    const std::string& getName() const;
    void setName(std::string new_name);

    bool validateRow(const Row& row) const;

    void clear();
    void clearRows();

    ConstraintPtr getPrimaryKeyConstraint() const;
    std::vector<std::string> getPrimaryKeyColumns() const;
};

#endif //TABLE_H
