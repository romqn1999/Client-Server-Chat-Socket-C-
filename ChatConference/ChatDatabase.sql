create database ChatMulti
go

use ChatMulti
go

create table [USER]
(
	username nvarchar(50) primary key,
	password nvarchar(51)
);
GO

create table MESSAGE
(
	Times int IDENTITY(1,1) PRIMARY KEY,
	Username nvarchar(50),
	Mess nvarchar(1000),
	To_user nvarchar(50)
)
GO

create table ONLINE
(
	Username nvarchar(50) primary key
)
GO

create table [FILE]
(
	ID int PRIMARY KEY,
	NAME nvarchar(50),
	FROMUSER nvarchar(50),
	TOWHOM nvarchar(50)
)

Insert into [USER] values('Server', '')