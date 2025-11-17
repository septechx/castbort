#include "database.hpp"
#include "generated/schema.hpp"
#include "sqlpp23/sqlite3/database/connection.h"
#include <optional>

namespace database {
sqlpp::sqlite3::connection init(const std::string &database_path) {
  auto config = std::make_shared<sqlpp::sqlite3::connection_config>();
  config->path_to_database = database_path;
  config->flags = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE;

  sqlpp::sqlite3::connection db;
  db.connect_using(config);

  return db;
}

namespace queries {
std::optional<int> get_money(sqlpp::sqlite3::connection &db,
                             const std::string &user_id) {
  const castbort::Users users{};
  auto result =
      db(sqlpp::select(users.money).from(users).where(users.id == user_id));

  if (result.empty()) {
    return std::nullopt;
  }

  return result.front().money;
}

void set_money(sqlpp::sqlite3::connection &db, const std::string &user_id,
               int money) {
  const castbort::Users users{};
  db(sqlpp::update(users).set(users.money = money).where(users.id == user_id));
}

void create_user(sqlpp::sqlite3::connection &db, const std::string &user_id) {
  const castbort::Users users{};
  db(sqlpp::insert_into(users).set(users.id = user_id));
}
} // namespace queries
} // namespace database
