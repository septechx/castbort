#pragma once

#include "dpp/dispatcher.h"
#include "sqlpp23/sqlite3/database/connection.h"

enum class Color { red, black, green };

namespace commands {
struct command_context {
  sqlpp::sqlite3::connection &db;
};

class command {
public:
  command_context *ctx;

  command(command_context *ctx) : ctx(ctx) {}

  virtual ~command() = default;
  virtual void execute(const dpp::slashcommand_t &event) = 0;
};

class ping : public command {
public:
  ping(command_context *ctx) : command(ctx) {}

  void execute(const dpp::slashcommand_t &event) override;
};

class give_stones : public command {
public:
  give_stones(command_context *ctx) : command(ctx) {}

  void execute(const dpp::slashcommand_t &event) override;
};

class roulette : public command {
public:
  roulette(command_context *ctx) : command(ctx) {}

  void execute(const dpp::slashcommand_t &event) override;
};
} // namespace commands
