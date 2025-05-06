//
// Created by Piotrek Rybiec on 05/05/2025.
//

#ifndef TABLE_H
#define TABLE_H

#include <memory>
#include <string>
#include <vector>

#include "Row.h"
#include "Column.h"
#include "Constraint.h"

class Table {
private:
    std::string table_name;
    std::vector<Column> columns;
    std::vector<Row> rows;
    std::vector<std::shared_ptr<Constraint>> table_constraints;

public:

};

#endif //TABLE_H
