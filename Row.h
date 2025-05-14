//
// Created by Piotrek Rybiec on 05/05/2025.
//

#ifndef ROW_H
#define ROW_H

#include <unordered_map>
#include <string>

#include "Value.h"
#include "CommonTypes.h"

class Row {
private:
    ValueMap values;

public:
    const Value& getValue(const std::string& column_name) const;
    const Value& getValue(const Column& col) const;
    void setValue(const std::string& column_name, const Value& val);
    bool hasColumn(const std::string& column_name) const;
    bool hasColumn(const Column& col) const;
    const ValueMap& getValues() const;
    bool empty() const;
    bool removeColumn(const std::string& column_name);
};

#endif //ROW_H
