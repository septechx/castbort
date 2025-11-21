#include "commands.hpp"
#include "database.hpp"
#include "video_generator.hpp"

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

namespace commands {
void ping::execute(const dpp::slashcommand_t &event) { event.reply("Pong!"); }

void give_stones::execute(const dpp::slashcommand_t &event) {
  const std::string id =
      std::get<dpp::snowflake>(event.get_parameter("user")).str();
  const int to_give = std::get<int64_t>(event.get_parameter("stones"));

  const int new_money = give_money(ctx->db, id, to_give);

  event.reply("<@" + id + "> now has " + bold(std::to_string(new_money)) +
              " stones");
}

void roulette::execute(const dpp::slashcommand_t &event) {
  const int spent = std::get<int64_t>(event.get_parameter("money"));
  const std::string color = std::get<std::string>(event.get_parameter("color"));
  const std::string user_id = event.command.get_issuing_user().id.str();

  event.thinking(false, [event, this, user_id, spent,
                         color](const dpp::confirmation_callback_t &callback) {
    const int rnd = bounded_rand(99) + 1;
    const Color clr = rnd < 50   ? Color::red
                      : rnd > 50 ? Color::black
                                 : Color::green;
    const bool won = (clr == Color::red && color == "red") ||
                     (clr == Color::black && color == "black");
    const int new_money = won ? give_money(ctx->db, user_id, spent)
                              : subtract_money(ctx->db, user_id, spent);
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
        "Ball landed on " + clr_str + ".\nYou " + (won ? "won" : "lost") + " " +
        bold(std::to_string(spent)) + " stones, and now have " +
        bold(std::to_string(new_money)) + " stones"));
  });
}
} // namespace commands
