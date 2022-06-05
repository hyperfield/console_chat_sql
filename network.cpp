    #include <chrono>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>

#include "headers/message.h"
#include "headers/sha1.h"
#include "headers/constants.h"


void transmit_message(Message& message, int& socket_file_descriptor, bool& message_is_accepted) {
    message_is_accepted = false;
    char command[] = "transmit_message";
    int bytes = write(socket_file_descriptor, command, cmd_size);
    if (bytes > 0) {
        bytes = -1;
        bytes = write(socket_file_descriptor, message.getMessage().c_str(), msg_size);
        if (bytes <= 0) {
            cout << CONNECTION_LOST_CLIENT_MSG << endl;
            return;
        }
        bytes = -1;
        bytes = write(socket_file_descriptor, message.getAuthor().c_str(), usr_size);
        if (bytes <= 0) {
            cout << CONNECTION_LOST_CLIENT_MSG << endl;
            return;
        }
        bytes = -1;
        bytes = write(socket_file_descriptor, message.forWhom().c_str(), usr_size);
        if (bytes <= 0) {
            cout << CONNECTION_LOST_CLIENT_MSG << endl;
            return;
        }
        bytes = -1;
        bytes = read(socket_file_descriptor, (char*) &message_is_accepted, size_of_bool);
        if (bytes <= 0) {
            cout << CONNECTION_LOST_CLIENT_MSG << endl;
            return;
        }
    }
}


void send_new_message(const std::string &message, const std::string &author,
                            const std::string &for_user, int& socket_file_descriptor)
{
    Message new_message(message, author, for_user);
    bool transmission_result = false;
    transmit_message(new_message, socket_file_descriptor, transmission_result);
}


const bool user_exists(std::string& login, int& socket_file_descriptor) {
    char command[] = "check_user_exists";
    ssize_t bytes = -1;
    bytes = write(socket_file_descriptor, command, cmd_size);
    bool login_exists = false;
    if (bytes > 0) {
      bytes = -1;
      bytes = write(socket_file_descriptor, login.c_str(), usr_size);
      if (bytes <= 0) {
        cout << CONNECTION_LOST_CLIENT_MSG << endl;
        // TODO: Handle lost connection
      }
      else {
        read(socket_file_descriptor, (char*) &login_exists, size_of_bool);
      }
    }
    return login_exists;
}


const bool password_is_correct(const std::string& login, const std::string &password,
                               uint* &hash, int& socket_file_descriptor)
{
    hash = sha1(password.c_str(), password.length());
    char command[] = "check_user_password";
    ssize_t bytes = write(socket_file_descriptor, command, cmd_size);
    bool password_is_correct = false;
    if (bytes > 0) {
        bytes = -1;
        bytes = write(socket_file_descriptor, (char*) hash, hsh_size);
        if (bytes > 0)
            read(socket_file_descriptor, (char*) &password_is_correct, size_of_bool);
    }
    return password_is_correct;
}


void show_messages(const std::string& login, const std::string& all, int& socket_file_descriptor) {
    char recipient[32], sender[32], message[1024];
    std::vector<Message> messages;
    std::string end = "end";
    char command[] = "get_messages";
    ssize_t bytes = write(socket_file_descriptor, command, cmd_size);
    if (bytes > 0) {
        while(true) {
            read(socket_file_descriptor, (char*) &message, msg_size);
            if (message == end) break;
            read(socket_file_descriptor, (char*) &sender, usr_size);
            bytes = read(socket_file_descriptor, (char*) &recipient, usr_size);
            Message new_message(message, sender, recipient);
            messages.push_back(new_message);
            // Reset the char arrays
            std::fill(message, message+1024, 0);
            std::fill(sender, sender+32, 0);
            std::fill(recipient, recipient+32, 0);
            if (bytes <= 0) {
                break;
            }
        }
    }
    // Enlist messages received from server
    for (Message& msg : messages) {
        cout << msg.getAuthor() << " => " << msg.forWhom() << ": " << msg.getMessage() << endl;
    }
    if (bytes <= 0) {
        cout << "\nWarning: Connection to server appears to have been lost!\n";
    }
}


void establish_connection(int& socket_file_descriptor, const char* connection_ip,
                          int& connection, bool& connection_success)
{
    struct sockaddr_in serveraddress, client;
    socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0); // Create a socket
    if(socket_file_descriptor == -1) {
        return;
    }
    serveraddress.sin_addr.s_addr = inet_addr(connection_ip);
    serveraddress.sin_port = htons(PORT);
    serveraddress.sin_family = AF_INET; // Using IPv4
    connection = connect(socket_file_descriptor, (struct sockaddr*)& serveraddress, sizeof(serveraddress));
    connection_success = connection != -1;
}


bool hang_up(int& socket_file_descriptor) {
    char command[] = "hang_up";
    size_t bytes = write(socket_file_descriptor, command, sizeof(command));
    return close(socket_file_descriptor);
}


bool change_user_password(int& socket_file_descriptor, string& login,
                          uint* current_hash, uint* new_hash)
{
    bool password_changed = false;
    char command[] = "change_user_password";
    ssize_t bytes = write(socket_file_descriptor, command, cmd_size);
    if (bytes > 0) {
        write(socket_file_descriptor, login.c_str(), usr_size);
        write(socket_file_descriptor, (char*) current_hash, hsh_size);
        write(socket_file_descriptor, (char*) new_hash, hsh_size);
        read(socket_file_descriptor, (char*) &password_changed, size_of_bool);
    }
    return password_changed;
}


bool accept_connection(int& socket_file_descriptor, int& connection) {
    struct sockaddr_in serveraddress, client;
    socklen_t length;
    socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_file_descriptor < 0) {
        return false;
    }
    bzero((char *) &serveraddress, sizeof(serveraddress));
    // Define server address
    serveraddress.sin_addr.s_addr = htonl(INADDR_ANY);
    // Define port number
    serveraddress.sin_port = htons(PORT);
    // Using IPv4
    serveraddress.sin_family = AF_INET;
    // Establish a connection with the server
    int bind_status = bind(socket_file_descriptor, (struct sockaddr*) &serveraddress, sizeof(serveraddress));
    if (bind_status == -1)  {
        cout << "Socket binding failed" << endl;
        return false;
    }
    else {
        cout << "Socket binding succeeded" << endl;
    }
    // Set the server to accept data
    int connection_status = listen(socket_file_descriptor, 5);

    if (connection_status < 0) {
        return false;
    }

    connection = accept(socket_file_descriptor, (struct sockaddr*) &client, &length);
    if(connection < 0) {
        return false;
    }
    return true;
}
