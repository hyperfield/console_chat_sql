#include <mysql/mysql.h>
#include <string>

void accept_connections(bool& run, int& sock, MYSQL& mysql);
void accept_connection_wrapper(int& sock, MYSQL& mysql);
