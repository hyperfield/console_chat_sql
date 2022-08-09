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


void transmit_message(const Message &message, int &sock,
                      bool &message_is_accepted)
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
                            const std::string &for_user, int &socket_file_descriptor)
{
    Message new_message(message, author, for_user);
    bool transmission_result = false;
    transmit_message(new_message, socket_file_descriptor, transmission_result);
}


bool user_exists(std::string &login, int &sock) {
    char command[] = "check_usr_exists";
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


bool password_is_correct(const std::string &password,
                               uint* &hash, int &sock)
{
    hash = sha1(password.c_str(), password.length());
    char command[] = "check_usr_passwd";
    ssize_t bytes = write(sock, command, cmd_size);
    bool password_is_correct = false;
    if (bytes > 0) {
        bytes = write(sock, (char*) hash, hsh_size);
        if (bytes > 0)
            read(sock, (char*) &password_is_correct, size_of_bool);
    }
    return password_is_correct;
}


void show_messages(int &sock) {
    char recipient[usr_size], sender[usr_size], message[msg_size];
    std::vector<Message> messages;
    std::string end = "end";
    char command[] = "get_the_messages";
    int bytes = write(sock, command, cmd_size);
    int counter = 0;
    if (bytes > 0) {
        while(true) {
            counter++;
            bytes = read(sock, (char*) &message, msg_size);
            if (bytes <= 0) {
                cout << "Communication error\n";
                break;
            }
            cout << "Reading first part of message\n";
            if (message == end) break;
            bytes = read(sock, (char*) &sender, usr_size);
            if (bytes <= 0) {
                cout << "Communication error\n";
                break;
            }
            bytes = read(sock, (char*) &recipient, usr_size);
            if (bytes <= 0) {
                cout << "Communication error\n";
                break;
            }
            cout << "Read second part of message\n";
            Message new_message(message, sender, recipient);
            messages.push_back(new_message);
            cout << "counter: " << counter << endl;
            // Reset the char arrays
            std::fill(message, message+msg_size, 0);
            std::fill(sender, sender+usr_size, 0);
            std::fill(recipient, recipient+usr_size, 0);
            if (bytes <= 0) {
                break;
            }
        }
    }
    cout << "Done the loop\n";
    // Enlist messages received from server
    for (Message& msg : messages) {
        cout << msg.getAuthor() << " => " << msg.forWhom() << ": " << msg.getMessage() << endl;
    }
    if (bytes <= 0) {
        cout << "\nWarning: Connection to server appears to have been lost!\n";
    }
}


void establish_connection(int &sock, const char *connection_ip,
                          int &connection, bool &connection_success)
{
    struct sockaddr_in serveraddress;
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


void hang_up(int& sock) {
    char command[] = "hang_up_session_";
    // TODO: bytes check
    write(sock, command, cmd_size);
}


bool change_user_password(int &sock, string &login,
                          uint *current_hash, uint *new_hash)
{
    bool password_changed = false;
    char command[] = "chang_usr_passwd";
    ssize_t bytes = write(sock, command, cmd_size);
    if (bytes > 0) {
        write(sock, login.c_str(), usr_size);
        write(sock, (char*) current_hash, hsh_size);
        write(sock, (char*) new_hash, hsh_size);
        read(sock, (char*) &password_changed, size_of_bool);
    }
    return password_changed;
}


void take_commands(int &connection, MYSQL &mysql, bool &stop_thread)
{
    char command[cmd_size], login[usr_size];
    do {
        int bytes = read(connection, command, cmd_size);
        if (!strcmp(command, "hang_up_session_") || bytes == 0) {
            close(connection);
            break;
        }
        process_connection_command(command, connection, login,
                                   usr_size, mysql);
    } while (!stop_thread);
}


bool accept_connection(int &sock, int &connection, MYSQL &mysql,
                       bool &stop_thread)
{
    int client;
    socklen_t length;
    while (true) {
        int conn = accept(sock, (struct sockaddr*) &client, &length);
        if(conn < 0) {
            return false;
        }
        take_commands(connection, mysql, stop_thread);
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


void accept_connection_wrapper(int &sock, MYSQL &mysql, bool &stop_thread)
{
    while (!stop_thread) {
        struct sockaddr_in client;
        socklen_t length = sizeof(client);
        int connection = accept(sock, (struct sockaddr*) &client, &length);
        if (connection != -1) {
            cout << "Connection with client established\n";
            thread t(take_commands, ref(connection),
                     ref(mysql), ref(stop_thread));
            t.detach();
        }
        else {
            cout << CONNECTION_ACCEPT_ERR << endl;
            break;
        }
    }
}


void accept_connections(int& sock, MYSQL& mysql, bool& stop_thread)
{
    thread t1(accept_connection_wrapper, ref(sock), ref(mysql),
              ref(stop_thread));
    t1.detach();
    cout << "Enter 'q' to go back\n";
    char a_key(' ');
    while (a_key != 'q') {
        cin >> a_key;
        if (a_key == 'q')
            break;
    }
}

