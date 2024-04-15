#pragma once

#include "IIrcClient.h"
#include "Channel.h"

#include <string_view>
#include <string>
#include <vector>
#include <poll.h>
#include <Tui.h>
#include <cassert>


struct Counters{
    int ping = 0;
    int unknown = 0;
    int join = 0;
    int part = 0;
    int other = 0;
    int mode = 0;
    int notice = 0;
    int nick = 0;
    int privMsg = 0;
};

class IrcClient : public IIrcClient
{
    public:
        IrcClient(const std::string_view nick, const std::string_view realname) : nick_(nick), realname(realname) {}
        IrcClient(Tui* tui, const std::string_view nick, const std::string_view realname) : tui_(tui), nick_(nick), realname(realname) {}
        void connect(const std::string_view server);

        Irc::Channel currentChannel{};

        void incPingCounter() override;
        void incUnknownCounter() override;
        void incJoinCounter() override;
        void incPartCounter() override;
        void incOtherCounter() override;
        void incModeCounter() override;
        void incNoticeCounter() override;
        void incNickCounter() override;
        void incPrivMsgCounter() override;

        void setNick(const std::string_view nick) override;
        void setChannelName(const std::string_view ch) override;
        void uiCmd(const std::string_view cmd) override;

        void addUserToUi(const std::string_view user) override;

        Tui& getTui() override {
            assert(tui_);
            return *tui_;
        }

        Tui* tui_;
    private:
        Counters counters{};
        std::string nick_;
        std::string realname;
        int socket_{};
        bool isRegistered{};

        void startPoll();
        void handleServerReply();

        std::vector<pollfd> fds_{};


};