#include "headers/user.h"
#include "headers/sha1.h"
#include <algorithm>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>


bool authenticateUser(const std::string &login, uint* hash,
                      const std::vector<User> &users)
{
    return std::any_of(users.begin(), users.end(),
        [&login, &hash](const User& user) {
            return user.getLogin() == login && *(user.getHash()) == *hash;
        });
}


User::User(const std::string &login, uint *hash, const std::string &name,
           const std::string &email):
           _login(login), _hash(hash), _name(name), _email(email) {}

User::User():
    _login("all"),
    _hash(0),
    _name("All"),
    _email("default@email.none") {}


const std::string& User::getName() const {
    return _name;
}


const std::string& User::getLogin() const {
    return _login;
}


const std::string& User::getEmail() const {
    return _email;
}


uint* User::getHash() const {
    return _hash;
}


bool User::changePassword(uint *hash, uint *new_hash) {
    if (*_hash == *hash) {
        _hash = new_hash;
        return true;
    }
    return false;
}


void User::userInfo() const {
    cout << "User login: " << _login << endl;
    cout << "User name: " << _name << endl;
    cout << "User email: " << _email << endl;
}


fstream& operator >> (fstream& is, User& obj)
{
    uint hash;
	is >> obj._login;
	is >> hash;
    is >> obj._name;
    *obj._hash = hash;
	return is;
}


ostream& operator << (ostream& os, const User& obj)
{
	os << obj._login;
	os << ' ';
	os << *(obj._hash);
    os << ' ';
    os << obj._name;
	return os;
}


bool User :: operator == (const User& u) const {
        return (_login == u.getLogin()) && (_name == u.getName()) &&
               (_email == u.getEmail()) && (_hash == u.getHash());
}
