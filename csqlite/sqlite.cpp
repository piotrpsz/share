#include "sqlite.h"
#include <format>
#include <sstream>
#include "stmt.h"
#include "helper.h"

using namespace std;

// close database
bool sqlite_t::close() noexcept {
    if (db_) {
        if (sqlite3_close_v2(db_) != SQLITE_OK) {
            LOG_ERROR();
            return false;
        }
        db_ = nullptr;
    }
    return true;
}

bool sqlite_t::open(fs::path const &path, bool const read_only) noexcept {
    if (db_ != nullptr) {
        cerr << "database is already opened\n";
        return false;
    }
    if (path == IN_MEMORY) {
        cerr << "database in memory can't be opened (use create)\n";
        return false;
    }

    auto const flags = read_only ? SQLITE_READONLY : SQLITE_OPEN_READWRITE;
    auto const database_path = path.string();
    if (SQLITE_OK == sqlite3_open_v2(database_path.c_str(), &db_, flags, nullptr)) {
        cout << "database opened: " << path << '\n';
        return true;
    }
    db_ = nullptr;
    LOG_ERROR();
    return false;
}

bool sqlite_t::create(fs::path const &path, function<bool(sqlite_t *)> const &lambda, bool override) noexcept {
    if (db_ != nullptr) {
        cerr << "database is already opened\n";
        return false;
    }
    if (lambda == nullptr) {
        cerr << "operations to be performed on created database were not specified\n";
        return false;
    }

    error_code err{};
    if (path != IN_MEMORY) {
        if (fs::exists(path, err)) {
            if (override) {
                if (!fs::remove(path)) {
                    cerr << "database file could not be deleted\n";
                    return false;
                }
            }
        } else if (err)
            cerr << "database already exist " << err << "\n";
    }

    auto const flags = SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE;
    auto const database_path = path.string();
    if (SQLITE_OK == sqlite3_open_v2(database_path.c_str(), &db_, flags, nullptr)) {
        if (!lambda(this)) {
            cerr << "error inside lambda\n";
            return false;
        }
        cout << "database created successfully: " << path << '\n';
        return true;
    }
    LOG_ERROR();
    return false;
}

bool sqlite_t::exec(std::string const &query, std::vector<value_t> const &args) const noexcept {
    if (stmt_t(db_).exec(query, args))
        return true;

    LOG_ERROR();
    return false;
}

i64 sqlite_t::insert(std::string const &query, std::vector<value_t> args) const noexcept {
    if (stmt_t(db_).exec(query, std::move(args)))
        return sqlite3_last_insert_rowid(db_);

    LOG_ERROR();
    return -1;
}

i64
sqlite_t::insert(std::string const &table_name, row_t fields) const noexcept {
    auto const [query, values] = query4insert(table_name, fields);
    return insert(query, values);
}

bool
sqlite_t::update(std::string const &table_name, row_t fields, optional<field_t> where) const noexcept {
    auto const [query, values] = query4update(table_name, std::move(fields), std::move(where));
    return update(query, values);
}

optional<result_t>
sqlite_t::select(string const &query, vector<value_t> const &args) const noexcept {
    if (auto result = stmt_t(db_).select(query, args); result)
        return result;
    LOG_ERROR();
    return {};
}

std::pair<std::string, std::vector<value_t>>
query4update(string const &table_name, row_t fields, optional<field_t> where) noexcept {
    auto [names, values] = fields.split();

    stringstream placeholders;
    auto it = names.cbegin();
    auto end = prev(names.end());
    for (; it < end; it++) {
        placeholders << *it << "=?, ";
    }
    placeholders << *it << "=?";

    string query = format("UPDATE {} SET {}", table_name, placeholders.str());

    if (where) {
        auto const [name, value] = *where;
        query += format(" WHERE {}=?", name);
        values.push_back(value);
    }
    query += ";";

    return make_pair(std::move(query), std::move(values));
}

std::pair<std::string, std::vector<value_t>>
query4insert(std::string const &table_name, row_t fields) noexcept {
    auto [names, values] = fields.split();
    vector<string> const placeholders(names.size(), "?");
    string query = format("INSERT INTO {} ({}) VALUES ({});",
                          table_name,
                          join(names),
                          join(placeholders));
    return make_pair(std::move(query), std::move(values));
}

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
