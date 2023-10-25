/*------- include files:
-------------------------------------------------------------------*/
#include "stmt.h"
#include "row.h"
#include <utility>
#include <fmt/core.h>

/*------- forward declarations:
-------------------------------------------------------------------*/
row_t fetch_row_data(sqlite3_stmt* stmt, int column_count) noexcept;
bool bind2stmt(sqlite3_stmt* stmt, std::vector<value_t> const& args) noexcept;
bool bind_at(sqlite3_stmt* stmt, int idx, value_t const& v) noexcept;

/// Destruktor jeśli trzeba finalizuje zapytanie (nie powinno się zdarzyć).
stmt_t::~stmt_t() {
    if (stmt_) {
        if (SQLITE_OK == sqlite3_finalize(stmt_)) {
            stmt_ = nullptr;
            return;
        }
        LOG_ERROR(db_);
    }
}

/// Wykonanie zapytania, które nie zwraca wartości.
/// \param query - zapytanie do wykonania.
/// \return True jeśli wykonanie było bezbłędne, False w przeciwnym przypadku.
bool stmt_t::
exec(query_t const& query) noexcept {
    if (query.valid())
        if (SQLITE_OK == sqlite3_prepare_v2(db_, query.query().c_str(), -1, &stmt_, nullptr))
            if (bind2stmt(stmt_, query.values()))
                if (SQLITE_DONE == sqlite3_step(stmt_))
                    if (SQLITE_OK == sqlite3_finalize(stmt_)) {
                        stmt_ = nullptr;
                        return true;
                    }

    LOG_ERROR(db_);
    return false;
}

/// Wykonanie zapytania SELECT.
/// \param query - obiekt zapytania (zapytanie wraz z wartościami)
/// \return opcjonalny wektor wierszy (obiekt result_t)
std::optional<result_t> stmt_t::
select(query_t const& query) noexcept {
    result_t result{};

    if (query.valid())
        if (SQLITE_OK == sqlite3_prepare_v2(db_, query.query().c_str(), -1, &stmt_, nullptr))
            if (bind2stmt(stmt_, query.values()))
                if (auto n = sqlite3_column_count(stmt_); n > 0) {
                    while (SQLITE_ROW == sqlite3_step(stmt_))
                        if (auto row = fetch_row_data(stmt_, n); !row.empty())
                            result.push_back(row);
                }

    if (SQLITE_DONE == sqlite3_errcode(db_))
        if (SQLITE_OK == sqlite3_finalize(stmt_)) {
            stmt_ = nullptr;
            return result;
        }

    LOG_ERROR(db_);
    return {};
}

//*******************************************************************
//*                                                                 *
//*                        P R I V A T E                            *
//*                                                                 *
//*******************************************************************

/// Odczytanie jednego wiersza z bazy danych (czyli wszystkich zwróconych pól).
/// \param stmt
/// \param column_count - liczba pól w weirszu.
/// \return wektor pól (row_t) - może być pusty
row_t fetch_row_data(sqlite3_stmt* const stmt, int const column_count) noexcept {
    row_t row{};

    for (auto i = 0; i < column_count; i++) {
        std::string name{sqlite3_column_name(stmt, i)};
        switch (sqlite3_column_type(stmt, i)) {
            case SQLITE_NULL:
                row.emplace_back(std::move(name), {});
                break;
            case SQLITE_INTEGER:
                row.emplace_back(std::move(name), sqlite3_column_int64(stmt, i));
                break;
            case SQLITE_FLOAT:
                row.emplace_back(std::move(name), sqlite3_column_double(stmt, i));
                break;
            case SQLITE_TEXT: {
                std::string text{reinterpret_cast<const char *>(sqlite3_column_text(stmt, i))};
                row.emplace_back(std::move(name), std::move(text));
                break;
            }
            case SQLITE_BLOB: {
                auto const ptr { reinterpret_cast<u8 const*>(sqlite3_column_blob(stmt, i))};
                auto const size{ sqlite3_column_bytes(stmt, i)};
                std::vector<u8> vec{ ptr, ptr + size };
                row.emplace_back(std::move(name), std::move(vec));
            }
        }
    }
    return row;
}

/// Dodanie wszystkich wartości do zapytania ('?' - placeholder).
/// \param stmt
/// \param args - wektor wartości, które należy dodać
/// \return  True jeśli wykonanie było bezbłędne, False w przeciwnym przypadku.
bool bind2stmt(sqlite3_stmt* const stmt, std::vector<value_t> const& args) noexcept {
    if (!args.empty()) {
        auto idx = 0;
        for (auto const& v: args)
            if (!bind_at(stmt, ++idx, v))
                return false;
    }
    return true;
}

/// Dodanie wskazanej wartości do wskazanego pola w zapytaniu.
/// \param stmt
/// \param idx - numer pola, do którego należy dodać wartość
/// \param v - wartość którą należy dodać.
/// \return  True jeśli wykonanie było bezbłędne, False w przeciwnym przypadku.
bool bind_at(sqlite3_stmt* const stmt, int const idx, value_t const& v) noexcept {
    switch (v.index()) {
        case MONOSTATE_INDEX:
            return SQLITE_OK == sqlite3_bind_null(stmt, idx);
        case INTEGER_INDEX:
            return SQLITE_OK == sqlite3_bind_int64(stmt, idx, v.int64());
        case DOUBLE_INDEX:
            return SQLITE_OK == sqlite3_bind_double(stmt, idx, v.float64());
        case STRING_INDEX: {
            auto const text{ v.str() };
            return SQLITE_OK == sqlite3_bind_text(stmt, idx, text.c_str(), -1, SQLITE_TRANSIENT); }
        case VECTOR_INDEX: {
            auto const vec{ v.vec() };
            auto const n{ static_cast<int>(vec.size())};
            return SQLITE_OK == sqlite3_bind_blob(stmt, idx, vec.data(), n, SQLITE_TRANSIENT); }
        default:
            return false;
    }
}
