#pragma once
#include <string>


void connection_wrapper(int& socket_file_descriptor, char connection_ip[],
                        int& connection, bool& connection_success);
void send_chat_message(std::string &login, std::string &all,
                       int& socket_file_descriptor);
void send_private_message(std::string &login, std::string& all,
                          int& socket_file_descriptor);
void change_user_password_wrapper(uint* hash, std::string& login,
                                  int& socket_file_descriptor);
void show_message_menu(std::string& login, int& socket_file_descriptor);
void start_chat(std::string& login, uint* hash, int& socket_file_descriptor);
void login_user(int& socket_file_descriptor);
void show_main_menu(int& socket_file_descriptor);
void set_password(bool &password_flag, std::string& new_password);
