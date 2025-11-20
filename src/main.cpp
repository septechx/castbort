#include "dotenv/dotenv.h"
#include "dpp/appcommand.h"
#include "dpp/cluster.h"
#include "dpp/once.h"
#include <optional>

#include "database.hpp"
#include "video_generator.hpp"

enum class Color { red, black, green };

unsigned bounded_rand(unsigned range) {
  for (unsigned x, r;;)
    if (x = rand(), r = x % range, x - r <= -range)
      return r;
}

int give_money(sqlpp::sqlite3::connection &db, const std::string &id,
               int to_give) {
  const std::optional<int> money = database::queries::get_money(db, id);

  if (!money.has_value())
    database::queries::create_user(db, id);

  const int new_money = money.value_or(0) + to_give;

  database::queries::set_money(db, id, new_money);

  return new_money;
}

int subtract_money(sqlpp::sqlite3::connection &db, const std::string &id,
                   int to_subtract) {
  const std::optional<int> money = database::queries::get_money(db, id);

  if (!money.has_value())
    database::queries::create_user(db, id);

  const int new_money = money.value_or(0) - to_subtract;

  database::queries::set_money(db, id, new_money);

  return new_money;
}

std::string bold(const std::string &str) { return "**" + str + "**"; }

int main() {
  dotenv::init();
  std::srand(std::time({}));

  dpp::cluster bot(std::getenv("BOT_TOKEN"));
  sqlpp::sqlite3::connection db = database::init(std::getenv("DATABASE_PATH"));

  bot.on_log(dpp::utility::cout_logger());

  bot.on_slashcommand([&db](const dpp::slashcommand_t &event) {
    const std::string cmd = event.command.get_command_name();
    if (cmd == "ping") {
      event.reply("Pong!");
    } else if (cmd == "give_stones") {
      const std::string id =
          std::get<dpp::snowflake>(event.get_parameter("user")).str();
      const int to_give = std::get<int64_t>(event.get_parameter("stones"));

      const int new_money = give_money(db, id, to_give);

      event.reply("<@" + id + "> now has " + bold(std::to_string(new_money)) +
                  " stones");
    } else if (cmd == "roulette") {
      const int spent = std::get<int64_t>(event.get_parameter("money"));
      const std::string color =
          std::get<std::string>(event.get_parameter("color"));
      const std::string user_id = event.command.get_issuing_user().id.str();

      event.thinking(false, [event, &db, user_id, spent, color](
                                const dpp::confirmation_callback_t &callback) {
        const int rnd = bounded_rand(99) + 1;
        const Color clr = rnd < 50   ? Color::red
                          : rnd > 50 ? Color::black
                                     : Color::green;
        const bool won = (clr == Color::red && color == "red") ||
                         (clr == Color::black && color == "black");
        const int new_money = won ? give_money(db, user_id, spent)
                                  : subtract_money(db, user_id, spent);
        const std::string clr_str = clr == Color::red     ? "🔴 Red"
                                    : clr == Color::black ? "⚫ Black"
                                                          : "🟢 Green";

        const std::string video_bytes =
            generate_video("assets/castor.png", "assets/overlay.png");
        dpp::message msg(event.command.channel_id, "Spinning...");
        msg.add_file("out.gif", video_bytes, "image/gif");

        event.edit_original_response(msg);

        sleep(11);

        event.edit_original_response(dpp::message(
            "Ball landed on " + clr_str + ".\nYou " + (won ? "won" : "lost") +
            " " + bold(std::to_string(spent)) + " stones, and now have " +
            bold(std::to_string(new_money)) + " stones"));
      });
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
