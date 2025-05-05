//
// Created by Piotrek Rybiec on 04/05/2025.
//

#ifndef COLUMN_H
#define COLUMN_H
#include "data_types.h"

struct Column {
    std::string name;
    DataType type;
    bool isNullable;
};

#endif //COLUMN_H
