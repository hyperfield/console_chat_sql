CREATE TABLE "users" (
  "id" SERIAL PRIMARY KEY,
  "name" varchar(20),
  "email" nvarchar(255) not null,
  "user_name" varchar(12) not null,
  "is_active" bit default 1::bit not null,
  "created_at" TIMESTAMPTZ DEFAULT Now()
);

CREATE TABLE "messages" (
  "id" SERIAL PRIMARY KEY,
  "sender_id" int,
  "receiver_id" int,
  "message" text(1000) not null,
  "read" bool,
  "delivered" bool,
  "created_at" TIMESTAMPTZ DEFAULT Now()
);

CREATE TABLE "msg_transport" (
  "id" SERIAL PRIMARY KEY,
  "message_id" int,
  "delivered_at" timestamp,
  "read_at" timestamp
);

CREATE TABLE "user_auth" (
  "id" SERIAL PRIMARY KEY,
  "user_id" int,
  "pwd_hash" varchar(40)
);

ALTER TABLE "messages" ADD FOREIGN KEY ("sender_id") REFERENCES "users" ("id");

ALTER TABLE "messages" ADD FOREIGN KEY ("receiver_id") REFERENCES "users" ("id");

ALTER TABLE "msg_transport" ADD FOREIGN KEY ("message_id") REFERENCES "messages" ("id");

ALTER TABLE "user_auth" ADD FOREIGN KEY ("user_id") REFERENCES "users" ("id");



CREATE TRIGGER add_user_hash 
AFTER INSERT ON users
FOR EACH ROW
EXECUTE FUNCTION add_hash();

CREATE OR REPLACE FUNCTION add_hash() RETURNS TRIGGER
AS $addhash$
	BEGIN
		insert into "user_auth"("user_id", "pwd_hash") values (new.id, null);
		return new;
	END;
$addhash$ LANGUAGE plpgsql;


-- CREATE TRIGGER delete_user
-- AFTER DELETE ON users
-- FOR EACH ROW
-- EXECUTE FUNCTION delete_user_messages();

-- CREATE OR REPLACE FUNCTION delete_user_messages() RETURNS TRIGGER
-- AS $delete_user_messages$
-- 	BEGIN
-- 		insert into "users"("created_at") values (new.id);
-- 		return new;
-- 	END;
-- $delete_user_messages$ LANGUAGE plpgsql;


-- select first_name, last_name, pwd_hash from users left join user_auth on users.id=user_auth.user_id;
-- select * from users u1 where exists (select * from users u2 where u1.email=u2.email and u1.id <> u2.id);
-- select * from messages where sender_id=1 and receiver_id=1;
-- select * from users where is_active=0::bit;
-- select * from messages where sender_id=receiver_id;
