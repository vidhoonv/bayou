#ifndef BUTYPES_H
#define BUTYPES_H

#include<stdio.h>
#include<pthread.h>
#include <sys/socket.h> 
#include <netdb.h> 
#include <netinet/in.h>
#include <arpa/inet.h> 
#include <stdio.h> 
#include <unistd.h> 
#include <string.h> 
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>
#include<stdlib.h>
#include <time.h>
#include<signal.h>


#define DEBUG 1

#define COMMAND_FILE_PREFIX "command_list_"
#define LOG_FILE_PREFIX "log_"
#define SLOG_FILE_PREFIX "stable_log_"
#define CLOG_FILE_PREFIX "create_log_"
#define RLOG_FILE_PREFIX "retire_log_"
#define RESOURCE_FILE_PREFIX "db"
#define FILENAME_LENGTH 100
#define COMMAND_LENGTH 1000


#define DELIMITER ":"
#define DELIMITER_SEC ";"
#define DELIMITER_TER "|"
#define DELIMITER_QUAT "~"
#define DELIMITER_CMD "-"
#define DELIMITER_ARGS "/"  //used only for command list (input file)

#define MAX_CLIENTS 2
#define MAX_SERVERS 6
#define MAX_LEVELS 100
#define CLIENT_PORT_STARTER 5000
#define SERVER_PORT_STARTER 8000

#define LISTENER_INDEX 0
#define TALKER_INDEX 1

#define BUFSIZE 5000

#define MAX_COMMANDS 1000
#define MAX_INPUT_LENGTH 200
#define INFINITY 999999999

#define GET_NEXT_CMD_ID command_counter*MAX_CLIENTS+my_pid 
#define MYSERVER(my_pid) my_pid
struct COMM_DATA
{

	//index 0 listener
	//index 1 talker
	int comm_port[2];
	int comm_fd[2];
};

enum COMMAND_TYPE
{
	COMMAND_ADD=0,
	COMMAND_DELETE=1,
	COMMAND_EDIT=2
};
struct COMMAND_ITEM
{
	int command_id;
	int timestamp;
	enum COMMAND_TYPE command_type;
	char command_data[BUFSIZE];
};
enum LOG_CMD
{
	LOG_ADD=0,
	LOG_INSERT=1,
	LOG_FETCH=2,
	LOG_DELETE=3
};
struct SERVER_ID {
	int level;
	int id[MAX_LEVELS];
};
struct VERSION_VECTOR {
	int server_count;
	int csn;
	struct SERVER_ID servers[MAX_SERVERS];
	int recent_timestamp[MAX_SERVERS];

};
struct ENTROPY_INPUT{
	struct COMM_DATA *server_comm;
	int *sender_list;
	int my_pid;
};
#endif
