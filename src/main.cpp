#include "dotenv/dotenv.h"
#include "dpp/cluster.h"
#include "dpp/once.h"
#include <optional>

#include "database.hpp"

int main() {
  dotenv::init();

  dpp::cluster bot(std::getenv("BOT_TOKEN"));
  sqlpp::sqlite3::connection db = database::init(std::getenv("DATABASE_PATH"));

  bot.on_log(dpp::utility::cout_logger());

  bot.on_slashcommand([&db](const dpp::slashcommand_t &event) {
    if (event.command.get_command_name() == "ping") {
      event.reply("Pong!");
    } else if (event.command.get_command_name() == "give_stones") {
      const std::string id =
          std::get<dpp::snowflake>(event.get_parameter("user")).str();
      const int to_give = std::get<int64_t>(event.get_parameter("stones"));

      const std::optional<int> money = database::queries::get_money(db, id);

      if (!money.has_value()) {
        database::queries::create_user(db, id);
      }

      const int new_money = money.value_or(0) + to_give;

      database::queries::set_money(db, id, new_money);

      event.reply("<@" + id + "> now has " + std::to_string(new_money) +
                  " stones!");
    }
  });

  bot.on_ready([&bot](const dpp::ready_t &event) {
    if (dpp::run_once<struct register_bot_commands>()) {
      bot.global_command_create(
          dpp::slashcommand("ping", "Ping pong!", bot.me.id));

      dpp::slashcommand give_stones("give_stones",
                                    "ADMIN: Give stones to an user", bot.me.id);
      give_stones.add_option(dpp::command_option(
          dpp::co_user, "user", "The user to give stones to", true));
      give_stones.add_option(dpp::command_option(
          dpp::co_integer, "stones", "The number of stones to give", true));
      give_stones.set_default_permissions(dpp::p_manage_guild);
      bot.global_command_create(give_stones);
    }
  });

  bot.start(dpp::st_wait);

  return 0;
}
