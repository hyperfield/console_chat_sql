#pragma once
#include "message.h"
#include "user.h"
#include <mysql/mysql.h>


const bool write_user_to_db(const User& user_obj, MYSQL& mysql);
const int user_exists(const std::string &login, MYSQL& mysql);
const bool update_user_pwdhash_in_DB(const User& user_obj, MYSQL& mysql);
const std::string get_user_by_id(int user_id, MYSQL& mysql);
const std::vector<Message> read_messages_from_db(int how_many, char login[],
                                            MYSQL &mysql);
const bool write_message_to_db(Message& message_obj, MYSQL& mysql);
const bool new_user(const std::string &login, uint *hash, const std::string &name,
                    const std::string &email, User &user, MYSQL &mysql);
User find_user(const std::string &login, uint* hash, MYSQL &mysql);
const bool check_password(const std::string &login, uint* &hash, MYSQL &mysql);
