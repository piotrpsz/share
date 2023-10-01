#ifndef BEESOFT_SQLITE_STMT
#define BEESOFT_SQLITE_STMT

#include "helper.h"
#include "amalgamation/sqlite3.h"
#include "result.h"

class stmt_t final {
    sqlite3* db_ = nullptr;
    sqlite3_stmt* stmt_ = nullptr;
public:
    explicit stmt_t(sqlite3* db) : db_{db} {};
    ~stmt_t();

    bool exec(std::string const& query, std::vector<value_t> args = {}) noexcept;
    std::optional<result_t> select(std::string const& query, std::vector<value_t> args = {}) noexcept;

};


#endif // BEESOFT_SQLITE_STMT
