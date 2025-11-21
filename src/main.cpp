#include "dotenv/dotenv.h"
#include "dpp/cluster.h"
#include "dpp/once.h"

#include "commands.hpp"
#include "database.hpp"

int main() {
  dotenv::init();
  std::srand(std::time({}));

  dpp::cluster bot(std::getenv("BOT_TOKEN"));
  sqlpp::sqlite3::connection db = database::init(std::getenv("DATABASE_PATH"));

  commands::command_context ctx{db};
  std::unordered_map<std::string, std::unique_ptr<commands::command>> commands{
      {"ping", std::make_unique<commands::ping>(&ctx)},
      {"give_stones", std::make_unique<commands::give_stones>(&ctx)},
      {"roulette", std::make_unique<commands::roulette>(&ctx)},
  };

  bot.on_log(dpp::utility::cout_logger());

  bot.on_slashcommand([&commands](const dpp::slashcommand_t &event) {
    auto it = commands.find(event.command.get_command_name());
    if (it != commands.end()) {
      it->second->execute(event);
    }
  });

  bot.on_ready([&bot](const dpp::ready_t &event) {
    if (dpp::run_once<struct register_bot_commands>()) {
      bot.global_command_create(
          dpp::slashcommand("ping", "Ping pong!", bot.me.id));

      bot.global_command_create(
          dpp::slashcommand("give_stones", "ADMIN: Give stones to an user",
                            bot.me.id)
              .add_option(dpp::command_option(
                  dpp::co_user, "user", "The user to give stones to", true))
              .add_option(dpp::command_option(dpp::co_integer, "stones",
                                              "The number of stones to give",
                                              true))
              .set_default_permissions(dpp::p_manage_guild));

      bot.global_command_create(
          dpp::slashcommand("roulette", "Play a roulette game", bot.me.id)
              .add_option(dpp::command_option(
                  dpp::co_integer, "money", "The amount of money to bet", true))
              .add_option(
                  dpp::command_option(dpp::co_string, "color",
                                      "The color to bet on", true)
                      .add_choice(dpp::command_option_choice("🔴 Red", "red"))
                      .add_choice(
                          dpp::command_option_choice("⚫ Black", "black"))));
    }
  });

  bot.start(dpp::st_wait);

  return 0;
}
