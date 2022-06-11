#include <mysql/mysql.h>
#include "message.h"
#include "user.h"


bool write_user_to_db(const User& user_obj, MYSQL& mysql);
int user_exists(const std::string &login, MYSQL& mysql);
bool update_user_pwdhash_in_DB(const User& user_obj, MYSQL& mysql);
const std::string get_user_by_id(int user_id, MYSQL& mysql);
const std::vector<Message> read_messages_from_db(int how_many, char login[],
                                            MYSQL &mysql);
bool write_message_to_db(const Message& message_obj, MYSQL& mysql);
bool new_user(const std::string &login, uint *hash, const std::string &name,
                    const std::string &email, User &user, MYSQL &mysql);
User find_user(const std::string &login, uint* hash, MYSQL &mysql);
bool check_password(const std::string &login, uint* &hash, MYSQL &mysql);
void connect_to_db(MYSQL &mysql);
void create_new_user(MYSQL& mysql);
