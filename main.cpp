#include <iostream>
#include <string>
#include <cpr/cpr.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include "client_wss.hpp"

std::string host = "api.harmony-chat.cf";
std::string port = "443";
std::string domain = "https://" + host + ":" + port;
std::string group;
std::string chat;
std::string groupName;
std::string chatName;

void websocketFun(std::string wsUrl, std::string token) {
    using WssClient = SimpleWeb::SocketClient<SimpleWeb::WSS>;
    WssClient client(wsUrl, false);
    client.on_message = [](std::shared_ptr<WssClient::Connection> connection, std::shared_ptr<WssClient::InMessage> in_message) {
        std::cout << in_message->string();
        connection->send("{}");
    };

    client.on_open = [](std::shared_ptr<WssClient::Connection> connection) {
        connection->send("{\"protocol\":\"json\",\"version\":1}\30");
    };

    client.start();
}

void liner(std::string token) {
    for (std::string line; std::getline(std::cin, line);) {
        if(line.rfind(".group", 0) == 0) {
            std::string cleanLineWithoutSpaces = line;
            cleanLineWithoutSpaces.erase(remove_if(cleanLineWithoutSpaces.begin(), cleanLineWithoutSpaces.end(), isspace), cleanLineWithoutSpaces.end());
            if(cleanLineWithoutSpaces.substr(6).empty()) {
                std::cout << "Groups:" << std::endl;
                cpr::Response gettingGroups = cpr::Get(cpr::Url{domain + "/user"},
                                                       cpr::Header{{"Authorization", token}},
                                                       cpr::VerifySsl{false});
                auto gettingGroupsJson = nlohmann::json::parse(gettingGroups.text);
                for (std::string groupId : gettingGroupsJson["groups"]) {
                    cpr::Response gettingGroup = cpr::Get(cpr::Url{domain + "/group/" + groupId},
                                                          cpr::Header{{"Authorization", token}},
                                                          cpr::VerifySsl{false});
                    auto gettingGroupJson = nlohmann::json::parse(gettingGroup.text);
                    std::cout << ((group == groupId ) ? "> " : "") << gettingGroupJson["name"].get<std::string>() << ": " << groupId << std::endl;
                }
            } else {
                std::string groupId = line.substr(7);
                cpr::Response gettingGroup = cpr::Get(cpr::Url{domain + "/group/" + groupId},
                                                      cpr::Header{{"Authorization", token}},
                                                      cpr::VerifySsl{false});
                auto gettingGroupJson = nlohmann::json::parse(gettingGroup.text);
                if(gettingGroupJson["success"]) {
                    group = groupId;
                    groupName = gettingGroupJson["name"];
                    std::cout << "You are now in " << gettingGroupJson["name"].get<std::string>() << "." << std::endl;
                } else {
                    std::cout << "Invalid group." << std::endl;
                }
            }
        } else if(line.rfind(".chat", 0) == 0) {
            std::string cleanLineWithoutSpaces = line;
            cleanLineWithoutSpaces.erase(remove_if(cleanLineWithoutSpaces.begin(), cleanLineWithoutSpaces.end(), isspace), cleanLineWithoutSpaces.end());
            if(cleanLineWithoutSpaces.substr(5).empty()) {
                cpr::Response gettingGroup = cpr::Get(cpr::Url{domain + "/group/" + group},
                                                      cpr::Header{{"Authorization", token}},
                                                      cpr::VerifySsl{false});
                auto gettingGroupJson = nlohmann::json::parse(gettingGroup.text);
                if(gettingGroupJson["success"]) {
                    for (nlohmann::json chatObject : gettingGroupJson["chats"]) {
                        std::cout << ((chat == chatObject["id"].get<std::string>() ) ? "> " : "") << chatObject["name"].get<std::string>() << ": " << chatObject["id"].get<std::string>() << std::endl;
                    }
                } else {
                    std::cout << "Invalid group." << std::endl;
                }
            } else {
                std::string chatId = line.substr(6);
                cpr::Response gettingChat = cpr::Get(cpr::Url{domain + "/group/" + group + "/chat/" + chatId},
                                                     cpr::Header{{"Authorization", token}},
                                                     cpr::VerifySsl{false});
                auto gettingChatJson = nlohmann::json::parse(gettingChat.text);
                if(gettingChatJson["success"]) {
                    chat = chatId;
                    chatName = gettingChatJson["name"];
                    std::cout << "You are now in " << gettingChatJson["name"].get<std::string>() << "." << std::endl;
                    cpr::Response gettingMessages = cpr::Get(cpr::Url{domain + "/group/" + group + "/chat/" + chatId + "/message"},
                                                             cpr::Header{{"Authorization", token}},
                                                             cpr::VerifySsl{false});
                    auto gettingMessagesJson = nlohmann::json::parse(gettingMessages.text);
                    if(gettingMessagesJson["success"]) {
                        std::vector<nlohmann::json> messages = gettingMessagesJson["messages"];
                        nlohmann::json author;
                        for(nlohmann::json message : messages) {
                            if(!author.contains(message["author"].get<std::string>())) {
                                cpr::Response gettingUser = cpr::Get(cpr::Url{domain + "/user/" + message["author"].get<std::string>()},
                                                                     cpr::Header{{"Authorization", token}},
                                                                     cpr::VerifySsl{false});
                                auto gettingUserJson = nlohmann::json::parse(gettingUser.text);
                                author[message["author"].get<std::string>()] = gettingUserJson["username"].get<std::string>() + "#" + gettingUserJson["usernumber"].get<std::string>();
                            }
                            std::cout << author[message["author"].get<std::string>()].get<std::string>() << ": " << message["content"].get<std::string>() << std::endl;
                        }
                    }
                } else {
                    std::cout << "Invalid chat." << std::endl;
                }
            }
        }  else if(line.rfind(".create", 0) == 0) {
            if(line.rfind("group", 8) == 8) {
                std::string groupNameC = line.substr(14);
                cpr::Response creatingGroup = cpr::Post(cpr::Url{domain + "/group"},
                                                        cpr::Header{{"Authorization", token},
                                                                    {"Name", groupNameC}},
                                                                    cpr::VerifySsl{false});
                auto creatingGroupJson = nlohmann::json::parse(creatingGroup.text);
                if(creatingGroupJson["success"]) {
                    std::cout << "Group created." << std::endl;
                } else {
                    std::cout << "Something went wrong." << std::endl;
                }
            } else if(line.rfind("chat", 8) == 8) {
                if(!group.empty()) {
                    std::string chatNameC = line.substr(13);
                    cpr::Response creatingChat = cpr::Post(cpr::Url{domain + "/group/" + group + "/chat"},
                                                           cpr::Header{{"Authorization", token},
                                                                       {"Name", chatNameC}},
                                                                       cpr::VerifySsl{false});
                    auto creatingChatJson = nlohmann::json::parse(creatingChat.text);
                    if(creatingChatJson["success"]) {
                        std::cout << "Chat created." << std::endl;
                    } else {
                        std::cout << "Something went wrong." << std::endl;
                    }
                } else {
                    std::cout << "Get into a group using .group." << std::endl;
                }
            } else if(line.rfind("invite", 8) == 8) {
                if(!group.empty()) {
                    cpr::Response creatingInvite = cpr::Post(cpr::Url{domain + "/group/" + group + "/invite"},
                                                             cpr::Header{{"Authorization", token}},
                                                             cpr::VerifySsl{false});
                    auto creatingInviteJson = nlohmann::json::parse(creatingInvite.text);
                    if(creatingInviteJson["success"]) {
                        std::cout << "Invite created: " << creatingInviteJson["code"].get<std::string>() << "." << std::endl;
                    } else {
                        std::cout << "Something went wrong." << std::endl;
                    }
                }
            } else {
                std::cout << "What do you want to create? group/chat/invite are accepted." << std::endl;
            }
        } else if(line.rfind(".join", 0) == 0) {
            std::string inviteCode = line.substr(6);
            cpr::Response joiningGroup = cpr::Post(cpr::Url{domain + "/invite/join/" + inviteCode},
                                                   cpr::Header{{"Authorization", token}},
                                                   cpr::VerifySsl{false});
            auto joiningGroupJson = nlohmann::json::parse(joiningGroup.text);
            if(joiningGroupJson["success"]) {
                std::cout << "Joined group " << joiningGroupJson["name"] << "." << std::endl;
            } else {
                std::cout << "Something went wrong." << std::endl;
            }
        } else if(line.rfind(".leave", 0) == 0) {
            if(!group.empty()) {
                cpr::Response leavingGroup = cpr::Post(cpr::Url{domain + "/group/" + group + "/leave" },
                                                       cpr::Header{{"Authorization", token}},
                                                       cpr::VerifySsl{false});
                auto leavingGroupJson = nlohmann::json::parse(leavingGroup.text);
                if(leavingGroupJson["success"]) {
                    std::cout << "Group left." << std::endl;
                } else {
                    std::cout << "Something went wrong." << std::endl;
                }
            } else {
                std::cout << "Get into a group using .group." << std::endl;
            }
        } else if(!group.empty() && !chat.empty()) {
            cpr::Post(cpr::Url{domain + "/group/" + group + "/chat/" + chat + "/message"},
                      cpr::Header{{"Authorization", token},
                                  {"Message", line}},
                                  cpr::VerifySsl{false});
        } else {
            std::cout << "Get into a chat using .group and .chat." << std::endl;
        }
        std::cout << groupName << "/" << chatName << ">";
    }
}

int main() {
    std::string token;
    std::ifstream preToken("Token");
    if (preToken.good()) {
        getline(preToken, token);
        cpr::Response initialGettingUser = cpr::Get(cpr::Url{domain + "/user"},
                                                     cpr::Header{{"Authorization", token}},
                                                     cpr::VerifySsl{false});
        auto initialGettingUserJson = nlohmann::json::parse(initialGettingUser.text);

        if(initialGettingUserJson["success"]) {
            std::cout << "Welcome to Harmony!" << std::endl;
            std::cout << groupName << "/" << chatName << ">";
            CURL *curl = curl_easy_init();
            std::string wsUrl = host + ":" + port + "/signalr?access_token=" + curl_easy_escape(curl, token.c_str(), 0);
            std::thread thread(websocketFun, wsUrl, token);
            std::thread thread2(liner, token);
            thread.join();
            thread2.join();
        } else {
            std::cout << "Invalid token.";
            return 1;
        }
    } else {
        std::cout << "Token missing.";
        return 1;
    }
}
