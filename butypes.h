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
#define RESOURCE_FILE_PREFIX "rep"
#define FILENAME_LENGTH 100
#define COMMAND_LENGTH 1000


#define DELIMITER ":"
#define DELIMITER_SEC ";"
#define DELIMITER_CMD "-"
#define DELIMITER_ARGS "/"  //used only for command list (input file)

#define MAX_CLIENTS 2
#define MAX_SERVERS 2

#define CLIENT_PORT_STARTER 5000
#define SERVER_PORT_STARTER 8000

#define LISTENER_INDEX 0
#define TALKER_INDEX 1

#define BUFSIZE 5000

#define MAX_COMMANDS 1000
#define MAX_INPUT_LENGTH 200

#define GET_NEXT_CMD_ID command_counter*MAX_CLIENTS+my_pid 
#define MYSERVER(my_pid) 0
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
	COMMAND_EDIT=2   //READ ONLY
};

struct COMMAND_ITEM
{
	int command_id;
	enum COMMAND_TYPE command_type;
	char command_data[BUFSIZE];
};

#endif
