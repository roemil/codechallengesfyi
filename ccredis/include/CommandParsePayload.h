#pragma once

#include <expected>

struct CommandUnknown;
struct CommandInvalid;
struct CommandPing;
struct CommandHello;
struct CommandSet;
struct CommandGet;
struct CommandExists;

struct RedisRespRes;
class RespEncoder;

struct ParseSuccessful;

struct ParsePayload {
  ParsePayload(const RedisRespRes &resp) : resp_(resp) {}
  const RedisRespRes &resp_;

  ParsePayload(const ParsePayload &) = delete;
  ParsePayload operator=(const ParsePayload &) = delete;

  std::expected<ParseSuccessful, CommandInvalid> operator()(CommandUnknown &);
  std::expected<ParseSuccessful, CommandInvalid> operator()(CommandInvalid &);
  std::expected<ParseSuccessful, CommandInvalid> operator()(CommandPing &);
  std::expected<ParseSuccessful, CommandInvalid> operator()(CommandHello &);
  std::expected<ParseSuccessful, CommandInvalid> operator()(CommandSet &);
  std::expected<ParseSuccessful, CommandInvalid> operator()(CommandGet &);
  std::expected<ParseSuccessful, CommandInvalid> operator()(CommandExists &);
};
