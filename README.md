## A simple Console Chat with authentication and private messages

This is a simple Console Chat program, which has a client and a server side. New features and improvements are being added periodiclaly. The purpose of this program is mainly for the author to develop his C++ skills.

Chat functionality:

- client side and server side 
  (only authentication-related functions are currently supposted remotely, messages are passed locally)
- registration of users
- user password change (supported remotely)
- sending and viewing of common and private messages
- password hashing
- saving of users, their hashes and their messages in configuration files

The configuration files are kept in the directory

    ~/.config/console-chat

on Linux or MacOS X, where `~` is the shortcut to the user home directory. Or

    %UserDir%\.config\console-chat

on Windows, where `%UserDir%` is the user directory, such as `C:\Users\my_user\`.

Console Chat features:
- No password echoing in the terminal.
- Gives an error when trying to register an existing user.
- Gives a message that the user does not exist if trying to send a private message to a non-existing user.
- All chat users and messages are loaded from the configuration files when Console Chat is loaded.
- New chat users and messages are saved to configuration files when Console Chat is quit.

## Installation

To get your copy of the program from Github, make sure that `git` in installed on your system and do

    git clone https://github.com/hyperfield/console_chat.git
    cd console_chat

In the program catalog, go to either the `client` or the `server` directory (e.g., `cd client`) and do

    make
    make install

But if your OS is Windows, then `make install` is not needed.

## Developer:

Kontur (P.Z.)


### Console Chat &mdash the client part

When this program is launched, it tries to connect to the server counterpart at `127.0.0.1:7777`. If no connection can be established, the user is offered to enter a Console Chat server address or quit the Concole Chat client. When a connection is established, the user is offered to login or quit the program. (At this time it is only possible to create a new user on the server side). Upon successful authentication, the user can begin participating in the chat, change his/her password or log out. A user's session is maintained using the variables `login` and `hash`, which are passed by reference between functions.

### Console Chat &mdash the server part

When the program is launched, a menu screen appears. This screen offers to choose to accept connections, to create a new user, or to quit the server. The server side of *Console Chat* is responsible for client user authentication and it stores user data, and receives and stores user messages data.

Presently users and messages are stored in plain text, but passwords are hashed and are thus not stored, but only their hashes are stored. When the server is launched, it reads the corresponding user and message files and stores them in memory as data of type `vector`. When new users or messages are added to the vector, the corresponding data are saved in the corresponding files. The program also dumps the vectors to files when it quits.

A request to display the contents of the messages vector is sent when
- a new common chat message is sent,
- a new private message is sent,
- the key `m` is pressed to display the messages in this menu.

The messages are displayed by iterating the entire `messages` vector and printing the contents of the parameter `message` of each object of the class `Message`. This happens with the condition that the parameter `_for_user` of this object either corresponds to the user `all`, or to the `login` variable of the user currently logged in. The parameter `hash` naturally has to correspond to `login`.

## Possible future improvements

- Improvement of the networking capabilities of the program: e.g., by adding multi-threading.
- Improving network capability under Windows, possibly MacOS X.
- Saving the `users` vector to file when the program is exited only if at least one password was changed.
- Re-display not all messages, but only new messages when in the chat view.
- Display message date and time.
- Ability to change `name` of an object of the class `User`, i.e. the name of the user.
- Ability of a user to delete own account.
- Display not all of the messages but only a certain number of them, or only of a certain period.
- Introduction of text contrasts, such as justified use of bold font and different font colours.
- Addition of a frame around the chat text.
- Demarcation of sections for displaying private and common messages.
- Quoting of messages with replying.
- Introducton of graphical interface.


### Project purpose

This program was written as part of my endeavour to learn C++.