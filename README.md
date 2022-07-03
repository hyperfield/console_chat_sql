## A simple client / server chat with MySQL, authentication and private messages

### Qt GUI coming soon.

This is a simple console chat program, which has its client and server sides. New features and improvements are being added periodiclaly. The purpose of this program is mainly for the author to improve his C++ skills.

Features:
    - Network sockets
    - Multithreading
    - Storage of user and message data in SQL
    - Authentication of clients via a central server
    - Password hashing
    - Common and private messages


To get your copy of the program from Github, make sure that `git` in installed on your system and do

    git clone https://github.com/hyperfield/console_chat_sql.git
    cd console_chat_sql
## Installation - Client side
### Prerequisites

    g++ (GNU C++ compiler)
    make (GNU utility to maintain groups of programs)

In the program catalog, go to the `client` directory (e.g., `cd client`) and do

    make
    sudo make install
    make clean

But if you are on Windows, then `make install` is not needed.
## Installation - Server side
### Prerequisites

    g++ (GNU C++ compiler)
    make (GNU utility to maintain groups of programs)
    MySQL

In the program catalog, go to the `server` directory (e.g., `cd client`) and do

    chmod +x install.sh
    ./install.sh
    make
    sudo make install
    make clean

But if your OS is Windows, then `make install` is not needed. Also, you will need to use a Bash shell on Windows to run the `install.sh` script.

## Developer:

Contour (P.Z.)


### Console Chat &mdash the client part

When this program is launched, it tries to connect to the server counterpart at `127.0.0.1:7777`. If no connection can be established, the user is offered to enter a Console Chat server address or quit the Concole Chat client. When a connection is established, the user is offered to login or quit the program. (At this time it is only possible to create a new user on the server side). Upon successful authentication, the user can begin participating in the chat, change his/her password or log out.

### Console Chat &mdash the server part

When the program is launched, a menu screen appears. This screen offers to choose to accept connections, to create a new user, or to quit the server. The server side of *Console Chat* is responsible for client user authentication and it stores user data, and receives and stores user messages data.

A request to display the contents of the messages vector is sent when
- a new common chat message is sent,
- a new private message is sent,
- the key `m` is pressed to display the messages in this menu.

## Possible future improvements

- Qt GUI version.
- ncurses version.
- Tested port for Windows.
- Re-display not all messages, but only new messages when in the chat view.
- Display message date and time.
- Ability to change other user info besides password.
- Ability of a user to delete own account.
- Display not all of the messages but only a certain number of them, or only of a certain period.
- Introduction of text contrasts, such as justified use of bold font and different font colors.
- Quoting of messages with replying.
