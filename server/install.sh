#!/bin/bash

echo "Welcome to the Console Chat server setup script!";
echo "This script will setup a MySQL database necessary for the chat server to operate.";
echo "You thus need to have MySQL server installed in your system.";
# TODO (maybe): check if MySQL is installed.
echo "Depending on your sudo timeout, you may now be prompted for your sudo password to effect MySQL processes.";
echo "Then MySQL will also ask you for your MySQL root password."
# TODO: check if password is correct.
# echo -n "Please choose a password for the Console Chat database user console_chat: ";
sudo mysql -u root -p -Bse "CREATE USER IF NOT EXISTS 'chat_root' IDENTIFIED BY '#Ch@7R00T!';
                            CREATE DATABASE IF NOT EXISTS chat_db;
                            USE chat_db; GRANT ALL ON chat_db.* TO 'chat_root'";
# TODO: Check success result.
echo "Created database user with username chat_root ...";
echo "Created database chat_db, set up priveleges for chat_root ..."
echo "Setting up the tables for chat_db ...";
sudo mysql -h localhost -u root -p "chat_db" < "setup_db.sql"
