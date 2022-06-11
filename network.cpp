#include <chrono>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <mysql/mysql.h>
#include <thread>
#include <unistd.h>
#include "headers/message.h"
#include "headers/sha1.h"
#include "headers/constants.h"
#include "headers/commands.h"
#include "headers/main_server.h"


void transmit_message(const Message& message, int& sock,
                      bool& message_is_accepted)
{
    message_is_accepted = false;
    char command[] = "transmit_message";
    int bytes = write(sock, command, cmd_size);
    if (bytes > 0) {
        bytes = write(sock, message.getMessage().c_str(), msg_size);
        if (bytes <= 0) {
            cout << CONNECTION_LOST_CLIENT_MSG << endl;
            return;
        }
        bytes = write(sock, message.getAuthor().c_str(), usr_size);
        if (bytes <= 0) {
            cout << CONNECTION_LOST_CLIENT_MSG << endl;
            return;
        }
        bytes = write(sock, message.forWhom().c_str(), usr_size);
        if (bytes <= 0) {
            cout << CONNECTION_LOST_CLIENT_MSG << endl;
            return;
        }
        bytes = read(sock, (char*) &message_is_accepted, size_of_bool);
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


bool user_exists(std::string& login, int& sock) {
    char command[] = "check_user_exists";
    ssize_t bytes = -1;
    bytes = write(sock, command, cmd_size);
    bool login_exists = false;
    if (bytes > 0) {
      bytes = write(sock, login.c_str(), usr_size);
      if (bytes <= 0) {
        cout << CONNECTION_LOST_CLIENT_MSG << endl;
        // TODO: Handle lost connection
      }
      else {
        read(sock, (char*) &login_exists, size_of_bool);
      }
    }
    return login_exists;
}


bool password_is_correct(const std::string& login, const std::string &password,
                               uint* &hash, int& sock)
{
    hash = sha1(password.c_str(), password.length());
    char command[] = "check_user_password";
    ssize_t bytes = write(sock, command, cmd_size);
    bool password_is_correct = false;
    if (bytes > 0) {
        bytes = write(sock, (char*) hash, hsh_size);
        if (bytes > 0)
            read(sock, (char*) &password_is_correct, size_of_bool);
    }
    return password_is_correct;
}


void show_messages(const std::string& login, const std::string& all, int& sock) {
    char recipient[32], sender[32], message[1024];
    std::vector<Message> messages;
    std::string end = "end";
    char command[] = "get_messages";
    ssize_t bytes = write(sock, command, cmd_size);
    if (bytes > 0) {
        while(true) {
            read(sock, (char*) &message, msg_size);
            if (message == end) break;
            read(sock, (char*) &sender, usr_size);
            bytes = read(sock, (char*) &recipient, usr_size);
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


void establish_connection(int& sock, const char* connection_ip,
                          int& connection, bool& connection_success)
{
    struct sockaddr_in serveraddress, client;
    sock = socket(AF_INET, SOCK_STREAM, 0); // Create a socket
    if(sock == -1) {
        return;
    }
    serveraddress.sin_addr.s_addr = inet_addr(connection_ip);
    serveraddress.sin_port = htons(PORT);
    serveraddress.sin_family = AF_INET; // Using IPv4
    connection = connect(sock, (struct sockaddr*)& serveraddress,
                         sizeof(serveraddress));
    connection_success = connection != -1;
}


bool hang_up(int& sock) {
    char command[] = "hang_up";
    // TODO: bytes check
    write(sock, command, sizeof(command));
    shutdown(sock, SHUT_RDWR);
    return close(sock);
}


bool change_user_password(int& sock, string& login,
                          uint* current_hash, uint* new_hash)
{
    bool password_changed = false;
    char command[] = "change_user_password";
    ssize_t bytes = write(sock, command, cmd_size);
    if (bytes > 0) {
        write(sock, login.c_str(), usr_size);
        write(sock, (char*) current_hash, hsh_size);
        write(sock, (char*) new_hash, hsh_size);
        read(sock, (char*) &password_changed, size_of_bool);
    }
    return password_changed;
}


void take_commands(int& connection, int& sock, MYSQL& mysql) {
    cout << "Take commands session started\n";
    char command[cmd_size], login[usr_size];
    // Add atomic flag server_is_active
    while (true) {
        int bytes = read(connection, command, cmd_size);
        if (!strcmp(command, "hang_up") || bytes == 0) {
            close(connection);
            cout << "Client has disconnected\n";
            break;
        }
        process_connection_command(command, connection, login,
                                   usr_size, mysql);
    }
}


bool accept_connection(int& sock, int connection, MYSQL& mysql) {
    int client;
    socklen_t length;
    while (true) {
        int conn = accept(sock, (struct sockaddr*) &client, &length);
        if(conn < 0) {
            return false;
        }
        cout << "Launching take_commands\n";
        take_commands(connection, sock, mysql);
        return true;
    }
}


void init_connection(int& sock, struct sockaddr_in& client, socklen_t& length) {
    length = sizeof(client);
    struct sockaddr_in serveraddress;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return;
    }
    bzero((char *) &serveraddress, sizeof(serveraddress));
    // Define server address
    serveraddress.sin_addr.s_addr = htonl(INADDR_ANY);
    // Define port number
    serveraddress.sin_port = htons(PORT);
    // Using IPv4
    serveraddress.sin_family = AF_INET;
    int bind_status = bind(sock, (struct sockaddr*) &serveraddress, sizeof(serveraddress));
    if (bind_status == -1)  {
        sock = -1;
        cout << SOCK_BIND_ERR_MSG << endl;
        return;
    }
    // Set server to accept data
    int connection_status = listen(sock, 5);
    if (connection_status < 0) {
        sock = -1;
        cout << CONNECTION_LISTEN_ERR << endl;
    }
}
