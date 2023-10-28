#pragma once

/*------- include files:
-------------------------------------------------------------------*/
#include <array>
#include <string>
#include <functional>
#include <utility>
#include <initializer_list>
#include <filesystem>
#include "amalgamation/sqlite3.h"
#include "../share.h"
#include "result.h"
#include "row.h"
#include "result.h"
#include "query.h"
#include "stmt.h"

class sqlite_t final {
    static inline std::array<u8, 16> header = {
            0x53, 0x51, 0x4c, 0x69, 0x74, 0x65, 0x20, 0x66,
            0x6f, 0x72, 0x6d, 0x61, 0x74, 0x20, 0x33, 0x00
    };
    sqlite3 *db_ = nullptr;
public:
    static inline std::string IN_MEMORY{":memory:"};

    static sqlite_t &instance() {
        static sqlite_t sq;
        return sq;
    }

    // no copy
    sqlite_t(sqlite_t const &) = delete;
    sqlite_t &operator=(sqlite_t const &) = delete;
    // no move
    sqlite_t(sqlite_t &&) = delete;
    sqlite_t &operator=(sqlite_t &&) = delete;

    static std::string version() noexcept {
        return sqlite3_version;
    }

    bool
    close() noexcept;

    bool
    open(fs::path const &path, bool read_only = false) noexcept;

    bool
    create(fs::path const &path, std::function<bool(sqlite_t *)> const &lambda, bool override = false) noexcept;

    //======= EXEC ==================================================
    [[nodiscard]] bool
    exec(query_t const& query ) const noexcept {
        return stmt_t(db_).exec(query);
    }
    [[nodiscard]] bool
    exec(std::string const& str, std::vector<value_t> const &args = {}) const noexcept {
        return exec(query_t{str, args});
    }
    //======= INSERT ================================================
    [[nodiscard]] i64
    insert(query_t const& query) const noexcept {
        if (stmt_t(db_).exec(query))
            return sqlite3_last_insert_rowid(db_);
        return -1;
    }
    [[nodiscard]] constexpr i64
    insert(std::string const& str, std::initializer_list<value_t> args) const noexcept {
        return insert(query_t{str, std::vector<value_t>(args)});
    }
    [[nodiscard]] constexpr i64
    insert(std::string const& str, std::vector<value_t> const& args = {}) const noexcept {
        return insert(query_t{str, args});
    }
    [[nodiscard]] constexpr i64
    insert(std::string const &table_name, row_t const& fields) const noexcept {
        return insert(query4insert(table_name, fields));
    };

    //======= UPDATE ================================================
    [[nodiscard]] constexpr bool
    update(query_t const& query) const noexcept {
        return exec(query);
    }
    [[nodiscard]] constexpr bool
    update(std::string const& str, std::initializer_list<value_t> const &args = {}) const noexcept {
        return exec(query_t{str, std::vector<value_t>(args)});
    }
    [[nodiscard]] constexpr bool
    update(std::string const& str, std::vector<value_t> const& args) const noexcept {
        return exec(query_t{str, args});
    }
    [[nodiscard]] constexpr bool
    update(std::string const& table_name, row_t const& fields, std::optional<field_t> const& where) const noexcept {
        return update(query4update(table_name, fields, where));
    }

    //======= SELECT ================================================
    [[nodiscard]] std::optional<result_t>
    select(query_t const& query) const noexcept {
        return stmt_t(db_).select(query);
    }
    [[nodiscard]] constexpr std::optional<result_t>
    select(std::string const& str, std::vector<value_t> const &args = {}) const noexcept {
        return select(query_t{str, args});
    }

    static query_t query4insert(std::string const &table_name, row_t const& fields) noexcept;
    static query_t query4update(std::string const &table_name, row_t const& fields, std::optional<field_t> const& where) noexcept;

private:
    sqlite_t() {
        sqlite3_initialize();
    }
};

