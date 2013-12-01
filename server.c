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
			printf("\n>>>>>>>>Performed command %d res:%d\n",command.command_id,rc); \
			respond(my_pid,TALKER,command.command_id,client_addr[command.command_id%MAX_CLIENTS],client_addr_len[command.command_id%MAX_CLIENTS],rc);	

#define PREPARE_IDSTR(serverid,STR,DL)	\
			i = 0;	\
			strcpy(STR,"");	\
			while(i < serverid.level)	\
			{	\
				sprintf(STR,"%s%d",STR,serverid.id[i]);	\
				strcat(STR,DL);	\
				i++;	\
			}	

#define CREATE_ENTROPY_TIMER \
				edata.server_comm = &server_comm;	\
				printf("****** fd: %d %d\n",edata.server_comm->comm_fd[TALKER_INDEX],server_comm.comm_fd[TALKER_INDEX]);\
				edata.sender_list = ent_list;	\
				edata.my_pid = my_pid;	\
				entropy_data = (void*)(&edata);	\
				sevp.sigev_notify = SIGEV_THREAD;	\
				sevp.sigev_value.sival_ptr = entropy_data;	\
				sevp.sigev_notify_function = &init_anti_entropy;	\
				sevp.sigev_notify_attributes = NULL;	\
				timer_create(clkid,&sevp,&entropy_timer);

#define PREPARE_VVSTR()	\
			sprintf(log_str,"%d",my_version_vector.csn);	\
			strcat(log_str,DELIMITER);	\
			for(j=0;j<my_version_vector.server_count;j++)	\
			{	\
				PREPARE_IDSTR(my_version_vector.servers[j],idstring,DELIMITER_QUAT);	\
				strcat(log_str,idstring);	\
				strcat(log_str,DELIMITER_TER);	\
				sprintf(log_str,"%s%d",log_str,my_version_vector.recent_timestamp[j]);	\
				strcat(log_str,DELIMITER_TER);	\
				strcat(log_str,DELIMITER_SEC);	\
			}	

#define EXTRACT_SERVERID(STR,sid,DL)	\
			sid.level = 0;	\
			id_data = strtok_r(STR,DL,&tok2);	\
			sid.level = 0;	\
			while(id_data)	\
			{	\
				sid.id[sid.level] = atoi(id_data);	\
				sid.level++;	\
				id_data = strtok_r(NULL,DL,&tok2);	\
			}	\
			
			
			
//globals
char log_str[BUFSIZE];
//version vector
	struct VERSION_VECTOR my_version_vector;
	char idstring[BUFSIZE/2];
//server and client addr related
	struct sockaddr_in *server_addr_in[MAX_SERVERS],*client_addr_in[MAX_CLIENTS];
	struct sockaddr server_addr[MAX_SERVERS],client_addr[MAX_CLIENTS];
	socklen_t server_addr_len[MAX_SERVERS],client_addr_len[MAX_CLIENTS];	

struct COMM_DATA server_comm;

bool equal_serverID(struct SERVER_ID s1,struct SERVER_ID s2)
{
	int i=0;
	
	if(s1.level == s2.level)
	{
		for(i=0;i<s1.level;i++)
		{

			if(s1.id[i] == s2.id[i])
				continue;
			else
				return false;
		}
		return true;

	}
return false;
}
bool copy_serverID(struct SERVER_ID *dest,struct SERVER_ID src)
{
	int i=0;

	dest->level = src.level;
	for(i=0;i<src.level;i++)
	{	
		dest->id[i] = src.id[i];	

	}
return true;
}
bool compare_serverID(struct SERVER_ID s1,struct SERVER_ID s2)
{
	int i=0;

	if(s1.level<s2.level)
		return true;
	if(s1.level == s2.level)
	{
		for(i=s1.level-1;i>=0;i--)
		{	
			if(s1.id[i] > s2.id[i])
				return false;
			else if(s1.id[i] < s2.id[i])
				return true;
		}
	}
return false;
}

bool log_insert(FILE *fp,FILE *fptemp,char *record)
{
	
	char *idstr,*cmd_str,*id_data;
	char *log_idstr,*log_cmd_str;
	char *log_data;
	int log_ts,ts;
	bool fail=false,inserted=false;

	char rcopy[BUFSIZE],lncopy[BUFSIZE];
	int i=0;
	size_t len;
	char *line=NULL,*data,*tok2;
	ssize_t read;
	
	struct SERVER_ID serv,log_serv;
	//the file is open
	
	//fetch ts, serverID from record
	strcpy(rcopy,record);
	
	data = strtok(rcopy,DELIMITER);
	if(data)
		ts = atoi(data);

	idstr = strtok(NULL,DELIMITER);
	cmd_str = strtok(NULL,DELIMITER);

	
	EXTRACT_SERVERID(idstr,serv,DELIMITER_SEC);					
		
	fail=true; inserted=false;
	//iterate over each log record and find position of received record
	while(( read = getline(&line,&len,fp)) != -1)
	{
		if(line[strlen(line)-1] == '\n')
			line[strlen(line)-1]='\0';
			
			strcpy(lncopy,line);

			//get ts and server id
			log_data = strtok(line,DELIMITER);
			log_ts = atoi(log_data);

			//compare ts , then server id
			log_idstr = strtok(NULL,DELIMITER);
			EXTRACT_SERVERID(log_idstr,log_serv,DELIMITER_SEC);
			
			//printf("line:%s rec:%s\n",line,record);
			if(ts < log_ts && !inserted)
			{
				printf("insert position found -inserting\n");
				fprintf(fptemp,"%s\n",record);
				fail = false;
				inserted =true;
				
			}
			else if(ts == log_ts && !inserted)
			{

				//check server ids to decide position

				if(compare_serverID(serv,log_serv))
				{
					//serv has high priority
					printf("insert position found -inserting\n");
					fprintf(fptemp,"%s\n",record);
					fail = false;
					inserted = true;
					

				}

			}
			fprintf(fptemp,"%s\n",lncopy);
	}
	if(!inserted)
	{
		//insert entry at the end of the log
		
		fprintf(fptemp,"%s\n",record);
		fail = false;
		inserted = true;
	}
	return (inserted);	
}
int log_command(enum LOG_CMD cmd,int my_pid,char *record)
{
	FILE *fp,*fptemp;
	char filename[FILENAME_LENGTH];
	char tempname[FILENAME_LENGTH];
	int i=0;
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
								
				fprintf(fp,"%s\n",record);
				fclose(fp);
				break;

		case LOG_INSERT:
				fptemp = fopen(tempname,"a+");
				if(fptemp == NULL)
				{
					printf("file could not be accessed\n");
					return -1;
				}	

				log_insert(fp,fptemp,record);
				fclose(fptemp);
				fclose(fp);
				if(unlink(filename) == -1)  //deleting old res file
					printf("deleting old resource file failed\n");
				else
					printf("old res file deleted\n");
				if(rename(tempname,filename) == -1) //rename 
					printf("new resource file rename failed\n");
				else
					printf("new resource file ready\n");
				break;
		case LOG_FETCH:
				
				break;
		case LOG_DELETE:
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
		default:
			break;

	}
				
				
return 0;
}
int slog_command(enum LOG_CMD cmd,int my_pid,char *record)
{
	FILE *fp,*fptemp;
	char filename[FILENAME_LENGTH];
	char tempname[FILENAME_LENGTH];
	int i=0;
	size_t len;
	char *line=NULL,*data;
	ssize_t read;
	bool fail=false;

	
	strcpy(filename,SLOG_FILE_PREFIX);
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
							
				fprintf(fp,"%s\n",record);
				fclose(fp);
				break;

		case LOG_INSERT:
				
			
				break;
		case LOG_FETCH:
				
				break;
		default:
				break;

	}
				
				
return 0;
}
int clog_command(enum LOG_CMD cmd,int my_pid,char *record)
{
	FILE *fp,*fptemp;
	char filename[FILENAME_LENGTH];
	char tempname[FILENAME_LENGTH];
	int i=0;
	size_t len;
	char *line=NULL,*data;
	ssize_t read;
	bool fail=false;

	
	strcpy(filename,CLOG_FILE_PREFIX);
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
				fprintf(fp,"%s\n",record);
				fclose(fp);
				break;
		default:
				break;

	}
				
				
return 0;
}
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

void init_anti_entropy(union sigval args)
{

	struct ENTROPY_INPUT *data;	
	struct COMM_DATA *server_comm;
	int my_pid,i,j,k,ret;
	char send_buff[BUFSIZE];
	int *sender_id;
	data = (struct ENTROPY_INPUT*)(args.sival_ptr);
	
	server_comm = data->server_comm;
	sender_id = data->sender_list;
	my_pid =  data->my_pid;
	printf("timer expired\n");
	
	PREPARE_VVSTR();

	printf("timer expired!!!\n");
	//send ENTROPY
	strcpy(send_buff,"ENTROPY");
	strcat(send_buff,DELIMITER);
	sprintf(send_buff,"%s%d",send_buff,my_pid);
	strcat(send_buff,DELIMITER);
	strcat(send_buff,log_str);
	strcat(send_buff,DELIMITER);
	
	for(k=0;k<MAX_SERVERS;k++)
	{
	if(*(sender_id+k) == 1)
	{
		printf("Server %d: Sending entropy  to SERVER %d \n",my_pid,k);
		ret = sendto(server_comm->comm_fd[TALKER_INDEX], send_buff, strlen(send_buff), 0, 
  			(struct sockaddr *)&server_addr[k], server_addr_len[k]);
					
		if (ret < 0)
		{
			perror("sendto ");
		        close(server_comm->comm_fd[TALKER_INDEX]);
		}
	}
	}
	//cleanup
	strcpy(log_str,"");
	
}

bool send_message(int talker_fd,struct sockaddr dest_addr, socklen_t dest_addr_len,char *send_buff)
{
	int ret=0;
	
	ret = sendto(TALKER, send_buff, strlen(send_buff), 0, 
  			(struct sockaddr *)&dest_addr, dest_addr_len);
					
	if (ret < 0)
	{
		perror("sendto ");
	        close(talker_fd);
	}
	
}
bool process_logs(struct VERSION_VECTOR recv_vv, struct VERSION_VECTOR my_vv, int recv_pid,int my_pid)
{

	FILE *fp_log,*fp_slog;
	char log_fname[FILENAME_LENGTH],slog_fname[FILENAME_LENGTH];
	int i=0;
	size_t len;
	char *line=NULL,*data;
	ssize_t read;
	bool fail=false;

	int csn;
	char *idstr,*id_data;
	int ts;
	char *cmd_str,*tok2;
	struct SERVER_ID serv;
	int serv_ts = -1;
	int line_count = 0;

	char send_buff[BUFSIZE];
	char lncopy[BUFSIZE];	

	strcpy(log_fname,LOG_FILE_PREFIX);
	sprintf(log_fname,"%s%d",log_fname,my_pid);
	strcat(log_fname,".log");

	strcpy(slog_fname,SLOG_FILE_PREFIX);
	sprintf(slog_fname,"%s%d",slog_fname,my_pid);
	strcat(slog_fname,".log");	
	
	fp_log = fopen(log_fname,"a+");
	if(fp_log == NULL)
	{
		printf("log file could not be accessed\n");
		return -1;
	}

	fp_slog = fopen(slog_fname,"a+");
	if(fp_slog == NULL)
	{
		printf("slog file could not be accessed\n");
		return -1;
	}		
	
	
//process stable log
	line_count = 0;
	if(my_vv.csn > recv_vv.csn)
	{
		//I have some stable writes that partner might not know
		//stable log line format
		// csn:acceptTS:serverID:command_data
		printf("\n\nrecv_vv.csn %d\n",recv_vv.csn);
		while(( read = getline(&line,&len,fp_slog)) != -1)
		{
			if(line_count<=recv_vv.csn)
			{
				line_count++;
				continue;
			}
			if(line[strlen(line)-1] == '\n')
				line[strlen(line)-1]='\0';


			strcpy(lncopy,line);

			data = strtok(lncopy,DELIMITER);
			if(data)
				csn = atoi(data);
			
			data = strtok(NULL,DELIMITER);
			if(data)	
				ts = atoi(data);

			idstr = strtok(NULL,DELIMITER);
			cmd_str = strtok(NULL,DELIMITER);

			
			EXTRACT_SERVERID(idstr,serv,DELIMITER_SEC);	
			
			for(i=0;i<recv_vv.server_count;i++)
			{
				if(equal_serverID(recv_vv.servers[i],serv))
				{

					serv_ts = recv_vv.recent_timestamp[i];
					break;
				}
				
			}
			
	
			if(ts <= serv_ts)
			{
				//send commit notification
				//copy log line as is
				//COMMIT_NOTIFICATION:<SENDER_ID>:<WRITE_LOG>:
				strcpy(send_buff,"COMMIT_NOTIFICATION");
				strcat(send_buff,DELIMITER);
				sprintf(send_buff,"%s%d",send_buff,my_pid);
				strcat(send_buff,DELIMITER);
				strcat(send_buff,line); //check line contents and change
				//strcat(send_buff,DELIMITER);
				printf("Server %d: Sending commit notification to server %d \n",my_pid,recv_pid);
				send_message(TALKER,server_addr[recv_pid],server_addr_len[recv_pid],send_buff);
			}
			else
			{

					if(serv_ts == -1) //the receiving server's version vector does not have an entry for the server
					{

						printf("the receiving server's version vector does not have an entry for the server!\n");
					}
				//send write log entry
				//send commit notification
				//copy log line as is
				//ENTROPY_MSG:<SENDER_ID>:<WRITE_LOG>:
				strcpy(send_buff,"ENTROPY_SMSG");
				strcat(send_buff,DELIMITER);
				sprintf(send_buff,"%s%d",send_buff,my_pid);
				strcat(send_buff,DELIMITER);
				strcat(send_buff,line); //check line contents and change
				//strcat(send_buff,DELIMITER);
				printf("Server %d: Sending WRITE LOG(STABLE) to server %d \n",my_pid,recv_pid);
				send_message(TALKER,server_addr[recv_pid],server_addr_len[recv_pid],send_buff);
			}
			//reset value
			serv_ts = -1;
			
						
		}	
		

	}


//process tentative log
	while(( read = getline(&line,&len,fp_log)) != -1)
	{

		if(line[strlen(line)-1] == '\n')
				line[strlen(line)-1]='\0';


			strcpy(lncopy,line);

			data = strtok(lncopy,DELIMITER);
						
			if(data)	
				ts = atoi(data);

			idstr = strtok(NULL,DELIMITER);
			cmd_str = strtok(NULL,DELIMITER);

			
			EXTRACT_SERVERID(idstr,serv,DELIMITER_SEC);	
			
			for(i=0;i<recv_vv.server_count;i++)
			{
				if(equal_serverID(recv_vv.servers[i],serv))
				{

					serv_ts = recv_vv.recent_timestamp[i];
					break;
				}
				
			}

			if(ts > serv_ts)
			{
				if(serv_ts == -1) //the receiving server's version vector does not have an entry for the server
				{

						printf("the receiving server's version vector does not have an entry for the server!\n");
				}
				//send commit notification
				//copy log line as is
				//COMMIT_NOTIFICATION:<SENDER_ID>:<WRITE_LOG>:
				strcpy(send_buff,"ENTROPY_TMSG");
				strcat(send_buff,DELIMITER);
				sprintf(send_buff,"%s%d",send_buff,my_pid);
				strcat(send_buff,DELIMITER);
				strcat(send_buff,line); //check line contents and change
				//strcat(send_buff,DELIMITER);
				printf("Server %d: Sending WRITE LOG(TENTATIVE) to server %d \n",my_pid,recv_pid);
				send_message(TALKER,server_addr[recv_pid],server_addr_len[recv_pid],send_buff);
			}
			//reset values
			serv_ts = -1;


	}
}

int main(int argc, char **argv)
{
	
	int my_pid,parent_id;
	struct SERVER_ID my_serverid;
	char my_id_str[BUFSIZE/2];
	int primary_mode = 0;

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


//commands
	struct COMMAND_ITEM command_list[MAX_COMMANDS];
	struct COMMAND_ITEM command;
	int command_counter = 0;
//misc
	int i,j,k=0,ret=0,recv_pid;
	char buff_copy[BUFSIZE];
	char *data,*cstr,*cmd_str,*tok,*tok1,*tok2,*idstr,*id_data,*vv_str,*vv_data;
	char command_str[BUFSIZE/2];
	char log_record[BUFSIZE];
	void *entropy_data;
	struct ENTROPY_INPUT edata;
	struct VERSION_VECTOR recv_vv;
	struct SERVER_ID serv;
	int ts=0,recv_csn = -1;
	char rec[BUFSIZE];
//time - logical clock
	int my_current_time = 0;

//timer
	timer_t entropy_timer;
	struct sigevent sevp;
	clockid_t clkid =  CLOCK_REALTIME;
	struct itimerspec entropy_timer_val,old_val;
	int tret = 0;
	int ent_list[MAX_SERVERS];
//check runtime arguments
	if(argc!=4)
	{
		printf("Usage: ./server <server_id> <parent_id> <primary_mode\n");
		return -1;
	}
	my_pid=atoi(argv[1]);
	parent_id=atoi(argv[2]);
	primary_mode=atoi(argv[3]);

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
		ent_list[i] = -1;
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
//init timer args
	entropy_timer_val.it_value.tv_sec = 10;
	entropy_timer_val.it_value.tv_nsec = 0;
	entropy_timer_val.it_interval.tv_sec = 15;
	entropy_timer_val.it_interval.tv_nsec = 0;

	if(!entropy_timer)
		CREATE_ENTROPY_TIMER;
//intial values for version vector 
	my_version_vector.csn = -1;
	for(i=0;i<MAX_SERVERS;i++)
	{
		my_version_vector.servers[i].level = -1;
		my_version_vector.recent_timestamp[i] = -1;
	
		recv_vv.servers[i].level = -1;
		recv_vv.recent_timestamp[i] = -1;
	}
		my_version_vector.server_count = 1;
		
//DO CREATE WRITE and get INITIAL TIMESTAMP
if(parent_id == my_pid)
{
	//I am the master server
	my_serverid.level = 1;
	my_serverid.id[my_serverid.level] = 0;

	//INIT current time
	my_current_time = my_serverid.id[my_serverid.level];

	PREPARE_IDSTR(my_serverid,my_id_str,DELIMITER_SEC);

	copy_serverID(&(my_version_vector.servers[0]),my_serverid);

	//start entropy timer
	tret = timer_settime(entropy_timer,0,&entropy_timer_val,&old_val);
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
				//<infinity>:<timestamp>:<my_id_str>
				
				sprintf(log_record,"%d",INFINITY);
				strcat(log_record,DELIMITER);	
				sprintf(log_record,"%s%d",log_record,my_current_time);
				strcat(log_record,DELIMITER);
				strcat(log_record,my_id_str);
				strcat(log_record,DELIMITER);

				clog_command(LOG_ADD,my_pid,log_record);
				//increment time
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
				EXTRACT_SERVERID(idstr,my_serverid,DELIMITER_SEC);
				//INIT current time
				my_current_time = my_serverid.id[my_serverid.level]+1;
				//prepare ID string 
				PREPARE_IDSTR(my_serverid,my_id_str,DELIMITER_SEC);
				printf("I created myself successfully - idstr:%s currtime:%d\n",my_id_str,my_current_time);
			
				//update version vector with my server id
				copy_serverID(&(my_version_vector.servers[0]),my_serverid);
				//start entropy timer
				tret = timer_settime(entropy_timer,0,&entropy_timer_val,&old_val);

				//entropy sender list
				ent_list[recv_pid] = 1;
				
				
			}
			else if(strcmp(data,"REQUEST") == 0)
			{
				//recved from client
				//expects data in the format
				//REQUEST:<CLIENT_ID>:<COMMAND_STR>:

				//retrive command string
				cstr = strtok(NULL,DELIMITER);
				strcpy(command_str,cstr);
				command_list[command_counter].command_id = atoi(strtok(cstr,DELIMITER_SEC));

				command_list[command_counter].command_type = (enum COMMAND_TYPE)atoi(strtok(NULL,DELIMITER_SEC));
				strcpy(command_list[command_counter].command_data,strtok(NULL,DELIMITER_SEC));				

				

				//update timestamp for received command
				command_list[command_counter].timestamp = my_current_time;
				my_current_time++;

				//log the write request
				if(primary_mode)
				{
					//<timestamp>:<server id>:<command str>
				
					my_version_vector.csn++;
	
					sprintf(log_record,"%d",my_version_vector.csn);
					strcat(log_record,DELIMITER);
					sprintf(log_record,"%s%d",log_record,my_current_time);
					strcat(log_record,DELIMITER);	
					strcat(log_record,my_id_str);
					strcat(log_record,DELIMITER);
					strcat(log_record,command_str);
					strcat(log_record,DELIMITER);

					slog_command(LOG_ADD,my_pid,log_record);
				}
				else
				{
					//<timestamp>:<server id>:<command str>
				
					sprintf(log_record,"%d",my_current_time);
					strcat(log_record,DELIMITER);	
					strcat(log_record,my_id_str);
					strcat(log_record,DELIMITER);
					strcat(log_record,command_str);
					strcat(log_record,DELIMITER);

					log_command(LOG_ADD,my_pid,log_record);

				}
				//perform command tentatively and send result to client
				command = command_list[command_counter];
				PERFORM_COMMAND(command);

				//update my entry in version vector
				my_version_vector.recent_timestamp[0] = my_current_time;

			

			}
			else if(strcmp(data,"ENTROPY") == 0)
			{
				//recved from server
				//expects data in the format
				//ENTROPY:<SERVER_ID>:<CSN>:<VERSION_VECTOR>:
				
				ent_list[recv_pid] = 1;
				data = strtok(NULL,DELIMITER);
				if(data)
				{
					recv_vv.csn = atoi(data);
				}
				else
				{

					printf("error: csn not found!");
				}

				vv_str = strtok(NULL,DELIMITER);

				if(vv_str)
				{
					vv_data = strtok_r(vv_str,DELIMITER_SEC,&tok);
					k=0;
					while(vv_data)
					{
					//<server_id,timestamp>

					idstr = strtok_r(vv_data,DELIMITER_TER,&tok1);

					EXTRACT_SERVERID(idstr,recv_vv.servers[k],DELIMITER_QUAT);

					recv_vv.recent_timestamp[k] = atoi(strtok_r(NULL,DELIMITER_TER,&tok1));


					vv_data = strtok_r(NULL,DELIMITER_SEC,&tok);
					k++;
					}
					recv_vv.server_count = k;

				}
				else
				{

					printf("new server - no version vector entries found!\n");
				}				
				if(process_logs(recv_vv,my_version_vector,recv_pid,my_pid)) 
				{

					printf("anti entropy messages sent!\n");
/*
					PREPARE_VVSTR();

					//send entropy
					//sending create entropy to client
					strcpy(send_buff,"ENTROPY");
					strcat(send_buff,DELIMITER);
					sprintf(send_buff,"%s%d",send_buff,my_pid);
					strcat(send_buff,DELIMITER);
					strcat(send_buff,log_str);
					strcat(send_buff,DELIMITER);
	
	
					printf("Server %d: Sending entropy  to SERVER %d \n",my_pid,recv_pid);
					ret = sendto(TALKER, send_buff, strlen(send_buff), 0, 
  								(struct sockaddr *)&server_addr[recv_pid], server_addr_len[recv_pid]);
					
					if (ret < 0)
					{
						perror("sendto ");
	      					close(TALKER);
					}
					//cleanup
					strcpy(log_str,"");
*/
				}

				else
				{
					printf("error: while sending anti entropy messages");

				}
				

				
				

			}
			else if(strcmp(data,"ENTROPY_TMSG") == 0 )
			{
				//ENTROPY_TMSG:SENDER_ID:TIMESTAMP:SERVER_ID:COMMAND_STR
				//printf("recv buff %s\n",recv_buff);
				strcpy(log_record,recv_buff);
				data = strtok(NULL,DELIMITER);
				if(data)
					ts = atoi(data);

				idstr = strtok(NULL,DELIMITER);
				cmd_str = strtok(NULL,DELIMITER);

			
				EXTRACT_SERVERID(idstr,serv,DELIMITER_SEC);					
		
				// do log insert for the received write log in tentative log
				data=strtok_r(log_record,DELIMITER,&tok);
				data=strtok_r(NULL,DELIMITER,&tok);
				strcpy(rec,tok);
				//printf("!!!!! log record %s\n",tok);

				//log the write request
				if(primary_mode)
				{
					//<timestamp>:<server id>:<command str>
				
					my_version_vector.csn++;
					
					sprintf(log_record,"%d",my_version_vector.csn);
					strcat(log_record,DELIMITER);
					strcat(log_record,rec);	

					printf("\nlog_record: %s\n",log_record);
					
					slog_command(LOG_ADD,my_pid,log_record);
				}
				else
				{
					log_command(LOG_INSERT,my_pid,rec);
				}
				//update version vector
				
				//updating timestamp
				for(i=0;i<my_version_vector.server_count;i++)
				{
					if(equal_serverID(my_version_vector.servers[i],serv))
					{
						printf("timestamp updated! %d\n",ts);
						my_version_vector.recent_timestamp[i] = ts;
						break;
					}
				
				}
				if(i == my_version_vector.server_count)
				{
					//server did not have an entry for the write log's server
					if(copy_serverID(&(my_version_vector.servers[my_version_vector.server_count]),serv))
					{
						printf("serv id copied to vv!");
					}
					else
					{
						printf("error: serv id copy to vv failed!");

					}
					printf("!!!!!!timestamp updated %d\n",ts);
					my_version_vector.recent_timestamp[my_version_vector.server_count] = ts;

					my_version_vector.server_count++;
				}
					
				
			}
			else if(strcmp(data,"ENTROPY_SMSG") == 0)
			{
				strcpy(log_record,recv_buff);
				data = strtok(NULL,DELIMITER);
				if(data)
					recv_csn = atoi(data);
				data = strtok(NULL,DELIMITER);
				if(data)
					ts = atoi(data);
				idstr = strtok(NULL,DELIMITER);
				cmd_str = strtok(NULL,DELIMITER);
				EXTRACT_SERVERID(idstr,serv,DELIMITER_SEC);

				data=strtok_r(log_record,DELIMITER,&tok);
				data=strtok_r(NULL,DELIMITER,&tok);
				strcpy(rec,tok);
				slog_command(LOG_ADD,my_pid,rec);

				//update version vector
				//printf("CSN RECEIVED -----------> %d\n",recv_csn);
				my_version_vector.csn++; //stable write so increment CSN
				
				//updating timestamp
				for(i=0;i<my_version_vector.server_count;i++)
				{
					if(equal_serverID(my_version_vector.servers[i],serv))
					{
						//printf("timestamp updated! %d\n",ts);
						my_version_vector.recent_timestamp[i] = ts;
						break;
					}
				
				}
				if(i == my_version_vector.server_count)
				{
					//server did not have an entry for the write log's server
					if(copy_serverID(&(my_version_vector.servers[my_version_vector.server_count]),serv))
					{
						printf("serv id copied to vv!");
					}
					else
					{
						printf("error: serv id copy to vv failed!");

					}
					//printf("!!!!!!timestamp updated %d\n",ts);
					my_version_vector.recent_timestamp[my_version_vector.server_count] = ts;

					my_version_vector.server_count++;
				}				
				
			}
			else if(strcmp(data,"COMMIT_NOTIFICATION") == 0)
			{
				strcpy(log_record,recv_buff);	
	
				data = strtok(NULL,DELIMITER);
				if(data)
					recv_csn = atoi(data);
				data = strtok(NULL,DELIMITER);
				if(data)
					ts = atoi(data);
				idstr = strtok(NULL,DELIMITER);
				cmd_str = strtok(NULL,DELIMITER);
		
				data=strtok_r(log_record,DELIMITER,&tok);
				data=strtok_r(NULL,DELIMITER,&tok);
				slog_command(LOG_ADD,my_pid,tok);
				
				//remove csn, delete from tentative log
				data=strtok_r(NULL,DELIMITER,&tok);
				log_command(LOG_DELETE,my_pid,tok);

				//update version vector
				my_version_vector.csn++; //stable write so increment CSN
				
				//updating timestamp
				for(i=0;i<my_version_vector.server_count;i++)
				{
					if(equal_serverID(my_version_vector.servers[i],serv))
					{
						//printf("timestamp updated! %d\n",ts);
						my_version_vector.recent_timestamp[i] = ts;
						break;
					}
				
				}
				if(i == my_version_vector.server_count)
				{
					//server did not have an entry for the write log's server
					if(copy_serverID(&(my_version_vector.servers[my_version_vector.server_count]),serv))
					{
						printf("serv id copied to vv!");
					}
					else
					{
						printf("error: serv id copy to vv failed!");

					}
					//printf("!!!!!!timestamp updated %d\n",ts);
					my_version_vector.recent_timestamp[my_version_vector.server_count] = ts;

					my_version_vector.server_count++;
				}				
				

			}
		} 

		
	}
return 0;
}
