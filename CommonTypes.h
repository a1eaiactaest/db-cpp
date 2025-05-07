#pragma once

#include <vector>
#include <unordered_map>
#include <memory>

// forwards
class Constraint;
class Column;
class Row;
class Table;
class Database;
class Value;

using ConstraintPtr = std::shared_ptr<Constraint>;
using ColumnPtr = std::shared_ptr<Column>;
using TablePtr = std::shared_ptr<Table>;

using ConstraintList = std::vector<ConstraintPtr>;
using TableList = std::vector<TablePtr>;
// no pointers because typically used by one container, lifetime tied to a table's lifetime
using ColumnList = std::vector<Column>; 
using RowList = std::vector<Row>;

using ColumnMap = std::unordered_map<std::string, Column>;
using TableMap = std::unordered_map<std::string, Table>;
using DatabaseMap = std::unordered_map<std::string, Database>;
using ValueMap = std::unordered_map<std::string, Value>;
