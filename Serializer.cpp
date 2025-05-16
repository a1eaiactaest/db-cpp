#include <sstream>
#include <stdexcept>
#include <fmt/core.h>
#include <fmt/base.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

#include "Serializer.hpp"
#include "Database.h"
#include "Column.h"
#include "Constraint.h"
#include "data_types.h"

auto Serializer::serializeDatabase(const Database& db, const std::string& fn) -> bool {
    std::ofstream file(fn);
    if (!file.is_open()) {
        return false;
    }

    writeLine(file, "DATABASE " + db.getName());

    for (const auto& table_name : db.getTableNames()) {
        auto table = db.getTable(table_name);
        writeLine(file, serializeTable(*table));
    }
    return true;
}


auto Serializer::deserializeDatabase(Database &db, const std::string &fn) -> bool {
    std::ifstream file(fn);
    if (!file.is_open()) {
        return false;
    }

    std::string line = readLine(file);
    if (line.substr(0, 8) != "DATABASE"){
        return false;
    }
    db.setName(line.substr(9));

    while (file.good()) {
        if (!deserializeTable(db, file)) {
            break;
        }
    }
    return true;
}

auto Serializer::serializeTable(const Table& table) -> std::string {
    std::stringstream ss;
    ss << fmt::format("TABLE {}\n", table.getName());

    for (const auto& col : table.getColumns()) {
        ss << serializeColumn(col);
    }

    for (const auto& cons : table.getConstraints()) {
        ss << serializeConstraint(*cons);
    }

    for (const auto& row : table.getRows()) {
        ss << serializeRow(row);
    }

    ss << "END_TABLE\n";
    return ss.str();
}


auto Serializer::serializeColumn(const Column& col) -> std::string {
    std::stringstream ss;
    ss << fmt::format("COLUMN {} {} {}\n", col.getName(), static_cast<int>(col.getType()), (col.isNullable() ? "1":"0"));
    return ss.str();
}

auto Serializer::serializeRow(const Row& row) -> std::string {
    std::string result = "ROW ";
    for (const auto& [col_name, val] : row.getValues()) {
        result += fmt::format("{}:{} ", col_name, serializeValue(val));
    }
    result += "\n";
    return result;
}

auto Serializer::serializeValue(const Value& val) -> std::string {
    std::stringstream ss;
    switch (val.getType()) {
        case DataType::INTEGER:
            ss << "INTEGER:" << val.toString();
            break;
        case DataType::FLOAT:
            ss << "FLOAT:" << val.toString();
            break;
        case DataType::STRING:
            ss << "STRING:" << val.toString();
            break;
        case DataType::BOOLEAN:
            ss << "BOOLEAN:" << val.toString();
            break;
        case DataType::NULL_VALUE:
            ss << "NULL_VALUE";
            break;
        default:
            throw std::runtime_error("Unknown data type in serialization");
    }
    return ss.str();
}

auto Serializer::serializeConstraint(const Constraint& constraint) -> std::string {
    fmt::memory_buffer buf;
    fmt::format_to(std::back_inserter(buf), "CONSTRAINT {} {} ", static_cast<int>(constraint.getType()), constraint.getName());

    switch (constraint.getType()) {
        case ConstraintType::PRIMARY_KEY: {
            const auto& pk = static_cast<const PrimaryKeyConstraint&>(constraint);
            for (const auto& col : pk.getColumnNames()) {
                fmt::format_to(std::back_inserter(buf), "{} ", col);
            }
            break;
        }
        case ConstraintType::FOREIGN_KEY: {
            const auto& fk = static_cast<const ForeignKeyConstraint&>(constraint);
            fmt::format_to(std::back_inserter(buf), "{} {} {} ", fk.getColumnName(), fk.getRefTable(), fk.getRefColumn());
            break;
        }
        case ConstraintType::UNIQUE: {
            const auto& unique = static_cast<const UniqueConstraint&>(constraint);
            for (const auto& col : unique.getColumnNames()) {
                fmt::format_to(std::back_inserter(buf), "{} ", col);
            }
            break;
        }
        case ConstraintType::NOT_NULL: {
            const auto& not_null = static_cast<const NotNullConstraint&>(constraint);
            fmt::format_to(std::back_inserter(buf), "{} ", not_null.getColumnName());
            break;
        }
        case ConstraintType::DEFAULT: {
            const auto& def = static_cast<const DefaultConstraint&>(constraint);
            fmt::format_to(std::back_inserter(buf), "{} {} ", def.getColumnName(), serializeValue(def.getDefaultValue()));
            break;
        }
    }

    fmt::format_to(std::back_inserter(buf), "\n");
    return fmt::to_string(buf);
}

auto Serializer::deserializeTable(Database& db, std::ifstream& file) -> bool {
    std::string line = readLine(file);
    if (line.empty() || line.substr(0, 5) != "TABLE") {
        return false;
    }

    std::string table_name = line.substr(6);
    auto table = std::make_shared<Table>(table_name);

    while (file.good()) {
        line = readLine(file);
        if (line == "END_TABLE") {
            break;
        }
        if (line.substr(0, 6) == "COLUMN") {
            deserializeColumn(*table, file);
        } else if (line.substr(0, 10) == "CONSTRAINT") {
            deserializeConstraint(*table, file);
        } else if (line.substr(0, 3) == "ROW") {
            deserializeRow(*table, file);
        }
    }
    db.addTable(table);
    return true;
}

auto Serializer::deserializeColumn(Table& table, std::ifstream& file) -> bool{
    std::string line = readLine(file);
    std::istringstream iss(line);
    std::string name;
    int type;
    bool nullable;

    iss >> name >> type >> nullable;

    table.addColumn(Column(name, static_cast<DataType>(type), nullable));
    return true;
}

auto Serializer::deserializeRow(Table& table, std::ifstream& file) -> bool {
    std::string line = readLine(file);
    std::istringstream iss(line);
    std::string token;
    Row row;
    while (iss >> token) {
        size_t pos = token.find(':');
        std::string column_name = token.substr(0, pos);
        std::string value_str = token.substr(pos + 1);
        Value value;
        if (value_str == "NULL") {
            value = Value();
        } else {
            size_t type_pos = value_str.find(':');
            if (type_pos == std::string::npos) continue;
            std::string type = value_str.substr(0, type_pos);
            std::string val = value_str.substr(type_pos + 1);
            if (type == "INT") {
                value = Value(std::stoi(val));
            } else if (type == "FLOAT") {
                value = Value(std::stof(val));
            } else if (type == "STRING") {
                value = Value(val);
            } else if (type == "BOOL") {
                value = Value(val == "1");
            }
        }
        row.setValue(column_name, value);
    }
    table.addRow(row);
    return true;
}

auto Serializer::deserializeConstraint(Table& table, std::ifstream& file) -> bool {
    std::string line = readLine(file);
    std::istringstream iss(line);
    int type;
    std::vector<std::string> columns;
    std::string col;

    iss >> type;
    while (iss >> col) {
        columns.push_back(col);
    }

    auto constraint = std::make_shared<Constraint>(static_cast<ConstraintType>(type), columns);
    table.addConstraint(constraint);
    return true;
}

auto Serializer::readLine(std::ifstream& file) -> std::string {
    std::string line;
    std::getline(file, line);
    return line;
}

auto Serializer::writeLine(std::ofstream& file, const std::string& line) -> void {
    file << line << "\n";
}
