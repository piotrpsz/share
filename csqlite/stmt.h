#pragma once

/*------- include files:
-------------------------------------------------------------------*/
#include "amalgamation/sqlite3.h"
#include <source_location>
#include "result.h"
#include "query.h"

inline static void LOG_ERROR(sqlite3 *const db, std::source_location sl = std::source_location::current()) {
    std::cerr << fmt::format("SQLite Error: {} ({}) => fn::{}().{} [{}]\n",
                             sqlite3_errmsg(db),
                             sqlite3_errcode(db),
                             sl.function_name(),
                             sl.line(),
                             sl.file_name());
}

class stmt_t final {
    sqlite3 *db_ = nullptr;
    sqlite3_stmt *stmt_ = nullptr;
public:
    explicit stmt_t(sqlite3 *db) : db_{db} {};
    stmt_t(stmt_t const&) = default;
    stmt_t(stmt_t&&) = default;
    stmt_t& operator=(stmt_t const&) = default;
    stmt_t& operator=(stmt_t&&) = default;
    ~stmt_t();

    bool exec(query_t const& query) noexcept;
    std::optional<result_t> select(query_t const& query) noexcept;

    constexpr bool exec(std::string const& str, std::vector<value_t> const& args = {}) noexcept {
        return exec(query_t{str, args});
    }
    constexpr std::optional<result_t> select(std::string const& str, const std::vector<value_t>& args = {}) noexcept {
        return select(query_t{str, args});
    }
};
