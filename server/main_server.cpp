#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <netinet/in.h>
#include <mysql/mysql.h>
#include <sys/socket.h>
#include <stdio.h>
#include <thread>
#include <unistd.h>
#include <vector>

#include "../headers/console.h"
#include "../headers/commands.h"
#include "../headers/user.h"
#include "../headers/message.h"
#include "../headers/passwordShadow.h"
#include "../headers/sha1.h"
#include "../headers/constants.h"
#include "../headers/database.h"
#include "../headers/network.h"


using namespace std;


void process_connection_command(char command[], int& connection, char login[],
                                int param_size, MYSQL& mysql) {
    if (!strcmp(command, "check_user_exists")) {
        command_check_user_exists(connection, login, param_size, mysql);
    }
    else if (!strcmp(command, "check_user_password")) {
        command_check_user_password(connection, login, mysql);
    }
    else if (!strcmp(command, "change_user_password")) {
        command_change_user_password(connection, login, mysql);
    }
    else if (!strcmp(command, "transmit_message")) {
        command_transmit_message(connection, mysql);
    }
    else if (!strcmp(command, "get_messages")) {
        command_get_messages(connection, login, mysql);
    }
    else {
        // cout << "No such command\n";
    }
}


void connect_to_db(MYSQL &mysql) {
  // TODO: Read db info from config
  if (!mysql_real_connect(&mysql, "localhost", "chat_root", "#Ch@7R00T!",
                          "chat_db", 0, NULL, 0))
    {
    cout << "Error: unable to connect to MySQL due to the following error: "
         << mysql_error(&mysql) << endl;
    exit(1);
    }
  else {
    mysql_set_character_set(&mysql, "utf8");
    cout << "Successfully connected to MySQL database." << endl;
  }
}


int main() {
  MYSQL mysql;
  MYSQL_ROW row;
  MYSQL_RES *result;
  mysql_init(&mysql);
  connect_to_db(mysql);

  bool connection_success;
  bool password_flag = false;
  char key;
  char message[msg_size], command[cmd_size], checkLogin[usr_size];
  User user; // User for current authenticated session
  string password, email, login, name;
  // Add default password hash for user "all" to DB if not there
  password = "password";
  uint* hash = sha1(password.c_str(), password.length());
  // Adding the default "all" user
  new_user("all", hash, "all", "", user, mysql);

  while (true) {
    cout << "\nPlease choose:\n\na - Accept connections\nn - New user\nq - Quit"
         << endl;
    cin >> key;
    switch (key) {
    case 'a':
    {
      int socket_file_descriptor, connection;
      {
        while (true) {
          connection_success = 
            accept_connection(socket_file_descriptor, connection);
          if (connection_success) {
            cout << "Connection with a client was established" << endl;
            while (true) {
              int bytes = read(connection, command, 32);
              if (!strcmp(command, "hang_up")) {
                close(socket_file_descriptor);
                close(connection);
                cout << "Client has disconnected\n";
                break;
              }
              process_connection_command(command, connection, checkLogin,
                                         usr_size, mysql);
            }
            close(socket_file_descriptor);
            close(connection);
          } else {
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
          }
        }
      }
    }

    case 'n': {
      cout << "Please enter new user login: ";
      cin >> login;
      string new_password;
      set_password(password_flag, new_password);
      uint *hash = sha1(password.c_str(), password.length());
      if (!password_flag) {
        cout << "Password mismatch\n";
        break;
      }
      password_flag = false;
      cout << "\nPlease enter new user name: ";
      cin >> name;
      cout << "\nPlease enter user e-mail: ";
      cin >> email;
      if (!new_user(login, hash, name, email, user, mysql)) {
        cout << "This login already exists!\n";
      }
      break;
    }

    case 'q': {
      mysql_close(&mysql);
      exit(0);
    }

    default:
      cout << "No such option" << endl;
    }
  }
  return 0;
}
