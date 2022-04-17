#include "headers/message.h"
#include "headers/user.h"
#include "headers/sha1.h"
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

using namespace std;

Message::Message(const std::string& message, const std::string& login, const std::string& forUser):
    _message(message), _author(login), _forUser(forUser) {}


// const string Message::getMessage(const string &login, uint* hash, const vector<User> &users) const {
//     if (_forUser == "all") return _message;
//     if (authenticateUser(login, hash, users)) return _message;
//     return "Authentication error";
// }


const std::string Message::getAuthor() const {
    return _author;
}


const string Message::getMessage() const {
    return _message;
}


const string& Message::forWhom() const {
    return _forUser;
}


void Message::messageInfo(const string &login, uint* hash, const vector<User> &users) const {
    bool authenticated = false;
    if (_forUser == "all") authenticated = true;
    authenticated = authenticateUser(login, hash, users);
    if (authenticated) {
        cout << "Author: " << _author << endl;
        cout << "For: " << _forUser << endl;
        cout << "Message:\n" << _message << endl;
    }
}


void Message::messageInfo() const {
    if (_forUser == "all") {
        cout << "Author: " << _author << endl;
        cout << "For: " << _forUser << endl;
        cout << "Message:\n" << _message << endl;
    }
    else cout << "\nAuthentication is required";
}


fstream& operator >>(fstream& is, Message& obj) {
    string line;
    std::getline(is, line);
    stringstream ss(line);
    ss >> obj._author;
    ss >> obj._forUser;
    string word;
    while (ss >> word) {
        obj._message.append(word + " ");
    }
	return is;
}


ostream& operator <<(ostream& os, const Message& obj) {
	os << obj._author;
	os << ' ';
	os << obj._forUser;
    os << ' ';
    os << obj._message;
	return os;
}
