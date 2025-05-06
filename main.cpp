#include <iostream>
#include <fmt/base.h>

#include "Row.h"
#include "Value.h"

int main() {
    std::cout << "Hello, World!" << std::endl;
    Row employeeRow;
    employeeRow.setValue("id", Value(101));
    employeeRow.setValue("name", Value("Dupens Parabolis"));
    employeeRow.setValue("salary", Value(7500000.01));
    employeeRow.setValue("kontrakt nirobistosci", Value(true));

    auto values = employeeRow.getValues();
    for (auto [k,v] : values) {
        fmt::println("{}: {}", k, v.toString());
    }
    return 0;
}
