#include <mysql/mysql.h>

#include "../headers/user.h"
#include "../headers/message.h"

using std::vector;
using std::to_string;


const bool write_user_to_db(const User &user_obj, MYSQL &mysql) {
  std::string query = "SELECT create_user('";
  query += user_obj.getEmail();
  query += "', '";
  query += user_obj.getName();
  query += "', '";
  query += user_obj.getLogin();
  query += "', '";
  query += std::to_string(*user_obj.getHash());
  query += "')";
  int result = mysql_query(&mysql, query.c_str());
  return !result;
}

const int user_exists(const std::string &login, MYSQL& mysql) {
    std::string query = "SELECT * from `users` WHERE `user_name`='";
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


const bool update_user_pwdhash_in_DB(const User& user_obj, MYSQL& mysql) {
    int user_id = user_exists(user_obj.getLogin(), mysql);
    std::string query = "UPDATE user_auth SET `pwd_hash`=";
    query += std::to_string(*user_obj.getHash());
    query += " WHERE `user_id`=";
    query += std::to_string(user_id);
    int result = mysql_query(&mysql, query.c_str());
    return !result;
}


const std::string get_user_by_id(int user_id, MYSQL& mysql) {
    std::string user_name;
    std::string query = "SELECT `user_name` FROM users "
                   "WHERE `id`=";
    query += std::to_string(user_id);
    mysql_query(&mysql, query.c_str());
    MYSQL_RES* res = mysql_store_result(&mysql);
    MYSQL_ROW row = mysql_fetch_row(res);
    if (res->row_count > 0) {
        user_name = row[0];
    }
    return user_name;
}


const vector<Message> read_messages_from_db(int how_many, char login[],
                                            MYSQL &mysql)
{
  std::vector<Message> messages;
  std::string receiver;
  int receiver_id = user_exists(login, mysql);
  int receiver_id_all = user_exists("all", mysql);
  std::string query = "SELECT q.`message`, q.`sender_id`, q.`receiver_id` "
                 "FROM (SELECT `id`, `message`, `sender_id`, `receiver_id` "
                 "FROM messages WHERE (`receiver_id`=";
  query += to_string(receiver_id_all);
  query += " OR `receiver_id`=";
  query += to_string(receiver_id);
  query += ") ORDER BY id DESC LIMIT ";
  query += to_string(how_many);
  query += ") q ORDER BY q.`id` ASC";
  mysql_query(&mysql, query.c_str());
  MYSQL_RES *res = mysql_store_result(&mysql);
  while (MYSQL_ROW row = mysql_fetch_row(res)) {
    int sender_id = atoi(row[1]);
    int receiver_id = atoi(row[2]);
    std::string sender = get_user_by_id(sender_id, mysql);
    if (receiver_id == receiver_id_all)
      receiver = "all";
    else
      receiver = login;
    std::string message = row[0];
    Message message_obj(message, sender, receiver);
    messages.push_back(message_obj);
  }
  return messages;
}


const bool write_message_to_db(Message& message_obj, MYSQL& mysql) {
    std::string query = "INSERT INTO messages("
                   "`sender_id`, `receiver_id`, `message`"
                   ") VALUES('";
    int author_id, forwhom_id;
    author_id = user_exists(message_obj.getAuthor(), mysql);
    forwhom_id = user_exists(message_obj.forWhom(), mysql);
    query += to_string(author_id);
    query += "', '";
    query += to_string(forwhom_id);
    query += "', '";
    query += message_obj.getMessage();
    query += "')";
    int result = mysql_query(&mysql, query.c_str());
    return !result;
}


// Create a new user and place it into DB.
// Return true if successful, return false if user is already in DB.
const bool new_user(const std::string &login, uint *hash, const std::string &name,
                   const std::string &email, User &user, MYSQL &mysql)
{
    if (user_exists(login, mysql)) {
        return false;
    }
    User new_user(login, hash, name, email);
    // This condition may not be redundant
    // if by some criterion user actually exists in DB
    if (!write_user_to_db(new_user, mysql)) {
        return false;
    }

    user = new_user;
    return true;
}


// Find a User object by login and password hash and return the found object
User find_user(const std::string &login, uint* hash, MYSQL &mysql) {
    User default_user;
    int user_id = user_exists(login, mysql);
    if (user_id == 0) return default_user;
    std::string found_user_query = "SELECT * FROM `users` WHERE `id`=";
    found_user_query += to_string(user_id);
    mysql_query(&mysql, found_user_query.c_str());
    MYSQL_RES* found_user_res = mysql_store_result(&mysql);
    MYSQL_ROW found_user_row = mysql_fetch_row(found_user_res);
    std::string query = "SELECT * from `user_auth` WHERE `user_id`=";
    query += to_string(user_id);
    mysql_query(&mysql, query.c_str());
    MYSQL_RES* res = mysql_store_result(&mysql);
    MYSQL_ROW row = mysql_fetch_row(res);
    uint init_hash = atoi(row[2]);
    uint* db_hash = new uint(init_hash);
    User found_user(found_user_row[3], db_hash, found_user_row[1], found_user_row[2]);
    if (*db_hash == *hash) {
        return found_user;
    }
    delete db_hash;
    return default_user;
}


const bool check_password(const std::string &login, uint* &hash, MYSQL &mysql) {
    User check_pwd_user = find_user(login, hash, mysql);
    return (check_pwd_user.getLogin() == login && *check_pwd_user.getHash() == *hash);
}