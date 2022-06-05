#pragma once
#include "message.h"


void establish_connection(int& socket_file_descriptor, const char* connection_ip,
                          int& connection, bool& connection_success);
void transmit_message(Message& message, int& socket_file_descriptor, bool& message_is_accepted);
void send_new_message(const std::string &message, const std::string &author,
                            const std::string &for_user, int& socket_file_descriptor);
const bool user_exists(std::string& login, int& socket_file_descriptor);
const bool password_is_correct(const std::string& login, const std::string &password,
                               uint* &hash, int& socket_file_descriptor);
void show_messages(const std::string& login, const std::string& all, int& socket_file_descriptor);
bool hang_up(int& socket_file_descriptor);
bool change_user_password(int& socket_file_descriptor, std::string& login,
                          uint* current_hash, uint* new_hash);
bool accept_connection(int& socket_file_descriptor, int& connection);
