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
    Value(const Value& other) = default; // copy constructor
    Value& operator=(const Value& other) = default; // assignment operator

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

    // operators
    bool operator==(const Value& o) const {
        if (isNull() && o.isNull()) return true;
        if (isNull() || o.isNull()) return false;
        return value == o.value;
    }

    bool operator!=(const Value& o) const {
        return !(*this == o);
    }

    bool operator<(const Value& o) const {
        if (isNull() || o.isNull()) return false;
        return std::visit([](const auto& a, const auto& b) -> bool {
            using A = std::decay_t<decltype(a)>;
            using B = std::decay_t<decltype(b)>;
            if constexpr (std::is_same_v<A, B>) {
                return a < b;
            }
            throw std::runtime_error("cannot compare values of different types");
        }, value, o.value);
    }

    bool operator>(const Value& o) const {
        return o < *this;
    }

    bool operator<=(const Value& o) const {
        return !(o < *this);
    }

    bool operator>=(const Value& o) const {
        return !(*this < o);
    }
};

#endif //VALUE_H
