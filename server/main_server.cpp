#include <future>
#include <future>
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
#include <boost/thread.hpp>

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


void init_connection(int& sock, struct sockaddr_in& client, socklen_t& length) {
    length = sizeof(client);
    struct sockaddr_in serveraddress;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        // return false;
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
        cout << "\nSocket binding failed\n";
        // return false;
    }
    else {
        cout << "\nSocket binding succeeded\n";
    }
    // Set the server to accept data
    int connection_status = listen(sock, 5);

    if (connection_status < 0) {
        cout << "Listen :: Connection error\n";
    }
}


void accept_connection_wrapper(int& sock, MYSQL& mysql, struct sockaddr_in& client, socklen_t& length)
{
    while (true) {
        struct sockaddr_in cli;
        socklen_t len;
        length = sizeof(cli);
        int connection = accept(sock, (struct sockaddr*) &cli, &len);
        if(connection == -1) {
            cout << "Accept :: Connection error\n";
        }
        thread t(take_commands, ref(connection), ref(sock), ref(mysql));
        t.detach();
        cout << "Launching take_commands\n";
        // take_commands(connection, sock, mysql);
    }
    //     else {
    //         std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    //     }
    // }
    close(sock);
    // close(connection);
}


int main() {
    MYSQL mysql;
    // MYSQL_ROW row;
    // MYSQL_RES *result;
    mysql_init(&mysql);
    connect_to_db(mysql);
    int sock;
    struct sockaddr_in client;
    socklen_t length;
    init_connection(sock, client, length);

    bool password_flag = false;
    char key;
    char message[msg_size];
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
                // TODO: Count how many connections are active
                thread t1(accept_connection_wrapper, ref(sock), ref(mysql), ref(client), ref(length));
                t1.detach();
                cout << "Enter 'q' to go back\n";
                char a_key;
                cin >> a_key;
                if (a_key == 'q') {
                    cout << "'q' pressed\n";
                }
                break;
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
                
                hang_up(sock);
                exit(0);
            }

            default:
            cout << "No such option" << endl;
        }
    }
    return 0;
}
