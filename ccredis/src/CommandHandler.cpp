#include "CommandHandler.h"

#include "Commands.h"
#include "Database.h"
#include "RespEncoder.h"

void CommandHandler::operator()(const CommandUnknown &) {
  encoder_->appendError(
      "Unknown command. Prefix did not match any expected prefixes");
}
void CommandHandler::operator()(const CommandInvalid &) {
  encoder_->appendError("Invalid Command");
}
void CommandHandler::operator()(const CommandPing &) {
  encoder_->appendBulkstring("PONG");
}
void CommandHandler::operator()(const CommandHello &cmd) {
  if (cmd.version_ != "3") {
    std::string error{"Version not supported: "};
    encoder_->appendError(error + cmd.version_.data());
    return;
  } else if (cmd.version_.empty()) {
    encoder_->appendError("Missing version");
    return;
  }
  encoder_->beginMap(3);
  encoder_->appendKV("server", "redis");
  encoder_->appendKV("version", "0.0.1");
  encoder_->appendKV("proto", 3);
}
void CommandHandler::operator()(const CommandSet &cmd) {
  if (cmd.expire.has_value()) {
    db_->set(cmd.key_, cmd.value_, cmd.expire.value());
  } else {
    db_->set(cmd.key_, cmd.value_);
  }
  encoder_->appendSimpleString("OK");
}
void CommandHandler::operator()(const CommandGet &cmd) {
  const auto &value_ = db_->get(cmd.key_);
  if (value_.has_value()) {
    encoder_->appendBulkstring(value_.value());
  } else {
    encoder_->appendError("Key does not exist");
  }
}
void CommandHandler::operator()(const CommandExists &cmd) {
  const auto &value_ = db_->get(cmd.key_);
  if (value_.has_value()) {
    encoder_->appendInt("1");
  } else {
    encoder_->appendInt("0");
  }
}