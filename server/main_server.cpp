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


bool init(MYSQL& mysql, int& sock, struct sockaddr_in& client,
          socklen_t& length, User& user)
{
    // TODO: verify MYSQL init success
    mysql_init(&mysql);
    connect_to_db(mysql);
    init_connection(sock, client, length);
    if (sock == -1) {
        quit(mysql, SOCKET_ERR_MSG, true);
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
