#pragma once
#include <array>
#include <string>
#include <functional>
#include <utility>
#include <initializer_list>
#include "amalgamation/sqlite3.h"
#include "result.h"
#include "helper.h"
#include "row.h"
#include "result.h"

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

    [[nodiscard]] bool
    exec(std::string const &query, std::vector<value_t> const &args = {}) const noexcept;

    [[nodiscard]] i64
    insert(std::string const &query, std::initializer_list<value_t> args) const noexcept {
        return insert(query, std::vector<value_t>(args));
    }

    [[nodiscard]] i64
    insert(std::string const &query, std::vector<value_t> args = {}) const noexcept;

    [[nodiscard]] i64
    insert(std::string const &table_name, row_t fields) const noexcept;

    [[nodiscard]] bool
    update(std::string const &query, std::initializer_list<value_t> const &args = {}) const noexcept {
        return exec(query, std::vector<value_t>(args));
    }

    [[nodiscard]] bool
    update(std::string const &query, std::vector<value_t> const &args) const noexcept {
        return exec(query, args);
    }

    [[nodiscard]] bool
    update(std::string const &table_name, row_t fields, std::optional<field_t> where = {}) const noexcept;

    [[nodiscard]] std::optional<result_t>
    select(std::string const &query, std::vector<value_t> const &args = {}) const noexcept;


private:
    sqlite_t() {
        sqlite3_initialize();
    }

    [[nodiscard]] std::string
    err_msg() const noexcept {
        return {sqlite3_errmsg(db_)};
    }

    [[nodiscard]] int
    err_code() const noexcept {
        return sqlite3_errcode(db_);
    }
};

std::pair<std::string, std::vector<value_t>>
query4insert(std::string const &table_name, row_t fields) noexcept;

std::pair<std::string, std::vector<value_t>>
query4update(std::string const &table_name, row_t fields, std::optional<field_t> where) noexcept;

