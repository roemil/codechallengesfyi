#pragma once

#include "Command.h"

#include "Channel.h"

#include <cstddef>
#include <ostream>
#include <string_view>
#include <optional>
#include <vector>
#include <cassert>

namespace IrcParser {
    
    constexpr std::string_view parseOrigin(const std::string_view data) {
        const auto originPos = data.find_first_of(":");
        assert(originPos != std::string_view::npos);
        return data.substr(originPos+1);
    }

    constexpr Irc::ChannelType parseRplNamReplyType(const std::string_view data) {
        if (data.find(" = ") != std::string_view::npos) {
            return Irc::ChannelType::Public;
        } else if (data.find(" @ ") != std::string_view::npos){
            return Irc::ChannelType::Secret;
        } else if (data.find(" * ") != std::string_view::npos){
            return Irc::ChannelType::Private;
        }
        return Irc::ChannelType::Unknown;
    }

    enum class MessageType {
        Other,
        Join,
        Part,
        Notice,
        Ping,
        Nick,
        Mode,
        PrivMsg,
        Users
    };

    inline constexpr std::string_view toString(const MessageType type) {
        switch (type) {
                case MessageType::Join:
                    return "JOIN";
                case MessageType::Part:
                    return "PART";
                case MessageType::Notice:
                    return "NOTICE";
                case MessageType::Ping:
                    return "PING";
                case MessageType::Nick:
                    return "NICK";
                case MessageType::Mode:
                    return "MODE";
                case MessageType::Other:
                    return "Other";
                case MessageType::PrivMsg:
                    return "PrivMsg";
                case MessageType::Users:
                    return "Users";
            }
        return "";
    }

    struct ParseResult {
        std::string_view origin;
        MessageType message;
        std::optional<std::string_view> target;
        std::vector<std::string_view> text;

        friend bool operator==(const ParseResult& lhs, const ParseResult& rhs) {
            return lhs.origin == rhs.origin && lhs.message == rhs.message
                   && lhs.target == rhs.target && lhs.text == rhs.text;
        }

        friend std::ostream& operator<<(std::ostream& os, const ParseResult& parseResult) {
            os << "Origin = " << parseResult.origin << "\n";
            os << "Message = " << toString(parseResult.message) << "\n";
            os << "target = " << parseResult.target.value_or("") << "\n";
            os << "text = ";
            for(const auto str : parseResult.text){
                os  << str << " ";
            }
            os  << "\n";
            return os;
        }
    };

    inline constexpr MessageType toMessageType(const std::string_view data){
         if(data == "JOIN") {
            return MessageType::Join;
        } else if(data == "PART") {
            return MessageType::Part;
        } else if(data == "NOTICE") {
            return MessageType::Notice;
        } else if(data == "NICK") {
            return MessageType::Nick;
        } else if(data == "PING") {
            return MessageType::Ping;
        } else if(data == "MODE") {
            return MessageType::Mode;
        } else if(data == "PRIVMSG") {
            return MessageType::PrivMsg;
        }else if(data == "353") {
            return MessageType::Users;
        } else {
            return MessageType::Other;
        }
    }

    inline std::vector<std::string_view> tokenize(std::string_view data){
        std::vector<std::string_view> tokens{};
        constexpr char delim = ' ';
        int index = 0;
        for (std::size_t i=0; i < data.size();i++)
        {
            if (data[i] == delim)
            {
                tokens.push_back(data.substr(index, i-index));
                index = i + 1;
            }
        }
        tokens.push_back(data.substr(index));
        return tokens;
    }

    inline std::vector<std::string_view> splitByCrlf(std::string_view data){
        std::vector<std::string_view> tokens{};
        constexpr char delim = '\r';
        int index = 0;
        for (std::size_t i=0; i < data.size()-1;i++)
        {
            if (data[i] == delim && data[i+1] == '\n')
            {
                tokens.push_back(data.substr(index, i+2-index));
                index = i + 2;
            }
        }
        return tokens;
    }

    enum class ParseState {
        Origin,
        Message,
        Target,
        Data,
        Done
    };

    enum class ParseUsers {
        Type,
        ChannelName,
        Users
    };

    inline ParseResult parse(const std::string_view data) {
            const auto isPing = data.find("PING :") != std::string_view::npos;
            ParseState state = ParseState::Origin;
            if(isPing) {
                state = ParseState::Message;
            }
            const auto tokenizedData = tokenize(data);
            ParseResult result{};
            for(const auto token : tokenizedData){
                if(state == ParseState::Target){
                    if(token.starts_with(":")){
                        state = ParseState::Data;
                    }
                }
                switch (state) {
                    case ParseState::Origin:
                    {
                        result.origin = parseOrigin(token);
                        state = ParseState::Message;
                        break;
                    }
                    case ParseState::Message:
                    {
                        result.message = toMessageType(token);
                        state = ParseState::Target;
                        break;
                    }
                    case ParseState::Target:
                    {
                        result.target = token;
                        state = ParseState::Data;
                        break;
                    }
                    case ParseState::Data:
                    {
                        int startIndex = result.text.empty() && token.starts_with(":") ? 1 : 0;
                        result.text.push_back(token.substr(startIndex, token.find("\r\n")-startIndex));
                        break;
                    }
                    case ParseState::Done:
                        break;
                }
            }
            assert(state == ParseState::Data);

            return result;
    };

    namespace {
        std::string_view parseUserName(const std::string_view name){
            if(name.starts_with(":@")){
                return name.substr(2);
            } else if(name.starts_with(":")){
                return name.substr(1);
            } else if(name.starts_with("@")){
                return name.substr(1);
            } else {
                return name;
            }
        }
    }

    inline std::vector<std::string_view> parseUsers(const std::vector<std::string_view>& rawUsers){
        std::vector<std::string_view> users{};
        ParseUsers userState = ParseUsers::Type;
        users.reserve(rawUsers.size()-2); // rawUsers contains channel name and channel type too.
        for(const auto rawUser : rawUsers){
            switch (userState) {
                case ParseUsers::Type:
                {
                    userState = ParseUsers::ChannelName;
                    break;
                }
                case ParseUsers::ChannelName:
                {
                    userState = ParseUsers::Users;
                    break;
                }
                case ParseUsers::Users:
                {
                    users.push_back(parseUserName(rawUser));
                    break;
                }
            }
        }
        return users;
    }

    inline Message createMessage(const ParseResult parseResult, const std::string_view data) {
        using namespace IrcParser;
        switch (parseResult.message) {
            case MessageType::Join:{
                JoinMessage cmd{data};
                cmd.channelName = parseResult.text[0];
                cmd.origin = parseResult.origin;
                return Message{cmd};
            }
            case MessageType::Part:{
                PartMessage cmd{data};
                cmd.channelName_ = parseResult.text[0];
                cmd.origin_ = parseResult.origin;
                return Message{cmd};
            }
            case MessageType::Notice:{
                NoticeMessage cmd{};
                cmd.origin_ = parseResult.origin;
                cmd.target_ = parseResult.target.value_or("");
                cmd.text = parseResult.text;
                return Message{cmd};
            }
            case MessageType::Ping:{
                PingMessage cmd{parseResult.text[0]};
                return Message{cmd};
            }
            case MessageType::Nick:{
                NickMessage cmd{};
                cmd.origin_ = parseResult.origin;
                cmd.text = parseResult.text;
                return Message{cmd};
            }
            case MessageType::Mode:{
                ModeMessage cmd{data};
                cmd.origin_ = parseResult.origin;
                cmd.target_ = parseResult.target.value_or("");
                cmd.mode_ = parseResult.text[0];
                return Message{cmd};
            }
            case MessageType::PrivMsg:{
                PrivMsgMessage cmd{data};
                cmd.origin_ = parseResult.origin;
                cmd.target_ = parseResult.target.value_or("");
                cmd.text_ = parseResult.text;
                return Message{cmd};
            }
            case MessageType::Users:{
                UserMessage msg{};
                msg.origin_ = parseResult.origin;
                msg.target_ = parseResult.target.value_or("");
                msg.users_ = parseUsers(parseResult.text);
                return Message{msg};
            }
            case MessageType::Other:{
                OtherMessage cmd{};
                cmd.origin_ = parseResult.origin;
                cmd.target_ = parseResult.target.value_or("");
                cmd.text_ = parseResult.text;
                return Message{cmd};
            }
        }
        return Message{UnknownMessage{}};
    }
    inline std::string createIrcCommand(const std::string_view data, const std::string_view channel){
        if(data.empty()){
            return "";
        }
        const auto tokens = tokenize(data);

        if(tokens[0][0] == '/' && tokens.size() < 2){
            std::cout << "[ERROR]: Not enough input data";
            return "";
        }

        if(tokens[0] == "/join") {
            return "JOIN " + std::string{tokens[1]} + "\r\n";
        }

        if(tokens[0] == "/part") {
            return "PART " + std::string{tokens[1]} + "\r\n";
        }

        if(tokens[0] == "/nick") {
            return "NICK " + std::string{tokens[1]} + "\r\n";
        }

        return "PRIVMSG " + std::string{channel} + " :" + std::string{data} + "\r\n";
    }
}
