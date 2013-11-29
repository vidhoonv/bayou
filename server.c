/*
CS 380D Distributing Computing

BAYOU protocol
==============

Authors:
@Vidhoon Viswanathan
@Layamrudha RV

*/
#include "butypes.h"

#define TALKER server_comm.comm_fd[TALKER_INDEX]
#define LISTENER server_comm.comm_fd[LISTENER_INDEX]

#define PERFORM_COMMAND(command) \
			printf("\nIN PERFORM COMMAND %d\n",command.command_id); \
			rc = do_command(my_pid,command.command_type,command.command_data);	\
			printf("\n>>>>>>>>Performed command %d res:%d\n",command,rc); \
			respond(my_pid,TALKER,command.command_id,client_addr[command.command_id%MAX_CLIENTS],client_addr_len[command.command_id%MAX_CLIENTS],rc);	

#define PREPARE_IDSTR(STR)	\
			i = 0;	\
			strcpy(STR,"");	\
			while(i <= my_serverid.level)	\
			{	\
				sprintf(STR,"%s%d",STR,my_serverid.id[i]);	\
				strcat(STR,DELIMITER_SEC);	\
				i++;	\
			}	

/*int log_command(struct LOG_CMD cmd,int command_id, time_t start_time)
{
	FILE *fp,,*fptemp;
	char filename[FILENAME_LENGTH];
	char tempname[FILENAME_LENGTH];
	int i=0,;
	size_t len;
	char *line=NULL,*data;
	ssize_t read;
	bool fail=false;
	time_t curr_time;

	strcpy(filename,LOG_FILE_PREFIX);
	sprintf(filename,"%s%d",filename,my_pid);
	strcat(filename,".log");

	strcpy(tempname,"temp");
	sprintf(tempname,"%s%d",tempname,my_pid);
	strcat(tempname,".log");

	fp = fopen(filename,"a+");
	if(fp == NULL)
	{
		printf("file could not be accessed\n");
		return -1;
	}	
	switch(cmd)
	{

		case LOG_ADD: 
				//log add entry
				//<timestamp>:<server id>:<command id>
				//get current time to calculate accept timestamp
				
				
return 0;
}*/
int do_command(int my_pid,int cmd_type,char* cmd_data)
{
	FILE *fp,*fptemp;
	char filename[FILENAME_LENGTH];
	char tempname[FILENAME_LENGTH];
	int i=0;
	size_t len;
	char *line=NULL,*data;
	ssize_t read;
	char song_name[MAX_INPUT_LENGTH],song_url[MAX_INPUT_LENGTH];
	char sn[MAX_INPUT_LENGTH],su[MAX_INPUT_LENGTH];
	char new_sn[MAX_INPUT_LENGTH],new_su[MAX_INPUT_LENGTH];
	char record[2*MAX_INPUT_LENGTH];
	int  op_arg;
	bool fail=false;
	
	strcpy(filename,RESOURCE_FILE_PREFIX);
	sprintf(filename,"%s%d",filename,my_pid);
	strcat(filename,".res");

	strcpy(tempname,"temp");
	sprintf(tempname,"%s%d",tempname,my_pid);
	strcat(tempname,".res");	
	
	fp = fopen(filename,"a+");
	if(fp == NULL)
	{
		printf("file could not be accessed\n");
		return -1;
	}	
	printf("while doing command: cmd_type:%d cmd_data:%s\n",cmd_type,cmd_data);
	
	data = strtok(cmd_data,DELIMITER_CMD);
	if(!data)
		return -2;
	strcpy(song_name,data);
	data = strtok(NULL,DELIMITER_CMD);
	if(!data)
		return -2;
	strcpy(song_url,data);

	//printf("while doing command: acc:%s arg:%d\n",acc_name,op_arg);
	switch(cmd_type)
	{
		case COMMAND_ADD:
					//received add command 
					printf("add received\n");
										
					//adding with new line
					fprintf(fp,"%s-%s\n",song_name,song_url);
					fclose(fp);
				
				break;
		case COMMAND_DELETE:
					//received delete command 
					printf("delete received\n");

					fail=true;
					fptemp = fopen(tempname,"a+");
					if(fptemp == NULL)
					{
						printf("file could not be accessed\n");
						return -1;
					}	


					while(( read = getline(&line,&len,fp)) != -1)
					{
						
						if(line[strlen(line)-1] == '\n')
							line[strlen(line)-1]='\0';
						sprintf(record,"%s-%s",song_name,song_url);
						//printf("line:%s rec:%s\n",line,record);
						if(strcmp(line,record) == 0 )
						{
							printf("record found -deleting\n");
							fail = false;
							continue;
						}
						fprintf(fptemp,"%s\n",line);
					}
					fclose(fptemp);
					fclose(fp);
					if(fail == true)
					{
						if(unlink(tempname) == -1)  //deleting temp res file
							printf("deleting temp resource file failed\n");
						else
							printf("temp res file deleted\n");
						return -1;
	
					}
					if(unlink(filename) == -1)  //deleting old res file
						printf("deleting old resource file failed\n");
					else
						printf("old res file deleted\n");
					if(rename(tempname,filename) == -1) //rename 
						printf("new resource file rename failed\n");
					else
						printf("new resource file ready\n");
					break;					
					
		case COMMAND_EDIT:
					//received edit command 
					printf("edit received\n");

					//fetch remaining arguments for command
					data = strtok(NULL,DELIMITER_CMD);
					if(!data)
						return -2;
					strcpy(new_sn,data);
					data = strtok(NULL,DELIMITER_CMD);
					if(!data)
						return -2;
					strcpy(new_su,data);

					fail=true;
					fptemp = fopen(tempname,"a+");
					if(fptemp == NULL)
					{
						printf("file could not be accessed\n");
						return -1;
					}	
					sprintf(record,"%s-%s",song_name,song_url);
					while(( read = getline(&line,&len,fp)) != -1)
					{
						
						if(line[strlen(line)-1] == '\n')
							line[strlen(line)-1]='\0';
						
						if(strcmp(record,line) == 0 )
						{
							printf("record found -editing\n");
							fprintf(fptemp,"%s-%s\n",new_sn,new_su);
							fail = false;
							continue;
						}
						fprintf(fptemp,"%s\n",line);
					}
					fclose(fptemp);
					fclose(fp);
					if(fail == true)
					{
						if(unlink(tempname) == -1)  //deleting temp res file
							printf("deleting temp resource file failed\n");
						else
							printf("temp res file deleted\n");
						return -1;
	
					}
					if(unlink(filename) == -1)  //deleting old res file
						printf("deleting old resource file failed\n");
					else
						printf("old res file deleted\n");
					if(rename(tempname,filename) == -1) //rename 
						printf("new resource file rename failed\n");
					else
						printf("new resource file ready\n");
					break;			

		default:
				printf("!!!some invalid command\n");
				return -1;

	}	
return 0;

}
void respond(int my_pid,int talker_fd,int command_id,struct sockaddr dest_addr, socklen_t dest_addr_len,int result)
{

	int ret;
	char send_buff[BUFSIZE];

	printf("\nSending response to client\n");
	strcpy(send_buff,"RESPONSE"); 	
	strcat(send_buff,DELIMITER);
	sprintf(send_buff,"%s%d",send_buff,my_pid);	
	strcat(send_buff,DELIMITER);
	sprintf(send_buff,"%s%d",send_buff,command_id);	
	strcat(send_buff,DELIMITER);

	if(result == -1)
	{
		
		strcat(send_buff,"F");

	}
	else if(result == 0)
	{
		
		strcat(send_buff,"S");
	}
	
	strcat(send_buff,DELIMITER);
	

	ret = sendto(talker_fd, send_buff, strlen(send_buff), 0, 
      		(struct sockaddr *)&dest_addr, dest_addr_len);
			
	if (ret < 0)
     	{
      		perror("sendto ");
	        close(talker_fd);
      		//return false;
     	}
//return true;	
}

bool configure_server(int my_pid,struct COMM_DATA *comm_replica)
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
	listener_addr.sin_port = htons(SERVER_PORT_STARTER+my_pid); 

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

	comm_replica->comm_fd[LISTENER_INDEX] = listener_fd;
	comm_replica->comm_fd[TALKER_INDEX] = talker_fd;

return true;
}

int main(int argc, char **argv)
{
	struct COMM_DATA server_comm;
	int my_pid,parent_id;
	struct SERVER_ID my_serverid;
	char my_id_str[BUFSIZE/2];

//comm common
	struct sockaddr_storage temp_paddr;
	socklen_t temp_paddr_len;

//comm related variables
	struct hostent *hp;
	char hostname[64];

//comm listening variables
	fd_set readfds;
	int maxfd;
	char recv_buff[BUFSIZE];
	int nread=0,rc=0;	

//comm sending variables
	char send_buff[BUFSIZE];
	struct sockaddr_in *server_addr_in[MAX_SERVERS],*client_addr_in[MAX_CLIENTS];
	struct sockaddr server_addr[MAX_SERVERS],client_addr[MAX_CLIENTS];
	socklen_t server_addr_len[MAX_SERVERS],client_addr_len[MAX_CLIENTS];	

//commands
	struct COMMAND_ITEM command_list[MAX_COMMANDS];
	struct COMMAND_ITEM command;
	int command_counter = 0;
//misc
	int i,ret=0,recv_pid;
	char buff_copy[BUFSIZE];
	char idstring[BUFSIZE/2];
	char *data,*cstr,*cmd_str,*tok,*idstr,*id_data;
//time - logical clock
	int my_current_time = 0;

//check runtime arguments
	if(argc!=3)
	{
		printf("Usage: ./server <server_id> <parent_id>\n");
		return -1;
	}
	my_pid=atoi(argv[1]);
	parent_id=atoi(argv[2]);

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
		memcpy(&server_addr_in[i]->sin_addr, hp->h_addr, hp->h_length); 
		server_addr_in[i]->sin_port  = htons(SERVER_PORT_STARTER+i);  
		server_addr_len[i] = sizeof(server_addr[i]);
	}

//setup client addresses
	for(i=0;i<MAX_CLIENTS;i++)
	{
		client_addr_in[i] = (struct sockaddr_in *)&(client_addr[i]);
		client_addr_in[i]->sin_family = AF_INET;
		memcpy(&client_addr_in[i]->sin_addr, hp->h_addr, hp->h_length); 
		client_addr_in[i]->sin_port  = htons(CLIENT_PORT_STARTER+i);  
		client_addr_len[i] = sizeof(client_addr[i]);
	}

	//configure server talker and listener ports	
	//setup the server
	if(configure_server(my_pid,&server_comm))
	{
		printf("Server id: %d configured successfully\n",my_pid);
	}
	else
	{
		printf("Error in config of server id: %d\n",my_pid);
		return -1;
	}


//DO CREATE WRITE and get INITIAL TIMESTAMP
if(parent_id == my_pid)
{
	//I am the master server
	my_serverid.level = 0;
	my_serverid.id[my_serverid.level] = 0;

	//INIT current time
	my_current_time = my_serverid.id[my_serverid.level];

	PREPARE_IDSTR(my_id_str);
}
else
{
//get my server id

//send create write
	//sending create write to client
	strcpy(send_buff,"CREATE");
	strcat(send_buff,DELIMITER);
	sprintf(send_buff,"%s%d",send_buff,my_pid);
	strcat(send_buff,DELIMITER);
	
	printf("Server %d: Sending create write to SERVER %d \n",my_pid,parent_id);
	ret = sendto(TALKER, send_buff, strlen(send_buff), 0, 
  			(struct sockaddr *)&server_addr[parent_id], server_addr_len[parent_id]);
					
	if (ret < 0)
	{
		perror("sendto ");
	        close(TALKER);
	}
}
	while(1)
	{
		maxfd = LISTENER+1;
		FD_ZERO(&readfds); 
		FD_SET(LISTENER, &readfds);

		ret = select(maxfd, &readfds, NULL, NULL, NULL);  //blocks forever
		if(ret <0)
	   	{ 
	     		printf("\nSelect error\n");   
	     		return -1;
	   	} 

		if(FD_ISSET (LISTENER, &readfds))
		{
			temp_paddr_len = sizeof(temp_paddr);
			nread = recvfrom (LISTENER, recv_buff, BUFSIZE, 0, 
               	       			(struct sockaddr *)&temp_paddr, &temp_paddr_len); 
		
		 	if (nread < 0)
		       	{
		        	perror("recvfrom ");
            			close(LISTENER);
            			return -1;
        		}		
			recv_buff[nread] = 0;
  			printf("received: %s\n", recv_buff);

			strcpy(buff_copy,recv_buff);			
			data = strtok(buff_copy,DELIMITER);

//retrive recv_pid
				recv_pid = atoi(strtok(NULL,DELIMITER));
#if DEBUG==1
				printf("recved msg from %d\n",recv_pid);
#endif		

			if(strcmp(data,"CREATE") == 0)	
			{
				//received from parent server
				//expects data in the format
				//CREATE:<SERVER_ID>:


				//LOG CREATE WRITE (update time)
				my_current_time++;

				//send ID to new server
				//sending idstr to client
				strcpy(send_buff,"CREATED");
				strcat(send_buff,DELIMITER);
				sprintf(send_buff,"%s%d",send_buff,my_pid);
				strcat(send_buff,DELIMITER);
				strcat(send_buff,my_id_str);
				//strcat(send_buff,DELIMITER);
				sprintf(send_buff,"%s%d",send_buff,my_current_time-1);	
				strcat(send_buff,DELIMITER);

				printf("Server %d: Sending ID to SERVER %d \n",my_pid,recv_pid);
				ret = sendto(TALKER, send_buff, strlen(send_buff), 0, 
  						(struct sockaddr *)&server_addr[recv_pid], server_addr_len[recv_pid]);
					
				if (ret < 0)
				{
					perror("sendto ");
				        close(TALKER);
				}			

			}
			else if(strcmp(data,"CREATED") == 0)
			{
				//received from parent server
				//expects data in the format
				//CREATED:<SERVER_ID>:<ID_STR>:
				//retrive id string
				idstr = strtok(NULL,DELIMITER);
				id_data = strtok(idstr,DELIMITER_SEC);
				my_serverid.level = 0;
				while(id_data)
				{
					my_serverid.id[my_serverid.level] = atoi(id_data);
					my_serverid.level++;
					id_data = strtok(NULL,DELIMITER_SEC);

				}					
				my_serverid.level--;
				//INIT current time
				my_current_time = my_serverid.id[my_serverid.level]+1;
				//prepare ID string 
				PREPARE_IDSTR(my_id_str);
				printf("I created myself successfully - idstr:%s currtime:%d\n",my_id_str,my_current_time);
			}
			else if(strcmp(data,"REQUEST") == 0)
			{
				//recved from client
				//expects data in the format
				//REQUEST:<CLIENT_ID>:<COMMAND_STR>:

				//retrive command string
				cstr = strtok(NULL,DELIMITER);

				command_list[command_counter].command_id = atoi(strtok(cstr,DELIMITER_SEC));

				command_list[command_counter].command_type = (enum COMMAND_TYPE)atoi(strtok(NULL,DELIMITER_SEC));
				strcpy(command_list[command_counter].command_data,strtok(NULL,DELIMITER_SEC));				
				
				//update timestamp for received command
				command_list[command_counter].timestamp = my_current_time;
				my_current_time++;

				//log the write request
				
				//perform command tentatively and send result to client
				command = command_list[command_counter];
				PERFORM_COMMAND(command);

			}
		} 

	}

return 0;
}
