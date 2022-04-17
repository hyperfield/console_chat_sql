#pragma once

#include <string>
#include <vector>
#include "user.h"
#include "message.h"



class Message {
public:
    Message(const std::string& message, const std::string& login, const std::string& forUser);
    Message() = default;
    ~Message() = default;
    const std::string getMessage() const;
    // const string getMessage(const string &login, uint* hash, const vector<User> &users) const;
    // const string getAuthor(const string &login, uint* hash, const vector<User> &users) const;
    const std::string getAuthor() const;
    const std::string& forWhom() const;
    void messageInfo(const std::string &login, uint* hash, const std::vector<User> &users) const;
    void messageInfo() const;
    friend std::fstream& operator >>(std::fstream& is, Message& obj);
    friend std::ostream& operator <<(std::ostream& os, const Message& obj);

private:
    std::string _author;
    std::string _forUser;
    std::string _message;
};
