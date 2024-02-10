#include "CommandHandler.h"
#include "Commands.h"
#include "Resp.h"

#include <string>

void ParsePayload::operator()(CommandUnknown&)
{
    return;
}
void ParsePayload::operator()(CommandInvalid&)
{
    return;
}
void ParsePayload::operator()(CommandPing&)
{
    return;
}
void ParsePayload::operator()(CommandHello& cmd)
{
    assert(resp_.string_.has_value()); // Version is passed as string in client
    cmd.version_ = resp_.string_.value();
}
void ParsePayload::operator()(CommandSet& cmd)
{
    if (cmd.key_.empty()) {
        if (resp_.string_.has_value()) {
            cmd.key_ = resp_.string_.value();
        } else if (resp_.integer_.has_value()) {
            cmd.key_ = std::to_string(resp_.integer_.value());
        }
    } else {
        if (resp_.string_.has_value()) {
            cmd.value_ = resp_.string_.value();
        } else if (resp_.integer_.has_value()) {
            cmd.value_ = std::to_string(resp_.integer_.value());
        }
    }
}
void ParsePayload::operator()(CommandGet& cmd)
{
    if (resp_.string_.has_value()) {
        cmd.key_ = resp_.string_.value();
    } else if (resp_.integer_.has_value()) {
        cmd.key_ = std::to_string(resp_.integer_.value());
    }
}