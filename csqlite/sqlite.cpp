/*------- include files:
-------------------------------------------------------------------*/
#include "sqlite.h"
#include <sstream>
#include <fmt/core.h>
#include "stmt.h"

/// close database
bool sqlite_t::
close() noexcept {
    if (db_) {
        if (sqlite3_close_v2(db_) != SQLITE_OK) {
            LOG_ERROR(db_);
            return false;
        }
        db_ = nullptr;
    }
    return true;
}

/// open existed database
bool sqlite_t::
open(fs::path const &path, bool const read_only) noexcept {
    if (db_ != nullptr) {
        std::cerr << "database is already opened\n";
        return false;
    }
    if (path == IN_MEMORY) {
        std::cerr << "database in memory can't be opened (use create)\n";
        return false;
    }

    auto const flags = read_only ? SQLITE_READONLY : SQLITE_OPEN_READWRITE;
    auto const database_path = path.string();
    if (SQLITE_OK == sqlite3_open_v2(database_path.c_str(), &db_, flags, nullptr)) {
        std::cout << fmt::format("database opened: {}\n", path.string());
        return true;
    }
    LOG_ERROR(db_);
    db_ = nullptr;
    return false;
}

/// create new database.
bool sqlite_t::
create(fs::path const &path, std::function<bool(sqlite_t *)> const &lambda, bool override) noexcept {
    if (db_ != nullptr) {
        std::cerr << "database is already opened\n";
        return false;
    }
    if (lambda == nullptr) {
        std::cerr << "operations to be performed on created database were not specified\n";
        return false;
    }

    std::error_code err{};
    if (path != IN_MEMORY) {
        if (fs::exists(path, err)) {
            if (override) {
                if (!fs::remove(path)) {
                    std::cerr << "database file could not be deleted\n";
                    return false;
                }
            }
        } else if (err)
            std::cerr << "database already exist " << err << "\n";
    }

    auto const flags = SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE;
    auto const database_path = path.string();
    if (SQLITE_OK == sqlite3_open_v2(database_path.c_str(), &db_, flags, nullptr)) {
        if (!lambda(this)) {
            std::cerr << "error inside lambda\n";
            return false;
        }
        std::cout << "database created successfully: " << path << '\n';
        return true;
    }
    LOG_ERROR(db_);
    return false;
}

query_t sqlite_t::
query4update(std::string const& table_name, row_t const& fields, std::optional<field_t> const& where) noexcept {
    auto [names, values] = fields.split();

    std::stringstream placeholders;
    auto it = names.cbegin();
    auto end = prev(names.end());
    for (; it < end; it++) {
        placeholders << *it << "=?, ";
    }
    placeholders << *it << "=?";

    std::stringstream ss{};
    ss << "UPDATE " << table_name << " SET " << placeholders.str();
    std::string query = ss.str();

    if (where) {
        auto f = *where;
        auto const [name, value] = f();
        std::stringstream ss{};
        ss << " WHERE " << name << "=?";
        query += ss.str();
        values.push_back(value);
    }
    query += ";";

    return query_t{std::move(query), std::move(values)};
}

query_t sqlite_t::
query4insert(std::string const& table_name, row_t const& fields) noexcept {
    auto [names, values] = fields.split();
    std::vector<std::string> const placeholders(names.size(), "?");
    std::stringstream ss{};
    ss << "INSERT INTO " << table_name
        << '(' << share::join_strings(names) << ')'
        << " VALUES "
        << '(' << share::join_strings(placeholders) << ')'
        << ';';

    return query_t{ss.str(), std::move(values)};
}
/*
std::string bytes_as_string(std::vector<u8> const& data) noexcept {
    if (data.empty()) return std::string{};

    stringstream ss;
    size_t const n = data.size() - 1;
    size_t i = 0;

    for (; i < n; i++)
        ss << "0x" << hex << setfill('0') << setw(2) << int(data[i]) << ", ";
    ss << "0x" << hex << setfill('0') << setw(2) << int(data[i]);
    return ss.str();
}
*/