//
// Created by Piotrek Rybiec on 29/04/2025.
//
#ifndef VALUE_H
#define VALUE_H

#include "data_types.h"

class Value {
private:
    VariantType value;

public:
    Value() : value(std::monostate{}){} // use null as default
    explicit Value(int v) : value(v){}
    explicit Value(double v) : value(v){}
    explicit Value(bool v) : value(v){}
    explicit Value(const std::string& v) : value(v){}
    explicit Value(const char* v) : value(std::string(v)){} // for inline Value("adasd") definition
    explicit Value(const Date& v) : value(v){}
    explicit Value(const DateTime& v) : value(v){}

    /*
    Value v1;
    Value v2 = Value::Null();
    v1.isNull() == v2.isNull();
    */
    static Value Null();
    bool isNull() const;
    DataType getType() const;
    std::string toString() const;

    // templates need to be in headers
    template<typename T>
    T get() const {
        try {
            return std::get<T>(value);
        } catch (const std::bad_variant_access&) {
            throw std::runtime_error("invalid type access in Value");
        }
    }
};

#endif //VALUE_H
