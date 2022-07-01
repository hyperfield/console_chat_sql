#include "message.h"
#include <mysql/mysql.h>
#include <sys/socket.h>


void establish_connection(int& sock,
                          const char* connection_ip,
                          int& connection, bool& connection_success);
void transmit_message(Message& message, int& sock,
                      bool& message_is_accepted);
void send_new_message(const std::string &message, const std::string &author,
                            const std::string &for_user,
                            int& sock);
bool user_exists(std::string& login, int& sock);
bool password_is_correct(const std::string& login,
                         const std::string &password,
                         uint* &hash, int& sock);
void show_messages(const std::string& login, const std::string& all,
                   int& sock);
void hang_up(int& sock);
bool change_user_password(int& sock, std::string& login,
                          uint* current_hash, uint* new_hash);
bool accept_connection(int& sock, int connection,
                       bool& stop_thread);
void take_commands(int& connection, MYSQL& mysql, bool& stop_thread);
bool accept_connection(int& sock, int connection, MYSQL& mysql);
void init_connection(int& sock, struct sockaddr_in& client,
                     socklen_t& length);
void accept_connections(int& sock, MYSQL& mysql, bool& stop_thread);
void accept_connection_wrapper(int& sock, MYSQL& mysql, bool& stop_thread);
