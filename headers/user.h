// TODO: operator= for classes User, Message

#pragma once

#include <string>
#include <vector>


class User {
public:
    User(const std::string &login, uint *hash, const std::string &name, const std::string &email);
    User();
    ~User() = default;
    void userInfo() const;
    const std::string& getName() const;
    const std::string& getLogin() const;
    const std::string& getEmail() const;
    uint* getHash() const;
    const bool changePassword(uint* hash, uint* newHash);
    friend std::fstream& operator >> (std::fstream& is, User& obj);
    friend std::ostream& operator << (std::ostream& os, const User& obj);
    bool operator == (const User& u) const;

private:
    std::string _login;
    uint* _hash;
    std::string _name;
    std::string _email;
};


const bool authenticateUser(const std::string &login, uint* hash, const std::vector<User> &users);
