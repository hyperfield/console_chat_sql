chat: userlib messagelib passwordlib sha1lib
	g++ -o chat_client main_client.cpp -L. -lmessage -luser -lpasswordShadow -lsha1 -std=c++17

userlib: ../user.cpp ../headers/user.h
	g++ -c ../user.cpp -o user.o -std=c++17
	ar rc libuser.a user.o

messagelib: ../message.cpp ../headers/message.h
	g++ -c ../message.cpp -o message.o -std=c++17
	ar rc libmessage.a message.o
	
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
