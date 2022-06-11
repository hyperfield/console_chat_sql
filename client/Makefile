chat: commandslib databaselib userlib networklib messagelib passwordlib sha1lib consolelib
	g++ -o chat_client main_client.cpp -L. -lconsole -lnetwork -lmessage -luser -lpasswordShadow -lcommands -ldatabase -lsha1 `mysql_config --cflags --libs` -std=c++17

commandslib: ../server/commands.cpp ../headers/commands.h
	g++ -c ../server/commands.cpp -o commands.o -std=c++17
	ar rc libcommands.a commands.o

consolelib: ../console.cpp ../headers/console.h
	g++ -c ../console.cpp -o console.o -std=c++17
	ar rc libconsole.a console.o

databaselib: ../database.cpp ../headers/database.h
	g++ -c ../database.cpp -o database.o -std=c++17
	ar rc libdatabase.a database.o

userlib: ../user.cpp ../headers/user.h
	g++ -c ../user.cpp -o user.o -std=c++17
	ar rc libuser.a user.o

messagelib: ../message.cpp ../headers/message.h
	g++ -c ../message.cpp -o message.o -std=c++17
	ar rc libmessage.a message.o

networklib: ../network.cpp ../headers/network.h
	g++ -c ../network.cpp -o network.o -std=c++17
	ar rc libnetwork.a network.o

passwordlib: ../passwordShadow.cpp ../headers/passwordShadow.h
	g++ -o passwordShadow.o ../passwordShadow.cpp -c -std=c++17
	ar rc libpasswordShadow.a passwordShadow.o

sha1lib: ../sha1.cpp ../headers/sha1.h
	g++ -o sha1.o ../sha1.cpp -c -std=c++17
	ar rc libsha1.a sha1.o

clean:
	rm *.o *.a

install:
	mv ./chat_client /usr/local/bin/

uninstall:
	rm /usr/local/bin/chat_client
