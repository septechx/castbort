#pragma once

#include "sqlpp23/sqlite3/database/connection.h"
#include <optional>
#include <string>

namespace database {
sqlpp::sqlite3::connection init(const std::string &database_path);

namespace queries {
std::optional<int> get_money(sqlpp::sqlite3::connection &db,
                             const std::string &user_id);
void set_money(sqlpp::sqlite3::connection &db, const std::string &user_id,
               int money);
void create_user(sqlpp::sqlite3::connection &db, const std::string &user_id);
} // namespace queries
} // namespace database
