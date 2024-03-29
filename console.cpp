#include <iostream>
#include <string>
#include <unistd.h>
#include "headers/database.h"
#include "headers/passwordShadow.h"
#include "headers/network.h"
#include "headers/sha1.h"

using std::cin;
using std::cout;
using std::endl;


void set_password(bool &password_flag, std::string& new_password)
{
    int counter = 0;
    while (!password_flag && counter++ < 3) {
        string password, confirm_password;
        cout << "Please enter new user password: ";
        SetStdinEcho(false);
        cin >> password;
        SetStdinEcho(true);
        cout << endl;
        cout << "Confirm new user password: ";
        SetStdinEcho(false);
        cin >> confirm_password;
        SetStdinEcho(true);
        cout << endl;
        if (password == confirm_password) {
            password_flag = true;
            new_password = password;
        }
    }
}


void connection_wrapper(int& socket_file_descriptor, char connection_ip[],
                        int& connection, bool& connection_success)
{
  establish_connection(socket_file_descriptor, connection_ip,
                       connection, connection_success);
  char key;
  while (!connection_success) {
    cout << "\nPlease choose:\n\ni - Specify a Console Chat server\nq - Quit\n";
    cin >> key;

    switch (key) {
    case 'i':
      cout << "Console chat server hostname (or IP address) (e.g. "
              "127.0.0.1 or mychatserver.com): ";
      establish_connection(socket_file_descriptor, connection_ip,
                           connection, connection_success);
      break;

    case 'q':
      exit(0);

    default:
      cout << "No such option" << endl;
      break;
    }
  }
}


void send_chat_message(const std::string &login, const std::string &all,
                       int &socket_file_descriptor)
{
    std::string message;
    cout << "Message: ";
    getline(std::cin >> std::ws, message);
    send_new_message(message, login, all, socket_file_descriptor);
    show_messages(socket_file_descriptor);
}


void send_private_message(const std::string &login, const std::string &all,
                          int &socket_file_descriptor)
{
  std::string message, recipient;
  cout << "Recipient: ";
  cin >> recipient;
  if (!user_exists(recipient, socket_file_descriptor)) {
    cout << "No such user exists\n";
    return;
    }
    cout << "Private message for " << recipient << ": ";
    getline(std::cin >> std::ws, message);
    send_new_message(message, login, recipient, socket_file_descriptor);
    show_messages(socket_file_descriptor);
}


void change_user_password_wrapper(uint *hash, std::string &login,
                                  int &socket_file_descriptor)
{
  std::string current_password;
  cout << "Your current password: ";
  SetStdinEcho(false);
  cin >> current_password;
  SetStdinEcho(true);
  uint *current_hash =
      sha1(current_password.c_str(), current_password.length());
  cout << std::endl;
  if (*current_hash == *hash) {
    std::string new_password;
    bool password_flag = false;
    set_password(password_flag, new_password);
    uint *new_hash = sha1(new_password.c_str(), new_password.length());
    if (password_flag) {
      bool password_changed = change_user_password(
          socket_file_descriptor, login, current_hash, new_hash);
      if (password_changed)
        std::cout << "\nPassword was successfully changed.\n";
      else
        std::cout << "\nPassword was not changed.\n";
      *hash = *new_hash;
    }
  }
  else {
    cout << "Wrong current password.";
  }
}


void show_message_menu(const std::string &login, int &sock)
{
    std::cout << "\nShowing messages for user " << login << std::endl;
    std::string all = "all"; // Placeholder for "all users"
    show_messages(sock);
    char key = '0';
    while (key != 'q') {
      cout << "\nPlease choose:\n\nw - Send a chat message\np - Send a private "
              "message\nq - Go back\n";
      cin >> key;
      switch (key) {
      case 'w':
        send_chat_message(login, all, sock);
        break;

      case 'p':
        send_private_message(login, all, sock);
        break;
        }
    }
}

void start_chat(std::string &login, uint *hash, int &socket_file_descriptor)
{
    char key = '\0';
    while (key != 'l') {
        std::cout << "\nPlease choose:\n\nm - Read & write messages\nc - Change password\nl - Logout" << std::endl;
        std::cin >> key;

        switch(key) {
            case 'm':
                show_message_menu(login, socket_file_descriptor);
                break;

            case 'c':
                change_user_password_wrapper(hash, login, socket_file_descriptor);
                break;
        }
    }
}


void login_user(int &socket_file_descriptor)
{
    std::string login, password;
    cout << "Login: ";
    cin >> login;
    if (user_exists(login, socket_file_descriptor)) {
        uint* hash = new uint;
        for (int i=0; i<3; i++) {
            std::cout << "Password: ";
            SetStdinEcho(false);
            std::cin >> password;
            SetStdinEcho(true);
            cout << std::endl;
            if (password_is_correct(password, hash, socket_file_descriptor)) {
                std::cout << login << " successfully logged in\n";
                start_chat(login, hash, socket_file_descriptor);
                break;
            }
            cout << "Incorrect password\n";
        }
    }
    else {
      cout << "No such user\n";
    }
}


void show_main_menu_client(int& sock) {
    char key = '0';
    while (true) {
      cout << "\nPlease choose:\n\nl - Login\nq - Quit" << endl;
      cin >> key;

      switch (key) {
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
        login_user(sock);
        break;

      case 'q':
        hang_up(sock);
        exit(0);

      default:
        cout << "No such option" << endl;
        }
    }
}


void quit(MYSQL& mysql, const string& error_msg = "", 
          bool error = false)
{
    mysql_close(&mysql);
    if (error) {
        cout << error_msg << endl;
    }
    cout << "Bye!" << endl;
}


void show_main_menu_server(MYSQL &mysql, int &sock) {
    bool stop_thread = false;
    char key;
    while (true) {
        cout << endl <<
             "Please choose:\n"
             << "\na - Accept connections\nn - New user\nq - Quit"
             << endl;
        cin >> key;
        switch (key) {
            case 'a':
            {
                // TODO: Count how many connections are active
                accept_connections(sock, mysql, stop_thread);
                break;
            }

            case 'n': {
                create_new_user(mysql);
                break;
            }

            case 'q':
            {
                stop_thread = true;
                quit(mysql, "", false);
                close(sock);
                return;
            }

            default:
                cout << "No such option" << endl;
        }
    }
}
