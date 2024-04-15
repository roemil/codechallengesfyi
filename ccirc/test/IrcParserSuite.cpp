#include "IrcParser.h"

#include <gtest/gtest.h>
#include <string_view>
#include <sys/types.h>

TEST(IrcParser, ParsePing) {
    constexpr std::string_view ping = "PING :*.freenode.net\r\n";
    const auto crlf = ping.find("\r\n");
    const auto dataPos = ping.find(":")+1;
    constexpr std::string_view pingdata = ping.substr(dataPos, crlf-dataPos);
    const auto pingMessage = IrcParser::parse(ping);
    EXPECT_EQ(pingMessage.text, std::vector<std::string_view>{pingdata});
}


TEST(IrcParser, ParseOrigin) {
    constexpr std::string_view joinStr = ":CCIRC!~guest@freenode-kge.qup.pic9tt.IP";
    ASSERT_EQ(IrcParser::parseOrigin(joinStr), "CCIRC!~guest@freenode-kge.qup.pic9tt.IP");
}

TEST(IrcParser, ParseRPL_NAMREPLY_Type_Public) {
    constexpr std::string_view data = ":*.freenode.net 353 CCIRC = #cc :@CCIRC\r\n";
    ASSERT_EQ(IrcParser::parseRplNamReplyType(data), Irc::ChannelType::Public);
}

TEST(IrcParser, ParseRPL_NAMREPLY_Type_Private) {
    constexpr std::string_view data = ":*.freenode.net 353 CCIRC * #cc :@CCIRC\r\n";
    ASSERT_EQ(IrcParser::parseRplNamReplyType(data), Irc::ChannelType::Private);
}

TEST(IrcParser, ParseRPL_NAMREPLY_Type_Secret) {
    constexpr std::string_view data = ":*.freenode.net 353 CCIRC @ #cc :@CCIRC\r\n";
    ASSERT_EQ(IrcParser::parseRplNamReplyType(data), Irc::ChannelType::Secret);
}

TEST(IrcParser, ParseRPL_NAMREPLY_Type_Unknown) {
    constexpr std::string_view data = ":*.freenode.net 353 CCIRC A #cc :@CCIRC\r\n";
    ASSERT_EQ(IrcParser::parseRplNamReplyType(data), Irc::ChannelType::Unknown);
}

TEST(IrcParser, Parse_NOTICE) {
    constexpr std::string_view data = ":*.freenode.net NOTICE CCIRC :*** Ident lookup timed out, using ~guest instead.\r\n";
    IrcParser::ParseResult result{};
    result.message = IrcParser::MessageType::Notice;
    result.origin = "*.freenode.net";
    result.text = {"***", "Ident", "lookup", "timed", "out,", "using", "~guest", "instead."};
    result.target = "CCIRC";
    ASSERT_EQ(IrcParser::parse(data), result);
}

TEST(IrcParser, Parse_PART) {
    constexpr std::string_view data = ":CCIRC!~guest@freenode-kge.qup.pic9tt.IP PART :#cc\r\n";
    IrcParser::ParseResult result{};
    result.message = IrcParser::MessageType::Part;
    result.origin = "CCIRC!~guest@freenode-kge.qup.pic9tt.IP";
    result.text = {"#cc"};
    ASSERT_EQ(IrcParser::parse(data), result);
}

TEST(IrcParser, Parse_NICK) {
    constexpr std::string_view data = ":Guest4454!~guest@freenode-kge.qup.pic9tt.IP NICK :JohnC\r\n";
    IrcParser::ParseResult result{};
    result.message = IrcParser::MessageType::Nick;
    result.origin = "Guest4454!~guest@freenode-kge.qup.pic9tt.IP";
    result.text = {"JohnC"};
    ASSERT_EQ(IrcParser::parse(data), result);
}

TEST(IrcParser, ParseTarget) {
    constexpr std::string_view data = ":CCIRC!~guest@freenode-kge.qup.pic9tt.IP MODE CCIRC :+wRix\r\n";
    IrcParser::ParseResult result{};
    result.message = IrcParser::MessageType::Mode;
    result.origin = "CCIRC!~guest@freenode-kge.qup.pic9tt.IP";
    result.target = {"CCIRC"};
    result.text = {"+wRix"};
    ASSERT_EQ(IrcParser::parse(data), result);
}

TEST(IrcParser, ParseOtherMessage) {
    constexpr std::string_view data = ":*.freenode.net 001 CCClient :Welcome to the freenode IRC Network CCClient!~guest@62-220-160-242.cust.bredband2.com\r\n";
    IrcParser::ParseResult result{};
    result.message = IrcParser::MessageType::Other;
    result.origin = "*.freenode.net";
    result.target = {"CCClient"};
    result.text = {"Welcome", "to", "the", "freenode", "IRC", "Network", "CCClient!~guest@62-220-160-242.cust.bredband2.com"};
    ASSERT_EQ(IrcParser::parse(data), result);
}

TEST(IrcParser, UnknownMessagePrivMsgNotEnoughParameters) {
    constexpr std::string_view privmsg = ":*.freenode.net 461 CCClient PRIVMSG :Not enough parameters\r\n";
    IrcParser::ParseResult result{};
    result.message = IrcParser::MessageType::Other;
    result.origin = "*.freenode.net";
    result.target = {"CCClient"};
    result.text = { "PRIVMSG", ":Not", "enough", "parameters"};
    ASSERT_EQ(IrcParser::parse(privmsg), result);
}

TEST(IrcParser, PrivMsg) {
    constexpr std::string_view privmsg = ":web-78!~web-78@freenode-2i9.pb9.rfqgc3.IP PRIVMSG #cc :general kenobi\r\n";
    IrcParser::ParseResult result{};
    result.message = IrcParser::MessageType::PrivMsg;
    result.origin = "web-78!~web-78@freenode-2i9.pb9.rfqgc3.IP";
    result.target = {"#cc"};
    result.text = { "general", "kenobi"};
    ASSERT_EQ(IrcParser::parse(privmsg), result);
}

TEST(IrcParser, Tokenize) {
    constexpr std::string_view data = ":CCIRC!~guest@freenode-kge.qup.pic9tt.IP MODE CCIRC :+wRix\r\n";
    const std::vector<std::string_view> tokens{":CCIRC!~guest@freenode-kge.qup.pic9tt.IP", "MODE", "CCIRC", ":+wRix\r\n"};
    ASSERT_EQ(tokens, IrcParser::tokenize(data));
}

TEST(IrcParser, SplitByLine) {
    constexpr std::string_view data = ":*.freenode.net NOTICE CCClient :*** Could not find your ident, using ~guest instead.\r\n"
                                      ":*.freenode.net 001 CCClient :Welcome to the freenode IRC Network CCClient!~guest@62-220-160-242.cust.bredband2.com\r\n";
    const std::vector<std::string_view> tokens{":*.freenode.net NOTICE CCClient :*** Could not find your ident, using ~guest instead.\r\n",
                                               ":*.freenode.net 001 CCClient :Welcome to the freenode IRC Network CCClient!~guest@62-220-160-242.cust.bredband2.com\r\n"};
    ASSERT_EQ(tokens, IrcParser::splitByCrlf(data));
}

TEST(IrcParser, InputJoin) {
    ASSERT_EQ("JOIN #cc\r\n", IrcParser::createIrcCommand("/join #cc", ""));
}

TEST(IrcParser, InputPart) {
    ASSERT_EQ("PART #cc\r\n", IrcParser::createIrcCommand("/part #cc", ""));
}

TEST(IrcParser, InputNick) {
    ASSERT_EQ("NICK NewNickname\r\n", IrcParser::createIrcCommand("/nick NewNickname", ""));
}

TEST(IrcParser, PrivMsgChannel) {
    ASSERT_EQ("PRIVMSG #cc :this is some text to send\r\n", IrcParser::createIrcCommand("this is some text to send", "#cc"));
}

TEST(IrcParser, OneUser) {
    constexpr std::string_view users = ":irc.example.com 353 dan = #test :@dan\r\n";
    IrcParser::ParseResult result{};
    result.message = IrcParser::MessageType::Users;
    result.origin = "irc.example.com";
    result.target = {"dan"};
    result.text = {  "=", "#test", ":@dan"};
    ASSERT_EQ(IrcParser::parse(users), result);
}

TEST(IrcParser, MultipleUsers) {
    constexpr std::string_view users = ":irc.example.com 353 alice @ #test :alice @dan\r\n";
    IrcParser::ParseResult result{};
    result.message = IrcParser::MessageType::Users;
    result.origin = "irc.example.com";
    result.target = {"alice"};
    result.text = { "@", "#test", ":alice", "@dan"};
    ASSERT_EQ(IrcParser::parse(users), result);
}