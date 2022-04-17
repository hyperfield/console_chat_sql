/*
TO DO:
    - Gentrify process_connection_command().
    - case 'a': make quitting possible.
    - Make "Change user password" possible.
    - Make functions pure.
    - Database username and password need to be entered manually if not in config file (hashed).
*/

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

#include "../headers/user.h"
#include "../headers/message.h"
#include "../headers/passwordShadow.h"
#include "../headers/sha1.h"


using namespace std;

#define MESSAGE_LENGTH 1024
#define LOGIN_SIZE 32
#define PORT 7777


// Write an object to database
template <typename T>
const bool writeUserToDB(const T user_obj, MYSQL& mysql) {
    cout << "Adding user to DB" << endl;
    cout << "user_obj.getName(): " << user_obj.getName() << endl;
    cout << "user_obj.getEmail(): " << user_obj.getEmail() << endl;
    cout << "user_obj.getLogin(): " << user_obj.getLogin() << endl;
    string query = "SELECT create_user('";
    query += user_obj.getEmail();
    query += "', '";
    query += user_obj.getName();
    query += "', '";
    query += user_obj.getLogin();
    query += "', '";
    query += to_string(*user_obj.getHash());
    query += "')";
    cout << "Query: " << query << endl;
    int result = mysql_query(&mysql, query.c_str());
    cout << "Result: " << result << endl;

    return !result;
}


const int userExists(const string &login, MYSQL& mysql) {
    string query = "SELECT * from `users` WHERE `user_name`='";
    query += login + "'";
    mysql_query(&mysql, query.c_str());
    MYSQL_RES* res = mysql_store_result(&mysql);
    MYSQL_ROW row = mysql_fetch_row(res);
    if (res->row_count > 0) {
        int user_id = atoi(row[0]);
        return user_id;
    }
    return 0;
}


// Create a new user and place it into DB.
// Return true if successful, return false if user exists.
const bool newUser(const string &login, uint *hash, const string &name, const string &email,
                   User &user, MYSQL &mysql) {
    
    if (userExists(login, mysql)) {
        return false;
    }
    User newUser(login, hash, name, email);

    if (!writeUserToDB(newUser, mysql)) {
        return false;
    }

    user = newUser;
    return true;
}


// Find a User object by login and password hash and return the object.
User findUser(const string &login, uint* hash, MYSQL &mysql) {
    User empty_user;
    int user_id = userExists(login, mysql);
    if (user_id == 0) return empty_user;

    string found_user_query = "SELECT * FROM `users` WHERE `id`=";
    found_user_query += to_string(user_id);
    mysql_query(&mysql, found_user_query.c_str());
    MYSQL_RES* found_user_res = mysql_store_result(&mysql);
    MYSQL_ROW found_user_row = mysql_fetch_row(found_user_res);
    string query = "SELECT * from `user_auth` WHERE `user_id`=";
    query += to_string(user_id);
    mysql_query(&mysql, query.c_str());
    MYSQL_RES* res = mysql_store_result(&mysql);
    MYSQL_ROW row = mysql_fetch_row(res);
    uint init_hash = atoi(row[2]);
    uint* db_hash = new uint(init_hash);

    User found_user(found_user_row[1], db_hash, found_user_row[2], found_user_row[3]);
    if (*db_hash == *hash) {
        return found_user;
    }

    delete db_hash;
    return empty_user;
}


// Check if the user password is correct.
const bool checkPassword(const string &login, uint* &hash, MYSQL &mysql) {
    User check_pwd_user = findUser(login, hash, mysql);
    return (check_pwd_user.getLogin() == login && *check_pwd_user.getHash() == *hash);
}


// Type and confirm password when a new user is created.
const string setPassword(bool &passwordFlag) {
    int counter = 0;
    while (!passwordFlag && counter < 2) {
        string password, confirmPassword;
        cout << "Please enter new user password: ";
        SetStdinEcho(false);
        cin >> password;
        SetStdinEcho(true);
        cout << endl;
        cout << "Confirm new user password: ";
        SetStdinEcho(false);
        cin >> confirmPassword;
        SetStdinEcho(true);
        if (password == confirmPassword) {
            passwordFlag = true;
            return password;
        }
        counter++;
        cout << endl;
    }
    cout << "\nPassword mismatch\n";
    return "Password mismatch";
}


bool accept_connection(int& socket_file_descriptor, int& connection) {
    struct sockaddr_in serveraddress, client;
    socklen_t length;
    socket_file_descriptor = socket(AF_INET, SOCK_STREAM, 0); // Create a socket
    if (socket_file_descriptor == -1) {
        return false;
    }
    // Define server address
    serveraddress.sin_addr.s_addr = htonl(INADDR_ANY);
    // Define port number
    serveraddress.sin_port = htons(PORT);
    // Using IPv4
    serveraddress.sin_family = AF_INET;
    // Establish a connection with the server
    int bind_status = bind(socket_file_descriptor, (struct sockaddr*)&serveraddress, sizeof(serveraddress));
    if(bind_status == -1)  {
        cout << "Socket binding failed" << endl;
        return false;
    }
    // Set the server to accept data
    int connection_status = listen(socket_file_descriptor, 5);

    if (connection_status == -1) {
        return false;
    }

    connection = accept(socket_file_descriptor, (struct sockaddr*)&client, &length);
    if(connection == -1) {
        return false;
    }
    return true;
}


void process_connection_command(char command[], int connection, char param[], int param_size, MYSQL& mysql) {
    string login = param;
    if (!strcmp(command, "checkUserExists")) {
        bool user_exists = false;
        read(connection, param, param_size);
        if (userExists(param, mysql)) {
            user_exists = true;
        }
        else {
            cout << "No such user " << param << endl;
        }
        write(connection, (char*) &user_exists, sizeof(bool));
    }

    else if (!strcmp(command, "checkUserPassword")) {
        bool password_correct = false;
        char pass_param[32];
        read(connection, pass_param, 32);
        uint* int_pass_param = (uint*) pass_param;
        if (checkPassword(login, int_pass_param, mysql))  {
            cout << "User " << login << " was authenticated\n";
            password_correct = true;
        }
        else {
            cout << "User " << login << " was not authenticated\n";
        }
        write(connection, (char*) &password_correct, sizeof(bool));
    }

    else if (!strcmp(command, "changeUserPassword")) {
        bool password_changed = false;
        const size_t login_param_size = 32;
        const size_t hash_param_size = 32;
        char login_param[login_param_size],
             current_hash_param[hash_param_size],
             new_hash_param[hash_param_size];
        read(connection, login_param, login_param_size);
        read(connection, current_hash_param, hash_param_size);
        uint* int_old_hash_param = (uint*) current_hash_param;
        read(connection, new_hash_param, hash_param_size);
        uint* int_new_hash_param = (uint*) new_hash_param;
        User this_user = findUser(login, int_old_hash_param, mysql);
        if (this_user.getLogin() != login_param) {
            write(connection, (char*) &password_changed, sizeof(bool));
            cout << "Password not changed\n";
        }
        else if (this_user.changePassword(int_old_hash_param, int_new_hash_param)) {
            password_changed = true;
            write(connection, (char*) &password_changed, sizeof(bool));
            cout << "Password successfully changed\n";
        }
    }

    else if (!strcmp(command, "transmit_message")) {
        bool message_accepted = false;
        size_t message_size = 1024;
        size_t username_size = 32;
        char message[message_size],
             message_sender[username_size],
             message_receiver[username_size];
        read(connection, message, message_size);
        read(connection, message_sender, username_size);
        read(connection, message_receiver, username_size);
        // NEXT: writeMessageToDB() or smth
    }

    else
        cout << "No such command\n";
}


void init_db(MYSQL& mysql) {
    mysql_set_character_set(&mysql, "utf8");
    // cout << "connection characterset: " << mysql_character_set_name(&mysql) << endl;
    cout << "Successfully connected to MySQL database." << endl;
    mysql_query(&mysql, "CREATE DATABASE IF NOT EXISTS chat_db");
    mysql_query(&mysql, "USE chat_db");
    mysql_query(&mysql, "CREATE TABLE IF NOT EXISTS `users` ("
                        "`id` SERIAL PRIMARY KEY,"
                        "`name` varchar(20),"
                        "`email` nvarchar(255) not null unique,"
                        "`user_name` varchar(12) not null unique,"
                        "`is_active` bit default 1 not null,"
                        "`created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP )"
                );
    mysql_query(&mysql, "CREATE TABLE IF NOT EXISTS `messages` ("
                        "`id` SERIAL PRIMARY KEY,"
                        "`sender_id` BIGINT UNSIGNED,"
                        "FOREIGN KEY (`sender_id`) REFERENCES `users`(`id`),"
                        "`receiver_id` BIGINT UNSIGNED,"
                        "FOREIGN KEY (`receiver_id`) REFERENCES `users` (`id`),"
                        "`message` text(1000) not null,"
                        "`read` BOOL,"
                        "`delivered` BOOL,"
                        "`created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP )"
                );
    mysql_query(&mysql, "CREATE TABLE IF NOT EXISTS `msg_transport` ("
                        "`id` SERIAL PRIMARY KEY,"
                        "`message_id` BIGINT UNSIGNED,"
                        "FOREIGN KEY (`message_id`) REFERENCES `messages`(`id`),"
                        "`delivered_at` TIMESTAMP,"
                        "`read_at` TIMESTAMP )"
                );
    mysql_query(&mysql, "CREATE TABLE IF NOT EXISTS `user_auth` ("
                        "`id` SERIAL PRIMARY KEY,"
                        "`user_id` BIGINT UNSIGNED,"
                        "FOREIGN KEY (`user_id`) REFERENCES `users`(`id`),"
                        "`pwd_hash` varchar(40) )"
                );
    // mysql_query(&mysql, "CREATE TRIGGER `add_user_hash`"
    //                     "AFTER INSERT ON `users`"
    //                     "FOR EACH ROW "
    //                     "INSERT INTO `user_auth`(`user_id`, `pwd_hash`)"
    //                     "VALUES (new.id, null)"
    //             );

    mysql_query(&mysql, // "delimiter $$ "
                        "CREATE FUNCTION `create_user2`("
                        "`p_email` varchar(255),"
                        "`p_name` varchar(20),"
                        "`p_user_name`  varchar(12),"
                        "`p_hash` varchar(40)"
                        ")"
                        " returns int "
                        "deterministic "
                        "BEGIN "
                        "insert into `users`(`name`, `email`, `user_name`)"
                        "values (`p_name`, `p_email`, `p_user_name`)"
                        "INSERT INTO `user_auth` (`user_id`, `pwd_hash`)"
                        "VALUES(`LAST_INSERT_ID()`, `p_hash`)"
                        "return `LAST_INSERT_ID()`"
                        "end$$"
                        // " delimiter"
                );
}


int main() {
    MYSQL mysql;
    MYSQL_ROW row;
    MYSQL_RES* result;
    mysql_init(&mysql);

    if (!mysql_real_connect(&mysql, "localhost", "chat_root", "#Ch@7R00T!", "", 0, NULL, 0)) {
        cout << "Error: can't connect to MySQL due to the following error: " << mysql_error(&mysql) << endl;
    }
    else {
        init_db(mysql);
	}

    bool connection_success;

    User user; // User for current authenticated session

    string login, password, name, email;
    uint* hash;
    login = "all"; // To add the default login for the "all" user to the users vector
    password = "password"; // To add the default password for the "all" user
    hash = sha1(password.c_str(), password.length());
    name = "all"; // To add the default name for the "all" user
    bool passwordFlag = false;
    char key;
    char message[128], command[128], checkLogin[32];
    newUser(login, hash, name, email, user, mysql); // Adding the default "all" user

    while (true) {
        cout << "\nPlease choose:\n\na - Accept connections\nn - New user\nq - Quit" << endl;
        cin >> key;

        switch(key) {
            case 'a':
            {
                int socket_file_descriptor, connection;
                {
                    while (true) {
                        connection_success =
                            accept_connection(socket_file_descriptor, connection);
                        if (connection_success) {
                            cout << "Connection with a client established" << endl;
                            while (true) {
                                read(connection, command, sizeof(command));
                                cout << "Received command " << command << endl;
                                if (!strcmp(command, "HangUp")) {
                                    close(socket_file_descriptor);
                                    close(connection);
                                    cout << "Client has disconnected\n";
                                    break;
                                }
                                process_connection_command(command, connection, checkLogin, LOGIN_SIZE, mysql);
                            }
                            close(socket_file_descriptor);
                            close(connection);
                        }
                        else {
                            std::this_thread::sleep_for(std::chrono::milliseconds(3000));
                        }
                    }
                }
            }
            
            case 'n':
            {
                cout << "Please enter new user login: ";
                cin >> login;
                password = setPassword(passwordFlag);
                uint* hash = sha1(password.c_str(), password.length());
                if (!passwordFlag) break;
                passwordFlag = false;
                cout << "\nPlease enter new user name: ";
                cin >> name;
                cout << "\nPlease enter user e-mail: ";
                cin >> email;
                if (!newUser(login, hash, name, email, user, mysql)) {
                    cout << "This login already exists!\n";
                }
                break;
            }

            case 'q': 
            { 
                mysql_close(&mysql);
                exit(0);
            }

            default:
                cout << "No such option" << endl;
        }
    }
    return 0;
}
