#include "stmt.h"
#include "row.h"
#include <utility>
#include <algorithm>
using namespace std;

// forward declarations
row_t fetch_row_data(sqlite3_stmt* stmt, int column_count) noexcept;
bool bind2stmt(sqlite3_stmt* stmt, vector<value_t> const& args) noexcept;
bool bind_at(sqlite3_stmt* stmt, int idx, value_t v) noexcept;


stmt_t::~stmt_t() {
    if (stmt_) {
        if (SQLITE_OK == sqlite3_finalize(stmt_))
            stmt_ = nullptr;
        else
            cerr << "statement finalize (" << sqlite3_errmsg(db_) << ")\n";
    }
}

bool stmt_t::exec(std::string const& query, std::vector<value_t> args) noexcept {
    if (!args.empty()) {
        auto const placeholder_count = count_if(query.cbegin(), query.cend(), [](char const c) {
            return '?' == c;
        });
        auto const arg_count = args.size();
        if (placeholder_count != arg_count) {
            cerr << "count of placeholders and arguments do not match ("
            << placeholder_count << ", " << arg_count << ")\n";
            return false;
        }
    }

    if (SQLITE_OK == sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt_, nullptr))
        if (bind2stmt(stmt_, args))
            if (SQLITE_DONE == sqlite3_step(stmt_))
                if (SQLITE_OK == sqlite3_finalize(stmt_)) {
                    stmt_ = nullptr;
                    return true;
                }


    return false;
}

optional<result_t> stmt_t::select(string const& query, vector<value_t> args) noexcept {
    result_t result{};

    if (SQLITE_OK == sqlite3_prepare_v2(db_, query.c_str(), -1, &stmt_, nullptr)) {
        if (bind2stmt(stmt_, args)) {
            if (int n = sqlite3_column_count(stmt_); n > 0) {
                while (SQLITE_ROW == sqlite3_step(stmt_)) {
                    if (auto row = fetch_row_data(stmt_, n); !row.empty())
                        result.push_back(std::move(row));
                }
            }
        }
    }
    if (SQLITE_DONE == sqlite3_errcode(db_))
        if (SQLITE_OK == sqlite3_finalize(stmt_)) {
            stmt_ = nullptr;
            return result;
        }

    return {};
}

//*******************************************************************
//*                                                                 *
//*                        P R I V A T E                            *
//*                                                                 *
//*******************************************************************

row_t fetch_row_data(sqlite3_stmt* const stmt, int const column_count) noexcept {
    row_t row{};

    for (auto i = 0; i < column_count; i++) {
        string name{sqlite3_column_name(stmt, i)};
        switch (sqlite3_column_type(stmt, i)) {
            case SQLITE_NULL:
                row.emplace_back(std::move(name), monostate());
                break;
            case SQLITE_INTEGER:
                row.emplace_back(std::move(name), sqlite3_column_int64(stmt, i));
                break;
            case SQLITE_FLOAT:
                row.emplace_back(std::move(name), sqlite3_column_double(stmt, i));
                break;
            case SQLITE_TEXT: {
                string text{reinterpret_cast<const char *>(sqlite3_column_text(stmt, i))};
                row.emplace_back(std::move(name), std::move(text));
                break;
            }
            case SQLITE_BLOB: {
                auto const ptr { reinterpret_cast<u8 const*>(sqlite3_column_blob(stmt, i))};
                auto const size{ sqlite3_column_bytes(stmt, i)};
                vector<u8> vec{ ptr, ptr + size };
                row.emplace_back(std::move(name), std::move(vec));
            }
        }
    }
    return row;
}

bool bind2stmt(sqlite3_stmt* const stmt, vector<value_t> const& args) noexcept {
    if (!args.empty()) {
        auto idx = 0;
        for (auto v: args)
            if (!bind_at(stmt, ++idx, std::move(v)))
                return false;
    }
    return true;
}

bool bind_at(sqlite3_stmt* const stmt, int const idx, value_t v) noexcept {
    switch (v.index()) {
        case 0:
            return SQLITE_OK == sqlite3_bind_null(stmt, idx);
        case 1:
            return SQLITE_OK == sqlite3_bind_int64(stmt, idx, get<1>(v));
        case 2:
            return SQLITE_OK == sqlite3_bind_double(stmt, idx, get<2>(v));
        case 3: {
            auto const text{ std::move(get<3>(v)) };
            return SQLITE_OK == sqlite3_bind_text(stmt, idx, text.c_str(), -1, SQLITE_TRANSIENT);
        }
        case 4: {
            auto const vec{ std::move(get<4>(v))};
            auto const n{ static_cast<int>(vec.size())};
            return SQLITE_OK == sqlite3_bind_blob(stmt, idx, vec.data(), n, SQLITE_TRANSIENT);
        }
        default:
            return false;
    }
}
