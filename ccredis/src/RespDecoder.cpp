#include "RespDecoder.h"

#include "CommandParsePayload.h"
#include "Resp.h"

#include <cassert>
#include <charconv>
#include <expected>
#include <iostream>
#include <map>
#include <ranges>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

std::pair<size_t, RedisRespRes>
RespDecoder::decodeSimpleString(const std::string_view str) {
  const auto crlfPos = str.find("\r\n");

  if (crlfPos == std::string::npos) {
    throw std::invalid_argument{"Missing CRLF in Simple String"};
  }
  return {crlfPos + 2, RedisRespRes{.string_ = str.substr(1, crlfPos - 1)}};
}

std::pair<size_t, RedisRespRes>
RespDecoder::decodeBulkString(const std::string_view str) {
  const auto lengthDelim = str.find_first_of("\r\n");
  if (lengthDelim == std::string::npos) {
    throw std::invalid_argument{"Missing CRLF in Simple String"};
  }

  if (str.substr(1, lengthDelim - 1) == "-1") {
    return {lengthDelim + 2, RedisRespRes{.string_ = "null"}};
  }

  int length{};
  std::from_chars_result result =
      std::from_chars(str.data() + 1, str.data() + lengthDelim, length);
  if (result.ec == std::errc::invalid_argument) {
    throw std::invalid_argument{"Invalid length of bulk string. lengthDelim: " +
                                std::string{str[lengthDelim]}};
  }

  const auto endDelim = lengthDelim + 2 + length;
  if (str[endDelim] != '\r') {
    throw std::invalid_argument{"Invalid format. End delimiter should come "
                                "after the string. Length mismatch?"};
  }
  if (endDelim == lengthDelim) {
    throw std::invalid_argument{
        "Invalid format. End delimiter same as length delimiter"};
  }
  const auto substr = str.substr(lengthDelim + 2, length);
  if (substr.find("\r\n") != std::string_view::npos) {
    throw std::invalid_argument{"Still CRLF..." + std::string{substr.data()}};
  }
  return {endDelim + 2, RedisRespRes{.string_ = substr}};
}

std::pair<size_t, RedisRespRes>
RespDecoder::decodeError(const std::string_view str) {
  const auto crlfPos = str.find("\r\n");
  if (crlfPos == std::string::npos) {
    throw std::invalid_argument{"Missing CRLF in Simple String"};
  }

  return {crlfPos + 2, RedisRespRes{.error_ = str.substr(1, crlfPos - 1)}};
}

std::pair<size_t, RedisRespRes>
RespDecoder::decodeInt(const std::string_view str) {
  const auto crlfPos = str.find("\r\n");
  if (crlfPos == std::string::npos) {
    throw std::invalid_argument{"Missing CRLF in Simple String"};
  }
  std::string_view decodedInt = str.substr(1, crlfPos - 1);
  // TODO: Verify value is integer with regex
  return {crlfPos + 2, RedisRespRes{.integer_ = std::stoi(decodedInt.data())}};
}

std::pair<size_t, RedisRespRes>
RespDecoder::decodeArray(const std::string_view str) {
  const auto arrLenDel = str.find_first_of("\r\n");
  int arrLen{};
  std::from_chars_result convertArrLenRes =
      std::from_chars(str.data() + 1, str.data() + arrLenDel, arrLen);
  if (convertArrLenRes.ec == std::errc::invalid_argument) {
    throw std::invalid_argument{"Invalid length of bulk string. lengthDelim: " +
                                std::string{str[arrLenDel]}};
  }

  auto startPos = arrLenDel + 2;
  std::vector<RedisRespRes> result{};
  for (int i = 0; i < arrLen; ++i) {
    const auto decodedVal = decode(str.substr(startPos));
    result.push_back(decodedVal.second);
    startPos += decodedVal.first;
  }
  return {startPos, RedisRespRes{.array_ = result}};
}

std::pair<size_t, RedisRespRes>
RespDecoder::decodeMap(const std::string_view str) {
  const auto mapLenDel = str.find_first_of("\r\n");
  int mapLen{};
  std::from_chars_result convertArrLenRes =
      std::from_chars(str.data() + 1, str.data() + mapLenDel, mapLen);
  if (convertArrLenRes.ec == std::errc::invalid_argument) {
    throw std::invalid_argument{"Invalid length of bulk string. lengthDelim: " +
                                std::string{str[mapLenDel]}};
  }

  auto startPos = mapLenDel + 2;
  std::map<std::string_view, RedisRespRes> result{};
  for (int i = 0; i < mapLen; ++i) {
    const auto decodedKey = decode(str.substr(startPos));
    std::string_view key{};
    if (decodedKey.second.string_.has_value()) {
      key = decodedKey.second.string_.value();
    } else {
      std::cout << "Expected key as string, got: " << decodedKey.second;
      assert(false);
    }
    startPos += decodedKey.first;
    const auto decodedVal = decode(str.substr(startPos));
    result[key] = decodedVal.second;
  }
  return {startPos + 2, RedisRespRes{.map_ = result}};
}

std::pair<size_t, RedisRespRes>
RespDecoder::decode(const std::string_view str) {
  // TODO: Implement sets
  // std::cout << __PRETTY_FUNCTION__ << " str= " << str;
  const auto prefix = static_cast<Prefix>(str[0]);
  switch (prefix) {
  case Prefix::SIMPLE_STRING:
    return decodeSimpleString(str);
  case Prefix::ERROR:
    return decodeError(str);
  case Prefix::INTEGER:
    return decodeInt(str);
  case Prefix::BULK_STRING:
    return decodeBulkString(str);
  case Prefix::ARRAY:
    return decodeArray(str);
  case Prefix::MAP:
    return decodeMap(str);
  }
  return {-11, RedisRespRes{}};
}

CommandVariant RespDecoder::parseRawCommand(const std::string_view rawCommand) {
  std::cout << "RawCommand: " << rawCommand << ".\n";
  if (rawCommand == "PING") {
    return CommandPing{};
    // return Command { .kind_ = CommandKind::PING };
  }
  // TODO Use std::expected??
  return CommandUnknown{};
}

CommandVariant RespDecoder::parseRawArrayCommands(
    const std::vector<RedisRespRes> &commandArray) {
  // Command cmd {};
  const auto rawKind = commandArray[0].string_;
  CommandVariant cmd;
  if (rawKind == "HELLO") {
    cmd = CommandHello{};
  } else if (rawKind == "SET") {
    cmd = CommandSet{};
  } else if (rawKind == "GET") {
    cmd = CommandGet{};
  } else if (rawKind == "PING") {
    cmd = CommandPing{};
  } else if (rawKind == "EXISTS") {
    cmd = CommandExists{};
  } else {
    cmd = CommandUnknown{};
  }

  // TODO: Refactor so that ParsePayload takes the entire command array
  for (const auto &rawCommand : commandArray | std::views::drop(1)) {
    const std::expected<ParseSuccessful, CommandInvalid> result =
        std::visit(ParsePayload{rawCommand}, cmd);
    if (!result.has_value()) {
      return result.error();
    }
  }

  /*
  Error handling: Easiest is to throw exceptions in case a command is not
  properly constructed... But how fun is that. Not sure what the alternative
  would be as long as I am using std::visit. If parsePayload takes the commit
  variant and the entire command array, maybe we could use std::expected as
  return type?
  */

  return cmd;
}

std::vector<CommandVariant>
RespDecoder::convertToCommands(const RedisRespRes &rawCommands) {
  std::vector<CommandVariant> commands{};
  if (rawCommands.string_.has_value()) {
    commands.push_back(parseRawCommand(rawCommands.string_.value()));
  } else if (rawCommands.array_.has_value()) {
    commands.push_back(parseRawArrayCommands(rawCommands.array_.value()));
  } else {
    commands.push_back(CommandUnknown{});
  }
  return commands;
}