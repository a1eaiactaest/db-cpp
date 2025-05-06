//
// Created by Piotrek Rybiec on 05/05/2025.
//

#ifndef ROW_H
#define ROW_H

#include <unordered_map>
#include <string>

#include "Value.h"

using ValueMap = std::unordered_map<std::string, Value>; 

class Row {
private:
    ValueMap values;

public:
    const Value& getValue(const std::string& column_name) const;
    void setValue(const std::string& column_name, Value val);
    bool hasColumn(const std::string& column_name) const;
    const ValueMap& getValues() const;
    bool empty() const;
    bool removeColumn(const std::string& columnName);
};

#endif //ROW_H
