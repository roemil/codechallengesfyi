#include "CommandParsePayload.h"
#include "Commands.h"
#include "Resp.h"

#include <cassert>
#include <charconv>
#include <chrono>
#include <string>

void ParsePayload::operator()(CommandUnknown &) { return; }
void ParsePayload::operator()(CommandInvalid &) { return; }
void ParsePayload::operator()(CommandPing & cmd) {
  if (resp_.string_.has_value()) {
    cmd.value_ = resp_.string_.value();
  }
}
void ParsePayload::operator()(CommandHello &cmd) {
  assert(resp_.string_.has_value()); // Version is passed as string in client
  cmd.version_ = resp_.string_.value();
}
void ParsePayload::operator()(CommandSet &cmd) {
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
      assert(ec == std::errc()); // TODO: Error handling :)
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
      cmd.state_ = CommandState::Done;
    }
  }
}
void ParsePayload::operator()(CommandGet &cmd) {
  if (resp_.string_.has_value()) {
    cmd.key_ = resp_.string_.value();
  } else if (resp_.integer_.has_value()) {
    cmd.key_ = std::to_string(resp_.integer_.value());
  }
}

void ParsePayload::operator()(CommandExists &cmd) {
  if (resp_.string_.has_value()) {
    cmd.key_ = resp_.string_.value();
  } else if (resp_.integer_.has_value()) {
    cmd.key_ = std::to_string(resp_.integer_.value());
  }
}
