#include <mysql/mysql.h>


void command_check_user_exists(int &connection, char login[], int param_size,
                               MYSQL &mysql);
void command_check_user_password(int& connection, char login[], MYSQL& mysql);
void command_change_user_password(int& connection, char login[], MYSQL& mysql);
void command_transmit_message(int& connection, MYSQL& mysql);
void command_get_messages(int& connection, char login[], MYSQL& mysql);
void process_connection_command(char command[], int& connection, char login[],
                                int param_size, MYSQL& mysql);
