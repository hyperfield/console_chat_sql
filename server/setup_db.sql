CREATE TABLE IF NOT EXISTS `users` (`id` SERIAL PRIMARY KEY,`name` varchar(20),
                           `email` nvarchar(255) not null unique,
                           `user_name` varchar(12) not null unique,
                           `is_active` bit default 1 not null,
                           `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                        );


CREATE TABLE IF NOT EXISTS `messages` (
                           `id` SERIAL PRIMARY KEY,
                           `sender_id` BIGINT UNSIGNED,
                            FOREIGN KEY (`sender_id`) REFERENCES `users`(`id`),
                           `receiver_id` BIGINT UNSIGNED,
                            FOREIGN KEY (`receiver_id`) REFERENCES `users` (`id`),
                           `message` text(1000) not null,
                           `read` BOOL,
                           `delivered` BOOL,
                           `created_at` TIMESTAMP DEFAULT CURRENT_TIMESTAMP
                        );


CREATE TABLE IF NOT EXISTS `msg_transport` (
                           `id` SERIAL PRIMARY KEY,
                           `message_id` BIGINT UNSIGNED,
                            FOREIGN KEY (`message_id`) REFERENCES `messages`(`id`),
                           `delivered_at` TIMESTAMP,
                           `read_at` TIMESTAMP
                        );


CREATE TABLE IF NOT EXISTS `user_auth` (
                           `id` SERIAL PRIMARY KEY,
                           `user_id` BIGINT UNSIGNED,
                            FOREIGN KEY (`user_id`) REFERENCES `users`(`id`),
                           `pwd_hash` varchar(40)
                        );


DROP FUNCTION IF EXISTS create_user;

delimiter $$
CREATE FUNCTION create_user(
	  p_email      varchar(255),
	  p_name	   varchar(20),
	  p_user_name  varchar(12),
	  p_hash       varchar(40)
)
returns int
deterministic

BEGIN

	insert into users (name, email, user_name)
	values (p_name, p_email, p_user_name);
	
	INSERT INTO user_auth (user_id, pwd_hash)
	VALUES(LAST_INSERT_ID(), p_hash);

return LAST_INSERT_ID();

END$$


DELIMITER //
CREATE TRIGGER IF NOT EXISTS after_messages_insert
AFTER INSERT
ON messages FOR EACH ROW
BEGIN
    INSERT INTO msg_transport (message_id)
    VALUES (new.id);
END//
