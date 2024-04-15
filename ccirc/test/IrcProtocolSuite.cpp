#include "IIrcClient.h"
#include "IrcHandler.h"
#include "Tui.h"

#include <sstream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

TEST(IrcProtocol, print) {
    const std::string ping = "PING :nwRP?cRJ[^\r\n";
    const std::string pingdata = ping.substr(ping.find(":")+1);
    const Message actualMessage{PingMessage{pingdata}};
    std::ostringstream expected;
    expected << "Ping Message\n";
    expected << "Data = " << pingdata << "\n";

    std::ostringstream actual;
    actual << actualMessage;
    EXPECT_EQ(expected.str(), expected.str());
}

TEST(IrcProtocol, CreatePingMessage) {
    constexpr std::string_view ping = "PING :*.freenode.net\r\n";
    const auto crlf = ping.find("\r\n");
    const auto dataPos = ping.find(":")+1;
    constexpr std::string_view pingdata = ping.substr(dataPos, crlf-dataPos);
    const Message pingMessage = IrcHandler::parse(ping);
    const Message expected{PingMessage{pingdata}};
    EXPECT_EQ(pingMessage, expected);
}

TEST(IrcProtocol, CreateJoinMessage) {
    constexpr std::string_view join = ":CCIRC!~guest@freenode-kge.qup.pic9tt.IP JOIN :#cc\r\n";
    const Message joinMessage = IrcHandler::parse(join);
    JoinMessage expectedJoin{};
    expectedJoin.origin = "CCIRC!~guest@freenode-kge.qup.pic9tt.IP";
    expectedJoin.channelName = "#cc";
    expectedJoin.data_ = join;
    const Message expected{expectedJoin};
    EXPECT_EQ(joinMessage, expected);
}

TEST(IrcProtocol, CreateModeMessage) {
    constexpr std::string_view mode = ":CCIRC!~guest@freenode-kge.qup.pic9tt.IP MODE CCIRC :+wRix\r\n";
    ModeMessage expectedMode{};
    expectedMode.origin_ = "CCIRC!~guest@freenode-kge.qup.pic9tt.IP";
    expectedMode.mode_ = "+wRix";
    expectedMode.target_ = "CCIRC";
    expectedMode.data_ = mode;
    const Message expected{expectedMode};
    const Message modeMessage = IrcHandler::parse(mode);
    EXPECT_EQ(modeMessage, expected);
}

TEST(IrcProtocol, CreateNoticeMessage) {
    constexpr std::string_view notice = ":*.freenode.net NOTICE CCIRC :*** Ident lookup timed out, using ~guest instead.\r\n";
    NoticeMessage expectedNotice{};
    expectedNotice.origin_ = "*.freenode.net";
    expectedNotice.target_ = "CCIRC";
    expectedNotice.text = {"***", "Ident", "lookup", "timed", "out,", "using", "~guest", "instead."};
    const Message expected{expectedNotice};
    const Message modeMessage = IrcHandler::parse(notice);
    EXPECT_EQ(modeMessage, expected);
}

TEST(IrcProtocol, CreateNickMessage) {
    constexpr std::string_view nick = ":Guest4454!~guest@freenode-kge.qup.pic9tt.IP NICK :JohnC\r\n";
    NickMessage expectedNick{};
    expectedNick.origin_ = "Guest4454!~guest@freenode-kge.qup.pic9tt.IP";
    expectedNick.text = {"JohnC"};
    const Message expected{expectedNick};
    const Message nickMessage = IrcHandler::parse(nick);
    EXPECT_EQ(nickMessage, expected);
}

TEST(IrcProtocol, CreatePartMessage) {
    constexpr std::string_view part = ":CCIRC!~guest@freenode-kge.qup.pic9tt.IP PART :#cc\r\n";
    PartMessage expectedPart{part};
    expectedPart.origin_ = "Guest4454!~guest@freenode-kge.qup.pic9tt.IP";
    expectedPart.channelName_ = {"#cc"};
    const Message expected{expectedPart};
    const Message partMessage = IrcHandler::parse(part);
    EXPECT_EQ(partMessage, expected);
}

class IrcClientMock : public IIrcClient {
 public:
    ~IrcClientMock() override = default;
    MOCK_METHOD(void, incPingCounter, (), (override));
    MOCK_METHOD(void, incUnknownCounter, (), (override));
    MOCK_METHOD(void, incJoinCounter, (), (override));
    MOCK_METHOD(void, incPartCounter, (), (override));
    MOCK_METHOD(void, incOtherCounter, (), (override));
    MOCK_METHOD(void, incModeCounter, (), (override));
    MOCK_METHOD(void, incNoticeCounter, (), (override));
    MOCK_METHOD(void, incNickCounter, (), (override));
    MOCK_METHOD(void, incPrivMsgCounter, (), (override));
    MOCK_METHOD(void, setNick, (const std::string_view), (override));
    MOCK_METHOD(void, setChannelName, (const std::string_view), (override));
    MOCK_METHOD(void, uiCmd, (const std::string_view), (override));
    MOCK_METHOD(void, addUserToUi, (const std::string_view), (override));

    Tui& getTui() override{
        return tui_;
    }
    Tui tui_{nullptr};
};

TEST(IrcProtocol, ExecutePingMessage) {
    constexpr std::string_view ping = "PING :nwRP?cRJ[^\r\n";
    const auto pingMessage = IrcHandler::parse(ping);
    IrcClientMock client{};
    EXPECT_CALL(client, incPingCounter()).Times(1);
    const auto pingReply = execute(pingMessage, client);
    constexpr std::string_view pong = "PONG nwRP?cRJ[^\r\n";
    ASSERT_EQ(pingReply, pong);
}

TEST(IrcProtocol, ExecuteJoinMessage) {
    constexpr std::string_view join = ":CCIRC!~guest@freenode-kge.qup.pic9tt.IP JOIN :#cc\r\n";
    const auto cmd = IrcHandler::parse(join);
    IrcClientMock client{};
    EXPECT_CALL(client, incJoinCounter()).Times(1);
    EXPECT_CALL(client, setChannelName("#cc")).Times(1);
    const auto joinResult = execute(cmd, client);
    constexpr std::string_view expectedJoinResult = "CCIRC!~guest@freenode-kge.qup.pic9tt.IP joined #cc\n";
    ASSERT_EQ(joinResult, expectedJoinResult);
}

TEST(IrcProtocol, UnknownMessagePrivMsg) {
    constexpr std::string_view privmsg = ":*.freenode.net 461 CCClient PRIVMSG :Not enough parameters\r\n";
    const auto privMsgcmd = IrcHandler::parse(privmsg);
    IrcClientMock client{};
    const auto result = execute(privMsgcmd, client);
    ASSERT_EQ(result, "*.freenode.net CCClient PRIVMSG :Not enough parameters\n");
}

TEST(IrcProtocol, PrivMsg) {
    constexpr std::string_view privmsg = ":web-78!~web-78@freenode-2i9.pb9.rfqgc3.IP PRIVMSG #cc :general kenobi\r\n";
    const auto privMsgcmd = IrcHandler::parse(privmsg);
    IrcClientMock client{};
    const auto result = execute(privMsgcmd, client);
    ASSERT_EQ(result, "web-78!~web-78@freenode-2i9.pb9.rfqgc3.IP(@#cc): general kenobi\n");
}

TEST(IrcProtocol, OneUser) {
    constexpr std::string_view users = ":irc.example.com 353 dan = #test :@dan\r\n";
    const auto userMsg = IrcHandler::parse(users);
    IrcClientMock client{};
    EXPECT_CALL(client, addUserToUi("dan")).Times(1);
    const auto result = execute(userMsg, client);
    ASSERT_EQ(result, "dan\n");
}

TEST(IrcProtocol, MultipleUser) {
    constexpr std::string_view users = ":irc.example.com 353 alice @ #test :alice @dan\r\n";
    const auto userMsg = IrcHandler::parse(users);
    IrcClientMock client{};
    EXPECT_CALL(client, addUserToUi("alice")).Times(1);
    EXPECT_CALL(client, addUserToUi("dan")).Times(1);
    const auto result = execute(userMsg, client);
    ASSERT_EQ(result, "alice dan\n");
}


