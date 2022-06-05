#include <filesystem>
#include <iostream>

#include "../headers/console.h"


using std::cout;
using std::string;
namespace fs = std::filesystem;


void create_config() {
    string home = getenv("HOME");
    string config = "/.config/console-chat";
    string home_config = home + config;
    fs::create_directory(home_config);
}


int main() {
    create_config();
    bool connection_success = false;
    int socket_file_descriptor, connection;
    char connection_ip[] = "127.0.0.1";

    connection_wrapper(socket_file_descriptor, connection_ip, connection, connection_success);
    if (connection_success) {
        cout << "Connection with server " << connection_ip << " has been established.\n";
    }
    show_main_menu(socket_file_descriptor);
}
