#pragma once

#include <string_view>

class Tui;

class IIrcClient
{
    public:
        virtual ~IIrcClient() {};
        virtual void incPingCounter() = 0;
        virtual void incUnknownCounter() = 0;
        virtual void incJoinCounter() = 0;
        virtual void incPartCounter() = 0;
        virtual void incOtherCounter() = 0;
        virtual void incModeCounter() = 0;
        virtual void incNoticeCounter() = 0;
        virtual void incNickCounter() = 0;
        virtual void incPrivMsgCounter() = 0;

        virtual void setNick(const std::string_view) = 0;
        virtual void setChannelName(const std::string_view) = 0;
        virtual void uiCmd(const std::string_view) = 0;
        virtual void addUserToUi(const std::string_view user) = 0;

        virtual Tui& getTui() = 0;
};