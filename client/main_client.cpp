/*
    - Save changed user passwords upon exit from server part.
    - Remove loops from bytes / writes. Re-establish connection if bytes < 0.
    - Rename passwordFlag to passwordChanged;
    - Analyze: string newPassword = setPassword(passwordFlag);
    - Save chat server IP address in a config file.
    - Implement addition of new users via chat client.
    - Think if additional authentication is needed for private messages methods (getAuthor() and getMessage()).
*/

#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "../headers/user.h"
#include "../headers/message.h"
#include "../headers/passwordShadow.h"
#include "../headers/sha1.h"


using namespace std;
namespace fs = std::filesystem;

#define MESSAGE_LENGTH 1024
#define PORT 7777


// White an object to file
template <typename T>
const bool writeObjToFile(const T obj, const string filepath) {
    fstream obj_file = fstream(filepath, ios::in | ios::out);
    if (!obj_file) {
        // Using the parameter ios::trunc to create the file
        obj_file = fstream(filepath, ios::in | ios::out | ios::trunc);
    }

    if (obj_file) {
        obj_file.seekg(0, std::ios::end);
        obj_file << obj << endl;
        obj_file.close();
        return true;
    }
    else
        return false;
}


// Read objects from file
template <typename T>
const bool readObjsFromFile(const string filepath, vector<T> &objs) {
    fstream objs_file = fstream(filepath, ios::in | ios::out);
    if (!objs_file) {
		// Using the parameter ios::trunc to create the file
        objs_file = fstream(filepath, ios::in | ios::out | ios::trunc);
    }
    if (objs_file) {
        T* obj = new T;
        while (objs_file >> *obj) {
            objs.insert(objs.end(), *obj);
            delete obj;
            obj = new T;
        }
        objs_file.close();
        return true;
    }
    else {
		cout << "Could not open or create the file " << filepath << "!\n";
		return false;
	}
}


const bool transmit_message(Message message, int socket_file_descriptor) {
    bool message_accepted;
    char command[] = "transmit_message";
    ssize_t bytes = write(socket_file_descriptor, command, sizeof(command));
    if (bytes >= 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        bytes = -1;
        while (bytes < 0) {
            bytes = write(socket_file_descriptor, message.getMessage().c_str(), 1024);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        bytes = -1;
        while (bytes < 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            bytes = write(socket_file_descriptor, message.getAuthor().c_str(), 32);
        }
        bytes = -1;
        while (bytes < 0) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            bytes = write(socket_file_descriptor, message.forWhom().c_str(), 32);
        }
        read(socket_file_descriptor, (char*) &message_accepted, sizeof(bool));
    }
    return message_accepted;
}


// Create a new message and place it in a vector.
void newMessage(const string &message, const string &author, const string &for_user, vector<Message> &messages, const string filepath) {
    Message newMessage(message, author, for_user);
    messages.insert(messages.end(), newMessage);

    if (!writeObjToFile(newMessage, filepath)) {
        cout << "Could not open file " << filepath << "!\n";
        cout << "Chat messages will NOT be saved.\n";
    }
}


// Check if a user with this login already exists.
const bool userExists(string& login, int socket_file_descriptor) {
    char command[] = "checkUserExists";
    ssize_t bytes = write(socket_file_descriptor, command, sizeof(command));
    bool login_exists;
    if (bytes >= 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        bytes = write(socket_file_descriptor, login.c_str(), 32);
        read(socket_file_descriptor, (char*) &login_exists, sizeof(bool));
    }
    return login_exists;
}


// Check if the user password is correct.
const bool checkPassword(const string& login, const string &password, uint* &hash, int socket_file_descriptor) {
    hash = sha1(password.c_str(), password.length());
    char command[] = "checkUserPassword";
    ssize_t bytes = write(socket_file_descriptor, command, sizeof(command));
    bool password_correct = false;
    if (bytes >= 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        bytes = -1;
        while (bytes < 0) {
            bytes = write(socket_file_descriptor, (char*) hash, 32);
        }
        read(socket_file_descriptor, (char*) &password_correct, sizeof(bool));
    }
    return password_correct;
}


// Type and confirm a password when a new user is created.
const string setPassword(bool &passwordFlag) {
    int counter = 0;
    while (counter++ < 2) {
        string password, confirmPassword;
        cout << "Please enter new user password: ";
        SetStdinEcho(false);
        cin >> password;
        SetStdinEcho(true);
        cout << endl;
        cout << "Confirm new user password: ";
        SetStdinEcho(false);
        cin >> confirmPassword;
        SetStdinEcho(true);
        if (password == confirmPassword) {
            passwordFlag = true;
            return password;
        }
        cout << endl;
    }
    cout << "\nPassword mismatch\n";
    return "Password mismatch";
}


// Show common and private chat messages
void showMessages(const vector<Message> &messages, const string &login, const string &all) {
        for (auto &message : messages) { // Get all common and private messages
            if (message.forWhom() == all) {
                cout << message.getAuthor() << " => " << message.forWhom() << ": " << message.getMessage() << endl;
                continue;
            }
        if (message.forWhom() != login) continue;
        cout << message.getAuthor() << " => you" << ": " << message.getMessage() << endl;
    }
}


bool establish_connection(int& socket_file_descriptor, const char* connection_ip, int& connection) {
    struct sockaddr_in serveraddress, client;
    socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0); // Create a socket
    if(socket_file_descriptor == -1) {
        return false;
    }
    // Define server address
    serveraddress.sin_addr.s_addr = inet_addr(connection_ip);
    // Define port number
    serveraddress.sin_port = htons(PORT);
    // Using IPv4
    serveraddress.sin_family = AF_INET;
    // Establish a connection with the server
    connection = connect(socket_file_descriptor, (struct sockaddr*)&serveraddress, sizeof(serveraddress));
    if(connection == -1){
        return false;
    }
    return true;
}


bool connection_wrapper(int& socket_file_descriptor, char connection_ip[], int connection) {
    bool connection_success = establish_connection(socket_file_descriptor, connection_ip, connection);
    char key;
    while (!connection_success) {
        cout << "\nPlease choose:\n\ni - Specify a Console Chat server\nq - Quit" << endl;
        cin >> key;

        switch (key) {
        case 'i':
            cout << "Console chat server hostname (or IP address) (e.g. 127.0.0.1 or mychatserver.com): ";
            cin  >> connection_ip;
            connection_success = establish_connection(socket_file_descriptor, connection_ip, connection);
            break;
        
        case 'q': 
            exit(0);

        default:
            cout << "No such option" << endl;
            break;
        }
    }
    return true;
}


bool hangUp(int& socket_file_descriptor) {
    char command[] = "HangUp";
    size_t bytes = -1;
    bytes = write(socket_file_descriptor, command, sizeof(command));
    return close(socket_file_descriptor);
}


bool changeUserPassword(int& socket_file_descriptor, string& login, uint* currentHash, uint* newHash) {
    bool password_changed = false;
    char command[] = "changeUserPassword";
    ssize_t bytes = write(socket_file_descriptor, command, sizeof(command));
    if (bytes >= 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        bytes = -1;
        while (bytes < 0) {
            bytes = write(socket_file_descriptor, login.c_str(), 32);
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
        bytes = -1;
        while (bytes < 0) {
            bytes = write(socket_file_descriptor, (char*) currentHash, 32);
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
        bytes = -1;
        while (bytes < 0) {
            bytes = write(socket_file_descriptor, (char*) newHash, 32);
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
        read(socket_file_descriptor, (char*) &password_changed, sizeof(bool));
    }
    return password_changed;
}


void send_chat_message(string &login, string &all, vector<Message> &messages, const string messages_file_path) {
    string message;
    cout << "Message: ";
    getline(cin >> ws, message);
    newMessage(message, login, all, messages, messages_file_path);
    showMessages(messages, login, all);
}


short send_private_message(int socket_file_descriptor, char connection_ip[], int connection,
                           string &login, vector<Message> &messages, string& all,
                           const string messages_file_path)
{
    string message, recipient;
    cout << "Recipient: ";
    cin >> recipient;
    connection_wrapper(socket_file_descriptor, connection_ip, connection);
    if (!userExists(recipient, socket_file_descriptor)) {
        cout << "No such user exists\n";
        hangUp(socket_file_descriptor);
        return -1;
    }
    else {
        hangUp(socket_file_descriptor);
    }
    cout << "Private message for " << recipient << ": ";
    getline(cin >> ws, message);
    newMessage(message, login, recipient, messages, messages_file_path);
    showMessages(messages, login, all);
    return 0;
}


void change_user_password_wrapper(uint* hash, int& socket_file_descriptor, char connection_ip[],
                                  int connection, string& login)
{
    string currentPassword;
    bool passwordFlag = false;
    cout << "Your current password: ";
    SetStdinEcho(false);
    cin >> currentPassword;
    SetStdinEcho(true);
    uint* currentHash = sha1(currentPassword.c_str(), currentPassword.length());
    cout << endl;
    if (*currentHash == *hash) {
        string newPassword = setPassword(passwordFlag);
        uint* newHash = sha1(newPassword.c_str(), newPassword.length());
        if (passwordFlag) {
            connection_wrapper(socket_file_descriptor, connection_ip, connection);
            bool passwordChanged = changeUserPassword(socket_file_descriptor, login, currentHash, newHash);
            if (passwordChanged) cout << "\nPassword was successfully changed.\n";
            else cout << "\nPassword was not changed.\n";
            hangUp(socket_file_descriptor);
            *hash = *newHash;
        }
    }
    else {
        cout << "Wrong current password.";
    }
}


void doChat(vector<Message>& messages, string& login, uint* hash, vector<User>& users, User& user,
            const string messages_file_path, int connection, int socket_file_descriptor, char connection_ip[]) {
    char key;
    string all = "all"; // Placeholder for "all users"
    while (key != 'l') {
        cout << "\nPlease choose:\n\nm - Read & write messages\nc - Change password\nl - Logout" << endl;
        cin >> key;

        switch(key) {
            case 'm':
                cout << endl;
                cout << "Showing messages for user " << login << endl;
                showMessages(messages, login, all);

                while (key != 'q') {
                    cout << "\nPlease choose:\n\nw - Send a chat message\np - Send a private message\nq - Go back\n";
                    cin >> key;
                    switch(key) {
                        case 'w':
                            send_chat_message(login, all, messages, messages_file_path);
                            break;
                        case 'p':
                            send_private_message(socket_file_descriptor, connection_ip,
                                                 connection, login, messages, all, messages_file_path);
                            break;
                    }
                }
                break;

            case 'c':
                change_user_password_wrapper(hash, socket_file_descriptor, connection_ip,
                                             connection, login);
                break;
        }
    }
}


void set_perms(const string file_path) {
    fs::permissions(file_path,
    fs::perms::group_all | fs::perms::others_all,
    fs::perm_options::remove);
}


void login_user(int connection, int& socket_file_descriptor, char connection_ip[], vector<Message>& messages,
                vector<User>& users, User& user, const string messages_file_path)
{
    string login, password;
    cout << "Login: ";
    cin >> login;
    if (userExists(login, socket_file_descriptor)) {
        uint* hash = new uint;
        for (int i=0; i<3; i++) {
            cout << "Password: ";
            SetStdinEcho(false);
            cin >> password;
            SetStdinEcho(true);
            cout << endl;
            if (checkPassword(login, password, hash, socket_file_descriptor)) {
                cout << login << " successfully logged in\n";
                char command[] = "HangUp";
                write(socket_file_descriptor, command, sizeof(command));
                close(socket_file_descriptor);
                doChat(messages, login, hash, users, user, messages_file_path,
                        connection, socket_file_descriptor, connection_ip);
                connection_wrapper(socket_file_descriptor, connection_ip, connection);
                break;
            }
            cout << "Incorrect password\n";
        }
    }
    else {
        cout << "No such user\n";
    }
}


int main() {
    // Create config folder if it does not exist
    string home = getenv("HOME");
    string config = "/.config/console-chat";
    string home_config = home + config;
    string messages_file_path = home_config + "/messages.txt";
    fs::create_directory(home + config);

    vector<User> users(0);
    User user; // User for a current authorized session

    vector<Message> messages(0);
    string login, password;
    string name;
    uint* hash;
    login = "all"; // To add the default login for the "all" user to the users vector
    password = "password"; // To add the default password for the "all" user
    int pass_length = password.length();
    hash = sha1(password.c_str(), pass_length);
    name = "all"; // To add the default name for the "all" user
    bool passwordFlag = false;
    char key;

    if (!readObjsFromFile(messages_file_path, messages)) {
        cout << "Could not open file " << messages_file_path << "!\n";
        return 1;
    }
    else
        set_perms(messages_file_path);

    // Connection establishment part
    int socket_file_descriptor, connection;
    char connection_ip[] = "127.0.0.1";

    connection_wrapper(socket_file_descriptor, connection_ip, connection);

    cout << "Connection with the server (IP " << connection_ip << ") has been established.\n";
    while (true) {
        cout << "\nPlease choose:\n\nl - Login\nq - Quit" << endl;
        cin >> key;

        switch(key) {
            // case 'n':
            // {
            //     cout << "Please enter new user login: ";
            //     cin >> login;
            //     password = setPassword(passwordFlag);
            //     uint* hash = sha1(password.c_str(), password.length());
            //     if (!passwordFlag) break;
            //     passwordFlag = false;
            //     cout << "\nPlease enter new user name: ";
            //     cin >> name;
            //     // newUser(login, hash, name, users, user, users_file_path);
            //     break;
            // }

            case 'l':
                login_user(connection, socket_file_descriptor, connection_ip, messages,
                           users, user, messages_file_path);
                break;

            case 'q': 
                exit(0);

            default:
                cout << "No such option" << endl;
        }
    }
}
