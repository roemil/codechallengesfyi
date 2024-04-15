#pragma once

#include "IrcClient.h"
#include "IIrcClient.h"
#include <cstddef>
#include <string_view>
#include <iostream>
#include <memory>
#include <cassert>
#include <sstream>
#include <vector>
#include "Tui.h"

struct UnknownMessage
{
    void print(std::ostream& os) const {
        os << "Unknown Message\n";
    }
    friend bool operator==([[maybe_unused]]const UnknownMessage& lhs, [[maybe_unused]]const UnknownMessage& rhs) {
        return true;
    }

};

struct UserMessage
{

    friend bool operator==(const UserMessage& lhs, const UserMessage& rhs) {
        return lhs.users_ == rhs.users_ && lhs.origin_ == rhs.origin_ && lhs.target_ == rhs.target_;;
    }
    void print(std::ostream& os) const {
        os << "User Message\n";
        os << "Origin = " << origin_ << "\n";
        os << "Target = " << target_ << "\n";
        os << "Users = ";
        for(const auto user : users_){
            std::cout << user << "\n";
        }
    }

    std::vector<std::string_view> users_{};
    std::string_view origin_{};
    std::string_view target_{};
};

struct PingMessage
{
    constexpr PingMessage() = default;
    constexpr PingMessage(const std::string_view data) : data_(data) {}

    friend bool operator==(const PingMessage& lhs, const PingMessage& rhs) {
        return lhs.data_ == rhs.data_;
    }
    void print(std::ostream& os) const {
        os << "Ping Message\n";
        os << "Data = " << data_ << ".\n";
    }

    std::string_view data_{};
};

struct ModeMessage
{
    constexpr ModeMessage() = default;
    constexpr ModeMessage(const std::string_view data) : data_(data) {}

    friend bool operator==(const ModeMessage& lhs, const ModeMessage& rhs) {
        return lhs.data_ == rhs.data_ && lhs.origin_ == rhs.origin_
        && lhs.mode_ == rhs.mode_ && lhs.target_ == rhs.target_;
    }
    void print(std::ostream& os) const {
        os << "Mode Message\n";
        os << "Data = " << data_ << ".\n";
        os << "Origin = " << origin_ << ".\n";
        os << "Mode = " << mode_ << ".\n";
        os << "Target = " << target_ << ".\n";
    }

    std::string_view data_{};
    std::string_view origin_{};
    std::string_view mode_{};
    std::string_view target_{};
};

struct PrivMsgMessage
{
    constexpr PrivMsgMessage() = default;
    PrivMsgMessage(const std::string_view data) : data_(data) {}

    friend bool operator==(const PrivMsgMessage& lhs, const PrivMsgMessage& rhs) {
        return lhs.data_ == rhs.data_ && lhs.origin_ == rhs.origin_
        && lhs.text_ == rhs.text_ && lhs.target_ == rhs.target_;
    }
    void print(std::ostream& os) const {
        os << "PrivMsg Message\n";
        os << "Data = " << data_ << ".\n";
        os << "Origin = " << origin_ << ".\n";
        os << "Text = ";
        for(const auto str : text_){
            os << str << " ";
        }
        os << "\n";
        os << "Target = " << target_ << ".\n";
    }

    std::string_view data_{};
    std::string_view origin_{};
    std::vector<std::string_view> text_{};
    std::string_view target_{};
};

struct OtherMessage
{
    constexpr OtherMessage() = default;

    friend bool operator==(const OtherMessage& lhs, const OtherMessage& rhs) {
        return lhs.text_ == rhs.text_ && lhs.origin_ == rhs.origin_ && lhs.target_ == rhs.target_;
    }
    void print(std::ostream& os) const {
        os << "Other Message\n";
        os << "Origin = " << origin_ << "\n";
        os << "Target = " << target_ << "\n";
        os << "Text = ";
        for(const auto str : text_){
            os << str << " ";
        }
        os << "\n";
    }
    std::string_view origin_{};
    std::string_view target_{};
    std::vector<std::string_view> text_{};
};

struct NoticeMessage
{
    friend bool operator==(const NoticeMessage& lhs, const NoticeMessage& rhs) {
        return lhs.text == rhs.text && lhs.origin_ == rhs.origin_ && lhs.target_ == rhs.target_;
    }
    void print(std::ostream& os) const {
        os << "Notice Message\n";
        os << "Origin = " << origin_ << "\n";
        os << "Target = " << target_ << "\n";
        os << " Text = ";
        for(const auto str : text){
            os << str << " ";
        }
        os << "\n";
    }
    std::string_view origin_{};
    std::string_view target_{};
    std::vector<std::string_view> text{};
};

struct NickMessage
{
    friend bool operator==(const NickMessage& lhs, const NickMessage& rhs) {
        return lhs.text == rhs.text && lhs.origin_ == rhs.origin_ && lhs.target_ == rhs.target_;
    }
    void print(std::ostream& os) const {
        os << "Nick Message\n";
        os << "Origin = " << origin_ << "\n";
        os << " Text = ";
        for(const auto str : text){
            os << str << " ";
        }
        os << "\n";
    }
    std::string_view origin_{};
    std::string_view target_{};
    std::vector<std::string_view> text{};
};

struct JoinMessage
{
    constexpr JoinMessage() = default;
    JoinMessage(const std::string_view data) : data_(data) {}

    friend bool operator==(const JoinMessage& lhs, const JoinMessage& rhs) {
        return lhs.data_ == rhs.data_ && lhs.channelName == rhs.channelName && lhs.origin == rhs.origin;
    }
    void print(std::ostream& os) const {
        os << "Join Message\n";
        os << "Data = " << data_ << ".\n";
        os << "Origin = " << origin << ".\n";
        os << "Channel = " << channelName << ".\n";
    }

    std::string_view data_{};
    std::string_view channelName{};
    std::string_view origin{};
};

struct PartMessage
{
    constexpr PartMessage() = default;
    PartMessage(const std::string_view data) : data_(data) {}

    friend bool operator==(const PartMessage& lhs, const PartMessage& rhs) {
        return lhs.data_ == rhs.data_;
    }
    void print(std::ostream& os) const {
        os << "Part Message\n";
        os << "Data = " << data_ << ".\n";
        os << "Origin = " << origin_ << ".\n";
        os << "Channel = " << channelName_ << ".\n";
    }

    std::string_view data_{};
    std::string_view channelName_{};
    std::string_view origin_{};
};

std::string inline execute(const PingMessage& cmd, IIrcClient& client) {
    std::string pingResponse = "PONG " + std::string{cmd.data_} + std::string{"\r\n"};
    assert(pingResponse.find("\r\n") != std::string::npos);
    client.incPingCounter();
    return pingResponse;
}
std::string inline execute(const UserMessage& cmd, IIrcClient& client) {
    std::ostringstream oss;
    std::size_t count = 0;
    for(const auto user : cmd.users_){
        client.addUserToUi(user);
        oss << user;
        if(count++ < cmd.users_.size()-1){
            oss << " ";
        }
    }
    return oss.str() + "\n";
}
std::string  inline execute(const OtherMessage& cmd, IIrcClient& client) {
    std::ostringstream oss;
    oss << cmd.origin_ << " " << cmd.target_ << " ";
    std::size_t count = 0;
    for(const auto str : cmd.text_){
        oss << str;
        if(count++ != cmd.text_.size()-1)
        {
            oss << " ";
        }
    }
    oss << "\n";
    client.getTui().updateChat(oss.str());
    client.incOtherCounter();
    return oss.str();
}
std::string  inline execute(const UnknownMessage& cmd, IIrcClient& client) {
    std::cout << "[DEBUG] Executing Unknown Message cmd\n";
    cmd.print(std::cout);
    client.incUnknownCounter();
    return "";
}
std::string inline execute(const JoinMessage& cmd, IIrcClient& client) {
    std::ostringstream oss;
    oss << cmd.origin << " joined " << cmd.channelName << "\n";
    client.incJoinCounter();
    client.setChannelName(cmd.channelName);
    client.getTui().addChannel(cmd.channelName);
    return oss.str();
}

std::string inline execute(const PartMessage& cmd, IIrcClient& client) {
    client.incPartCounter();
    std::ostringstream oss;
    oss << cmd.origin_ << " left " << cmd.channelName_ << "\n";
    client.setChannelName("");
    client.getTui().removeChannel(cmd.channelName_);
    return oss.str();
}

std::string inline execute(const ModeMessage& cmd, IIrcClient& client) {
    client.incModeCounter();
    std::ostringstream oss;
    oss << cmd.origin_ << " Mode= " << cmd.mode_ << "\n";
    client.getTui().updateChat(oss.str());
    return oss.str();
}

std::string inline execute(const NoticeMessage& cmd, IIrcClient& client) {
    client.incNoticeCounter();
    std::ostringstream oss;
    oss << "Notice from: " << cmd.origin_ << " to " << cmd.target_ << " :";
    for(const auto str : cmd.text){
        oss << str << " ";
    }
    oss << "\n";
    client.getTui().updateChat(oss.str());
    return oss.str();
}

std::string inline execute(const PrivMsgMessage& cmd, IIrcClient& client) {
    client.incPrivMsgCounter();
    std::ostringstream oss;
    oss << cmd.origin_ << "(@" << cmd.target_ << "): ";
    std::size_t count = 0;
    for(const auto str : cmd.text_){
        oss << str;
        if(count++ != cmd.text_.size()-1)
        {
            oss << " ";
        }
    }
    oss << "\n";
    client.getTui().updateChat(oss.str());
    return oss.str();
}

std::string inline execute(const NickMessage& cmd, IIrcClient& client) {
    client.incNickCounter();
    std::ostringstream oss;
    std::ostringstream nickOss;
    oss << cmd.origin_ << " changed nick to ";
    for(const auto str : cmd.text){
        nickOss << str;
    }
    client.setNick(nickOss.str());
    oss << nickOss.str() << "\n";
    client.getTui().updateChat(oss.str());
    return oss.str();
}

struct Message
{
    template<typename T>
    explicit Message(const T& cmd) : impl(std::make_unique<ConcreteMessage<T>>(cmd)) {}

    friend bool operator==(const Message& lhs, const Message& rhs) {
        return *lhs.impl == *rhs.impl;
    }

    friend std::ostream& operator<<(std::ostream& os, const Message& cmd) {
        cmd.print(os);
        return os;
    }

    void print(std::ostream& os) const {
        assert(impl);
        impl->print(os);
    }

    struct MessageConcept
    {
        virtual void print(std::ostream& os) const = 0;
        virtual ~MessageConcept() {};
        virtual bool operator==(const MessageConcept& other) const = 0;
        virtual std::string execute(IIrcClient&) const = 0;
    };

    template<typename T>
    struct ConcreteMessage : MessageConcept
    {
        ConcreteMessage(const T& object) : object_(object) {}
        void print(std::ostream& os) const override {
            object_.print(os);
        }

        bool operator==(const MessageConcept& otherBase) const override {
            const auto* other = dynamic_cast<const ConcreteMessage*>(&otherBase);
            if(!other) {
                return false;
            }
            return *this == *other && *other == *this;
        }
        friend bool operator==(const ConcreteMessage& lhs, const ConcreteMessage& rhs) {
            return lhs.object_ == rhs.object_;
        }

        std::string execute(IIrcClient& client) const override {
            return ::execute(object_, client);
        }

        private:
            T object_;
    };

    friend std::string  execute(const Message& message, IIrcClient& client) {
        return message.impl->execute(client);
    }

    std::unique_ptr<MessageConcept> impl{};
};