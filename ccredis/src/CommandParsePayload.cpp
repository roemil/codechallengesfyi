#include "CommandParsePayload.h"
#include "Commands.h"
#include "Resp.h"

#include <cassert>
#include <charconv>
#include <chrono>
#include <expected>
#include <string>

std::expected<ParseSuccessful, CommandInvalid>
ParsePayload::operator()(CommandUnknown &) {
  return ParseSuccessful{};
}
std::expected<ParseSuccessful, CommandInvalid>
ParsePayload::operator()(CommandInvalid &) {
  return ParseSuccessful{};
}
std::expected<ParseSuccessful, CommandInvalid>
ParsePayload::operator()(CommandPing &cmd) {
  if (resp_.string_.has_value()) {
    cmd.value_ = resp_.string_.value();
  }
  return ParseSuccessful{};
}
std::expected<ParseSuccessful, CommandInvalid>
ParsePayload::operator()(CommandHello &cmd) {
  assert(resp_.string_.has_value()); // Version is passed as string in client
  cmd.version_ = resp_.string_.value();
  if (cmd.version_.empty()) {
    CommandInvalid invalidCmd{};
    invalidCmd.errorString = "Missing version";
    return std::unexpected{invalidCmd};
  }
  return ParseSuccessful{};
}
std::expected<ParseSuccessful, CommandInvalid>
ParsePayload::operator()(CommandSet &cmd) {
  if (cmd.key_.empty()) {
    if (resp_.string_.has_value()) {
      cmd.key_ = resp_.string_.value();
    } else if (resp_.integer_.has_value()) {
      cmd.key_ = std::to_string(resp_.integer_.value());
    }
  } else if (cmd.value_.empty()) {
    if (resp_.string_.has_value()) {
      cmd.value_ = resp_.string_.value();
    } else if (resp_.integer_.has_value()) {
      cmd.value_ = std::to_string(resp_.integer_.value());
    }
  } else {
    // Optional data, eg EX, PX
    if (resp_.string_.has_value() && resp_.string_.value() == "EX") {
      cmd.state_ = CommandState::WaitingForValue;
      cmd.resolution_ = ExpireTimeResolution::Seconds;
    } else if (resp_.string_.has_value() && resp_.string_.value() == "PX") {
      cmd.state_ = CommandState::WaitingForValue;
      cmd.resolution_ = ExpireTimeResolution::Milliseconds;
    } else if (resp_.string_.has_value() &&
               cmd.state_ == CommandState::WaitingForValue) {

      int time{};
      const auto [_, ec] = std::from_chars(resp_.string_.value().begin(),
                                           resp_.string_.value().end(), time);
      if (ec != std::errc()) {
        CommandInvalid invalidCmd{};
        invalidCmd.errorString = "Invalid expiration time";
        return std::unexpected{invalidCmd};
      }
      const auto now = std::chrono::duration_cast<std::chrono::seconds>(
          std::chrono::system_clock::now().time_since_epoch());
      std::chrono::time_point<std::chrono::system_clock,
                              std::chrono::milliseconds>
          timeS{std::chrono::seconds{time} + now};
      cmd.expire = timeS;
      if (cmd.resolution_ == ExpireTimeResolution::Milliseconds) {
        const auto now = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch());
        std::chrono::time_point<std::chrono::system_clock,
                                std::chrono::milliseconds>
            timeMs{std::chrono::milliseconds{time} + now};
        cmd.expire = timeMs;
      }
    }
  }
  cmd.state_ = CommandState::Done;
  return ParseSuccessful{};
}
std::expected<ParseSuccessful, CommandInvalid>
ParsePayload::operator()(CommandGet &cmd) {
  if (resp_.string_.has_value()) {
    cmd.key_ = resp_.string_.value();
  } else if (resp_.integer_.has_value()) {
    cmd.key_ = std::to_string(resp_.integer_.value());
  }

  if (cmd.key_.empty()) {
    CommandInvalid invalidCmd{};
    invalidCmd.errorString = "Missing key";
    return std::unexpected{invalidCmd};
  }

  return ParseSuccessful{};
}

std::expected<ParseSuccessful, CommandInvalid>
ParsePayload::operator()(CommandExists &cmd) {
  if (resp_.string_.has_value()) {
    cmd.key_ = resp_.string_.value();
  } else if (resp_.integer_.has_value()) {
    cmd.key_ = std::to_string(resp_.integer_.value());
  }
  if (cmd.key_.empty()) {
    CommandInvalid invalidCmd{};
    invalidCmd.errorString = "Missing key";
    return std::unexpected{invalidCmd};
  }
  return ParseSuccessful{};
}
