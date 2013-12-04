/*
CS 380D Distributing Computing

BAYOU protocol
==============

Authors:
@Vidhoon Viswanathan
@Layamrudhaa RV

*/
#include "butypes.h"

#define TALKER client_comm.comm_fd[TALKER_INDEX]
#define LISTENER client_comm.comm_fd[LISTENER_INDEX]

#define CMD_DATA_PREP(OP1,OP2,OP3,OP4,STR) strcpy(STR,"");	\
					strcat(STR,OP1);	\
					strcat(STR,DELIMITER_CMD);	\
					strcat(STR,OP2);	\
					strcat(STR,DELIMITER_CMD); \
					if(cmd_type == 2)	\
					{	\
						strcat(STR,OP3);	\
						strcat(STR,DELIMITER_CMD);	\
						strcat(STR,OP4);	\
						strcat(STR,DELIMITER_CMD); \
					}	\

struct playlist
{
    int cmds[MAX_COMMANDS];
    char song_name[BUFSIZE];
    char song_url[BUFSIZE];
};

struct playlist client_playlist[MAX_COMMANDS];
struct CMD_ITEM cmd_list[MAX_COMMANDS];

int do_command(int command_id)
{
	
	int i=0;
	size_t len;
        char *data;
	
	char sn[MAX_INPUT_LENGTH],su[MAX_INPUT_LENGTH];
	char new_sn[MAX_INPUT_LENGTH],new_su[MAX_INPUT_LENGTH];
	char record[2*MAX_INPUT_LENGTH];
	int  op_arg;
	bool fail=false;
        
        int cmd_type = cmd_list[command_id].command_type;
        
        

	//printf("while doing command: acc:%s arg:%d\n",acc_name,op_arg);
        //do commands on tentative data
	switch(cmd_type)
	{
		case COMMAND_ADD:
                  
                    for(i=0;i<MAX_COMMANDS;i++)
                    {
                        if(strcmp("",client_playlist[i].song_name) == 0 && strcmp("",client_playlist[i].song_url) == 0)
                        {
                            //found entry
                            strcpy(client_playlist[i].song_name,cmd_list[command_id].arg1);
                            strcpy(client_playlist[i].song_url,cmd_list[command_id].arg2);
                            break;
                        }
                    }
					
                                        break;
		case COMMAND_DELETE:
                    for(i=0;i<MAX_COMMANDS;i++)
                    {
                        if(strcmp(cmd_list[command_id].arg1,client_playlist[i].song_name) == 0 && strcmp(cmd_list[command_id].arg2,client_playlist[i].song_url) == 0)
                        {
                            //found entry
                            strcpy(client_playlist[i].song_name,"");
                            strcpy(client_playlist[i].song_url,"");
                            break;
                        }
                    }
									
                                        break;
		case COMMAND_EDIT:
                    
                     for(i=0;i<MAX_COMMANDS;i++)
                    {
                        if(strcmp(cmd_list[command_id].arg1,client_playlist[i].song_name) == 0 && strcmp(cmd_list[command_id].arg1,client_playlist[i].song_url) == 0)
                        {
                            //found entry
                            strcpy(client_playlist[i].song_name,cmd_list[command_id].arg3);
                            strcpy(client_playlist[i].song_url,cmd_list[command_id].arg4);
                            break;
                        }
                    }
                     if(i==MAX_COMMANDS)
                     {
                         //could not find entry in local playlist
                         //ADD new entry to playlist
                         
                        for(i=0;i<MAX_COMMANDS;i++)
                         {
                                if(strcmp("",client_playlist[i].song_name) == 0 && strcmp("",client_playlist[i].song_url) == 0)
                                {
                                //found entry
                                     strcpy(client_playlist[i].song_name,cmd_list[command_id].arg3);
                                    strcpy(client_playlist[i].song_url,cmd_list[command_id].arg4);
                                         break;
                                }
                        }
                     }
					
                                        break;
		default:
				printf("!!!some invalid command\n");
				return -1;

	}	
return 0;

}

bool session_manager(int command_id)
{
    do_command(command_id);
}
int  get_next_command(int my_pid,int cmd_cnt, int *cmd_type,char *song_name, char *song_url,char *new_name, char *new_url)
{
	FILE *fp;
	char filename[FILENAME_LENGTH];
	int i=0;
	size_t len;
	char *line=NULL;
	ssize_t read;
	char *tok;
	char *data,*op_args;
	
	
	strcpy(filename,COMMAND_FILE_PREFIX);
	sprintf(filename,"%s%d",filename,my_pid);

	fp = fopen(filename,"r");
	if(fp == NULL)
	{
		printf("file could not be accessed\n");
		return -1;
	}		

	while(( read = getline(&line,&len,fp)) != -1)
	{
		
		if(i==cmd_cnt)
			break;
		i++;
	}
	if(read == -1)
		return -1;
	if(line[strlen(line)-1] == '\n')
				line[strlen(line)-1]='\0';

	data =  strtok_r(line,DELIMITER_CMD,&tok);

	if(strcmp(data,"COMMAND_ADD") == 0)
	{
		*cmd_type = 0;
	}
	else if(strcmp(data,"COMMAND_DELETE") == 0)
	{
		*cmd_type = 1;
	}
	else if(strcmp(data,"COMMAND_EDIT") == 0)
	{
		*cmd_type = 2;
	}
        else if(strcmp(data,"COMMAND_READ") == 0)
	{
		*cmd_type = 3;
	}
	op_args = strtok_r(NULL,DELIMITER_CMD,&tok);

	if(op_args)
	{
		strcpy(song_name,strtok_r(op_args,DELIMITER_ARGS,&tok));
                if(*cmd_type == 3)
                {
                    //do nothing
                }
                else
                {
                        strcpy(song_url,strtok_r(NULL,DELIMITER_ARGS,&tok));
                }
		if(*cmd_type == 2)
		{
			strcpy(new_name,strtok_r(NULL,DELIMITER_ARGS,&tok));
			strcpy(new_url,strtok_r(NULL,DELIMITER_ARGS,&tok));
		}
	}
	else
	{
		printf("error getting next command\n");
		return -1;
	}
	return 0;
}
bool configure_client(int my_pid,struct COMM_DATA *comm_client)
{
	
	int listener_fd,port,talker_fd;
	socklen_t listener_len;
	int yes=1;
	int i;
	struct sockaddr_in talker_addr, listener_addr, *process_addr_in;
	//listen setup
	if ((listener_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
		perror("listener socket ");
        	return false;
    	}

	if (setsockopt(listener_fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
		perror("setsockopt");
		return false;
	}
    	listener_addr.sin_family = AF_INET;
   	listener_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	listener_addr.sin_port = htons(CLIENT_PORT_STARTER+my_pid); 

	if (bind(listener_fd, (struct sockaddr *) &listener_addr, sizeof(listener_addr)) < 0)
	{
	        perror("listener bind ");
	        close(listener_fd);
	        return false;
	}
    	listener_len = sizeof(listener_addr);

	if (getsockname(listener_fd, (struct sockaddr *)&listener_addr, &listener_len) < 0)
    	{
        	perror("listener getsockname ");
        	close(listener_fd);
        	return false;
    	}
    	printf("listener using port %d\n", ntohs(listener_addr.sin_port));

	//talker setup
	if (( talker_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0){
			perror("talker socket ");
       			return false;
	}
	printf("talker_fd = %d\n",talker_fd);

	talker_addr.sin_family = AF_INET;
	talker_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    	talker_addr.sin_port = htons(0);  // pick any free port

	if (bind(talker_fd, (struct sockaddr *) &talker_addr, sizeof(talker_addr)) < 0)
    	{
        perror("talker bind ");
        close(talker_fd);
        return false;
    	}   

	comm_client->comm_fd[LISTENER_INDEX] = listener_fd;
	comm_client->comm_fd[TALKER_INDEX] = talker_fd;

return true;
}

bool remove_tentative_entry(int command_id)
{
    int i;
    
    if(cmd_list[command_id].command_type == COMMAND_EDIT)
    {
      for(i=0;i<MAX_COMMANDS;i++)
                    {
                        if(strcmp(cmd_list[command_id].arg3,client_playlist[i].song_name) == 0 && strcmp(cmd_list[command_id].arg4,client_playlist[i].song_url) == 0)
                        {
                            //found entry
                            strcpy(client_playlist[i].song_name,cmd_list[command_id].arg1);
                            strcpy(client_playlist[i].song_url,cmd_list[command_id].arg2);
                            break;
                        }
                    }
    }
}

void* listener(void *arg)
{
//comm listening variables
	fd_set readfds;
	int maxfd;
	char recv_buff[BUFSIZE];
	int nread=0;

//comm common
	struct sockaddr_storage temp_paddr;
	socklen_t temp_paddr_len;

//misc
	int i,ret=0,recv_pid;
	char buff_copy[BUFSIZE];
	char *data,*tok,*res;
	int recv_cmd_id;
	struct COMM_DATA *client_comm = (struct COMM_DATA *)arg;
        int cmd_res[MAX_COMMANDS];
        int rc = -1;
        
        for(i=0;i<MAX_COMMANDS;i++)
            cmd_res[i] = -1;
        
	printf("listening for msgs \n");
	while(1)
	{
	
		maxfd = client_comm->comm_fd[LISTENER_INDEX]+1;
		FD_ZERO(&readfds); 
		FD_SET(client_comm->comm_fd[LISTENER_INDEX], &readfds);

		ret = select(maxfd, &readfds, NULL, NULL, NULL);  //blocks forever till it receives a message


		if(ret <0)
	   	{ 
	     		printf("\nSelect error\n");   
	     		return NULL;
	   	} 

		if(FD_ISSET (client_comm->comm_fd[LISTENER_INDEX], &readfds))
		{
			temp_paddr_len = sizeof(temp_paddr);
			nread = recvfrom (client_comm->comm_fd[LISTENER_INDEX], recv_buff, BUFSIZE, 0, 
               	       			(struct sockaddr *)&temp_paddr, &temp_paddr_len); 
                        
		 	if (nread < 0)
		       	{
		        	perror("recvfrom ");
            			close(client_comm->comm_fd[LISTENER_INDEX]);
            			return NULL;
        		}		
			recv_buff[nread] = 0;
  			//printf("received: %s\n", recv_buff);

			strcpy(buff_copy,recv_buff);			
			data = strtok_r(buff_copy,DELIMITER,&tok);

			recv_pid = atoi(strtok_r(NULL,DELIMITER,&tok));
//retrive recv_pid
			recv_cmd_id = atoi(strtok_r(NULL,DELIMITER,&tok));

			res = strtok_r(NULL,DELIMITER,&tok);
                        if(cmd_res[recv_cmd_id] == -1 && strcmp(res,"SS") == 0)
                        {
        			printf("recved msg from server content:%s for command %d  res:%s\n",data,recv_cmd_id,res);
                                cmd_res[recv_cmd_id] = 1;
			}
                                //invoke session manager for tracking
                                if(strcmp(res,"TS") == 0 || strcmp(res,"SS") == 0)
                                {
                                   // printf("received  success %s\n",res);
                                    session_manager(recv_cmd_id);
                                }
                                else if(strcmp(res,"SF") == 0)
                                {
                                    //update client local playlist
                                    remove_tentative_entry(recv_cmd_id);
                                }
                                else
                                {
                                    //printf("received something else %s%d",res,strcmp(res,"TS"));
                                }
		}
		
	}
	
}
bool check_tentative(int command_id,char *url)
{
    int i;
    for(i=0;i<MAX_COMMANDS;i++)
    {  // printf("%s\n",client_playlist[i].song_name);
        if(strcmp(cmd_list[command_id].arg1, client_playlist[i].song_name)==0)
        {
            strcpy(url, client_playlist[i].song_url);
            return true;
        }
    }
    
    return false;
    
}
int main(int argc, char **argv)
{

//comm related variables
	struct hostent *hp;
	char hostname[64];
	struct COMM_DATA client_comm;
        
	struct sockaddr_in *server_addr_in[MAX_SERVERS];
	struct sockaddr server_addr[MAX_SERVERS];
	socklen_t server_addr_len[MAX_SERVERS];

//comm buffers
	char send_buff[BUFSIZE];
	char cmd_str[BUFSIZE];

//personal identification related
	int my_pid = 0;

//counters
	int command_counter=0;
//input
	struct COMMAND_ITEM command; //might be needed to store all commands for session guarantee
	char song_name[MAX_INPUT_LENGTH];
	char song_url[MAX_INPUT_LENGTH];
	char new_name[MAX_INPUT_LENGTH];
	char new_url[MAX_INPUT_LENGTH];
	int cmd_type=-1;

//thread related
	pthread_t listener_thread;
	
//misc
	int i=0,ret=0,k=0;
        char *data,*tok1,*tok2;    
        int my_server;
//runtime commands
        char join_map[BUFSIZE];
        int disjoin_command[MAX_COMMANDS/100];
        int new_parent[MAX_COMMANDS/100];
        int pause_mode=0;
 
//check runtime arguments
	if(argc!=5)
	{
		printf("Usage: ./client <client_id> <def_server> <join_ins> <pause_mode>\n");
		return -1;
	}
	for(i=0;i<MAX_COMMANDS/100;i++)
            disjoin_command[i] = -1;
	my_pid = atoi(argv[1]);
        my_server = atoi(argv[2]);
        strcpy(join_map,argv[3]);
        pause_mode = atoi(argv[4]);
        data = strtok_r(join_map,DELIMITER,&tok1);
        k=0;
        while(data)
        {
                        
                        disjoin_command[k] = atoi(strtok_r(data,DELIMITER_SEC,&tok2));
                        new_parent[k] = atoi( strtok_r(NULL,DELIMITER_SEC,&tok2));
                        k++; 
                        data = strtok_r(NULL,DELIMITER,&tok1);
        }
        //

	 //hostname configuration
	gethostname(hostname, sizeof(hostname));
	hp = gethostbyname(hostname);
	if (hp == NULL) 
	{ 
		printf("\n%s: unknown host.\n", hostname); 
		return 0; 
	} 

//setup server addresses
	for(i=0;i<MAX_SERVERS;i++)
	{
		server_addr_in[i] = (struct sockaddr_in *)&(server_addr[i]);
		server_addr_in[i]->sin_family = AF_INET;
		memcpy(&(server_addr_in[i]->sin_addr), hp->h_addr, hp->h_length); 
		server_addr_in[i]->sin_port  = htons(SERVER_PORT_STARTER+i);  
		server_addr_len[i] = sizeof(server_addr[i]);
	}
	//configure client talker and listener ports	
	//setup the client
	if(configure_client(my_pid,&client_comm))
	{
		printf("Client id: %d configured successfully\n",my_pid);
	}
	else
	{
		printf("Error in config of client id: %d\n",my_pid);
	}

//create listener thread
pthread_create(&listener_thread,NULL,listener,(void *)&client_comm);	
k=0;
while(1)
{
    if(pause_mode)
    {
        printf("=====>paused\n");
         getchar();
         
    }
 
    if(disjoin_command[k] != -1 && disjoin_command[k] == command_counter)
    {
       
        my_server = new_parent[k++]; 
         printf("Switching to server %d\n",my_server);
    }

//fetch commands and supply to server
#if DEBUG == 1
	printf("Client id %d: Command counter:%d\n",my_pid,command_counter);
#endif
	//populate acc_name, cmd_type and arguments to perform the command
	ret = get_next_command(my_pid,command_counter,&cmd_type,song_name,song_url,new_name,new_url);
	if(ret == -1)
	{

		printf("End of command list... !\n");
		break;
	}
        
	CMD_DATA_PREP(song_name,song_url,new_name,new_url,command.command_data);
	command.command_id = GET_NEXT_CMD_ID;
	command.command_type = (enum COMMAND_TYPE) cmd_type;
        cmd_list[command.command_id].command_id = command.command_id;
        cmd_list[command.command_id].command_type = command.command_type ;
        
        //back up commands
        strcpy(cmd_list[command.command_id].arg1,song_name);
        if(command.command_type == 3)
        {
            strcpy(cmd_list[command.command_id].arg2,"");
        }
        else
        {
         strcpy(cmd_list[command.command_id].arg2,song_url);
        }
         if(command.command_type == 2)
         {
                strcpy(cmd_list[command.command_id].arg3,new_name);
                strcpy(cmd_list[command.command_id].arg4,new_url);
         }   
         else
         {
             strcpy(cmd_list[command.command_id].arg3,"");
                strcpy(cmd_list[command.command_id].arg4,"");
         }
         
	command_counter++;

        if(command.command_type ==  COMMAND_READ)
        {
            //check for tentative read
            if(check_tentative(command.command_id,song_url))
            {
                //its a tentative read                
                printf("doing tentative read:%s for command %d  \n",song_url,command.command_id);
                //get result and print
               
            }
            else
            {
                //its something else
                printf("you are not reading your write %d\n",command.command_id);
               
            }
             continue;
        }
	//sending command to client
	strcpy(send_buff,"REQUEST");
	strcat(send_buff,DELIMITER);
	sprintf(send_buff,"%s%d",send_buff,my_pid);
	strcat(send_buff,DELIMITER);
	sprintf(cmd_str,"%s%d",cmd_str,command.command_id);
	strcat(cmd_str,DELIMITER_SEC);
	sprintf(cmd_str,"%s%d",cmd_str,command.command_type);
	strcat(cmd_str,DELIMITER_SEC);
	strcat(cmd_str,command.command_data);
	strcat(cmd_str,DELIMITER_SEC);
	strcat(send_buff,cmd_str);
	strcat(send_buff,DELIMITER);
	strcpy(cmd_str,"");

	printf("Command details: id:%d type:%d data:%s \n",command.command_id,command.command_type,command.command_data);	
	printf("Client %d: Sending commmand %d to SERVER %d \n",my_pid,command.command_id,my_server);
	ret = sendto(TALKER, send_buff, strlen(send_buff), 0, 
  			(struct sockaddr *)&server_addr[my_server], server_addr_len[my_server]);
					
	if (ret < 0)
	{
		perror("sendto ");
	        close(TALKER);
	}
}
pthread_join(listener_thread,NULL);
return 0;
}
