#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <mysql/mysql.h>
#include <thread>
#include "../headers/console.h"
#include "../headers/database.h"
#include "../headers/sha1.h"
#include "../headers/constants.h"
#include "../headers/network.h"

using namespace std;

bool stop_thread = false;


void accept_connection_wrapper(int& sock, MYSQL& mysql)
{
    while (!stop_thread) {
        struct sockaddr_in client;
        socklen_t length = sizeof(client);
        int connection = accept(sock, (struct sockaddr*) &client, &length);
        if (connection != -1 && !stop_thread) {
            thread t(take_commands, ref(connection), ref(sock), ref(mysql));
            t.detach();
        }
        else if (!stop_thread) {
            cout << CONNECTION_ACCEPT_ERR << endl;
        }
    }
}


void accept_connections(bool& run, int& sock, MYSQL& mysql)
{
    if (run) {
        thread t1(accept_connection_wrapper, ref(sock), ref(mysql));
        t1.detach();
        run = false;
    }
    cout << "Enter 'q' to go back\n";
    char a_key(' ');
    while (a_key != 'q') {
        cin >> a_key;
        if (a_key == 'q')
            break;
    }
}


void show_main_menu_server(MYSQL& mysql, int& sock) {
    bool run_accept_connection_wrapper = true;
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
                accept_connections(run_accept_connection_wrapper, sock, mysql);
                break;
            }

            case 'n': {
                create_new_user(mysql);
                break;
            }

            case 'q':
            {
                stop_thread = true;
                quit(mysql, sock, "", false);
                return;
            }

            default:
                cout << "No such option" << endl;
        }
    }
}


bool init(MYSQL& mysql, int& sock, struct sockaddr_in& client,
          socklen_t& length, User& user)
{
    // TODO: verify MYSQL init success
    mysql_init(&mysql);
    connect_to_db(mysql);
    init_connection(sock, client, length);
    if (sock == -1) {
        stop_thread = true;
        quit(mysql, sock, SOCKET_ERR_MSG, true);
        return true;
    }
    // Init default user for current session
    string password = "password";
    uint* hash = sha1(password.c_str(), password.length());
    new_user("all", hash, "all", "", user, mysql);
    return false;
}


int main() {
    MYSQL mysql;
    int sock = -1;
    struct sockaddr_in client;
    socklen_t length;
    User user;
    bool do_quit = init(mysql, sock, client, length, user);
    if (do_quit) {
        return -1;
    }
    show_main_menu_server(mysql, sock);
    return 0;
}
