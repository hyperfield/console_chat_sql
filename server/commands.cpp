#include <cstring>
#include <iostream>
#include <mysql/mysql.h>
#include <unistd.h>

#include "../headers/constants.h"
#include "../headers/database.h"

using std::cout;
using std::endl;


void command_check_user_exists(int &connection, char login[], int param_size,
                               MYSQL &mysql) {
  bool usr_exists = false;
  read(connection, login, param_size);
  if (user_exists(login, mysql)) {
    usr_exists = true;
  }
  int bytes = write(connection, (char *) &usr_exists, size_of_bool);
  if (bytes == 0) {
    cout << CONNECTION_LOST_SERVER_MSG << endl;
    // TODO: HANDLE LOST CONNECTION
  } else if (bytes == -1) {
    cout << CONNECTION_ERROR_MSG << endl;
    // TODO: HANDLE CONNECTION ERROR
  }
}


void command_check_user_password(int& connection, char login[], MYSQL& mysql)
{
    bool password_correct = false;
    char pass_param[hsh_size];
    read(connection, pass_param, hsh_size);
    uint* int_pass_param = (uint*) &pass_param;
    if (check_password(login, int_pass_param, mysql))  {
        // TODO: Logger messages
        // cout << "User " << login << " was authenticated\n";
        password_correct = true;
    }
    else {
        // cout << "User " << login << " was not authenticated\n";
    }
    write(connection, (char*) &password_correct, size_of_bool);
}


void command_change_user_password(int& connection, char login[], MYSQL& mysql)
{
    bool password_changed = false;
    char login_param[usr_size],
         current_hash_param[hsh_size],
         new_hash_param[hsh_size];
    read(connection, login_param, usr_size);
    read(connection, current_hash_param, hsh_size);
    uint* int_old_hash_param = (uint*) &current_hash_param;
    read(connection, new_hash_param, hsh_size);
    uint* int_new_hash_param = (uint*) &new_hash_param;
    User this_user = find_user(login, int_old_hash_param, mysql);
    if (this_user.getLogin() != login_param) {
        write(connection, (char*) &password_changed, size_of_bool);
        // cout << "Password not changed\n";
    }
    else if (this_user.changePassword(int_old_hash_param, int_new_hash_param)) {
        if (update_user_pwdhash_in_DB(this_user, mysql)) {
            password_changed = true;
            // cout << "Password successfully changed\n";
        }
        else {
            // cout << "Coulnd't change this user password\n";
        }
        write(connection, (char*) &password_changed, size_of_bool);
    }
}


void command_transmit_message(int& connection, MYSQL& mysql) {
    bool message_accepted = false;
    char message[msg_size],
            message_sender[usr_size],
            message_receiver[usr_size];
    read(connection, message, msg_size);
    read(connection, message_sender, usr_size);
    read(connection, message_receiver, usr_size);
    Message this_message(message, message_sender, message_receiver);
    write_message_to_db(this_message, mysql);
    message_accepted = true;
    ssize_t bytes = -1;
    bytes = write(connection, (char*) &message_accepted, size_of_bool);
    if (bytes == 0) {
        cout << CONNECTION_LOST_SERVER_MSG << endl;
        // TODO: HANDLE LOST CONNECTION
    }
    else if (bytes == -1) {
        cout << CONNECTION_ERROR_MSG << endl;
        // TODO: HANDLE CONNECTION ERROR
    }
}


void command_get_messages(int& connection, char login[], MYSQL& mysql) {
    std::vector<Message> show_messages = read_messages_from_db(n_msgs_on_screen, login, mysql);
    for (auto& message : show_messages) {
        write(connection, message.getMessage().c_str(), msg_size);
        write(connection, message.getAuthor().c_str(), usr_size);
        write(connection, message.forWhom().c_str(), usr_size);
    }
    char end[] = "end";
    write(connection, end, 4);
}


void process_connection_command(char command[], int& connection, char login[],
                                int param_size, MYSQL& mysql) {
    cout << "Received command " << command << endl;
    if (!strcmp(command, "check_usr_exists")) {
        command_check_user_exists(connection, login, param_size, mysql);
    }
    else if (!strcmp(command, "check_usr_passwd")) {
        command_check_user_password(connection, login, mysql);
    }
    else if (!strcmp(command, "chang_usr_passwd")) {
        command_change_user_password(connection, login, mysql);
    }
    else if (!strcmp(command, "transmit_message")) {
        command_transmit_message(connection, mysql);
    }
    else if (!strcmp(command, "get_the_messages")) {
        command_get_messages(connection, login, mysql);
    }
}
