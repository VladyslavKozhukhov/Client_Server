#define _GNU_SOURCE

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include "TCP_helpers.h"
#include "constants.h"
#include "_Aux.h"

#define VAR_SIZE 26
#define MAX_CHAT_LEN 500


int main(int argc, char** argv)
{	fd_set mainSet;
	fd_set tmpSet;

	FD_ZERO(&mainSet);
	FD_ZERO(&tmpSet);
	
	
	int defPort = 1337;
	char* defHostname = "localhost";
	struct sockaddr_in server_addr;
	int sockfd = 0;
	char receivedBuff[MESSAGE_SIZE] = { 0 };
	int sizeByte = 0;
	struct hostent* host;//struct to get IP by Host Name


	memset(receivedBuff, '\0', sizeof(receivedBuff));//put 0 to all recive memory

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)//open socket
	{
		printf("\n Error : Could not create socket \n");
		return 1;
	}

	memset(&server_addr, '0', sizeof(server_addr));
	server_addr.sin_family = AF_INET;//set IPV4


	if (argc == 3) {
		server_addr.sin_port = htons(atoi(argv[2]));
	}
	else {
		server_addr.sin_port = htons(defPort);
	}
	if (argc == 2) {
		host = gethostbyname(argv[1]);//get Host struck from Name
	}
	else {
		host = gethostbyname(defHostname);
	}
	if (host == NULL){
		printf("DNS error\n");
		exit(1);
	}
	struct in_addr *address = (struct in_addr *) host->h_addr;

	char* IP = inet_ntoa(*address);//get Ip from Host Name

	if (inet_pton(AF_INET, IP, &server_addr.sin_addr) <= 0)//parse String "IP" to binary
	{
		printf("\n inet_pton error occured\n");
		return 1;
	}
	if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)//connect to Server.
	{
		printf("\n Error : Connect Failed \n");
		return 1;
	}

	char var1[PATH_MAX] = { 0 };
	char var2[PATH_MAX] = { 0 };
	char var3[PATH_MAX] = { 0 };

	//read login message
	readEntireMessage(sockfd, receivedBuff, MESSAGE_SIZE);//add select to this function
	if (receivedBuff[0] == 0){
		//message
		printf("%s", receivedBuff + MSG_ARGUMENT_OFFSET);
	}
	


	while(true){
		FD_SET(0,&tmpSet);//add StdIn to listener set
		FD_SET(sockfd,&tmpSet);//add socked to listener set
		if (select(sockfd+1,&tmpSet,NULL,NULL,NULL) == -1)
		{
			perror("select:");
			exit(1);
		}
	
	
	
	
	if (FD_ISSET(sockfd, &tmpSet)){
		sizeByte = readEntireMessage(sockfd,receivedBuff,MAX_CHAT_LEN);//read msg
		receivedBuff[sizeByte]='\0';
		char* path = "";
		path = var3;


		if(sizeByte < 0 || sizeByte > MESSAGE_SIZE)//check size
		{
			printf("size of message out of range ");
			exit(1);
		}
		
	
		if(receivedBuff[0]==9)//check if get from User
		{
			printf("New Message from ");

			for(int dh =1; dh <MSG_FILE_CONTENTS_OFFSET; dh ++){
				if(receivedBuff[dh]!=EOF){
				printf("%c",receivedBuff[dh]);
				} else{
					break;
				}
			}
			printf(": ");
			for(int dh = MSG_FILE_CONTENTS_OFFSET; dh < FILE_MAX_SIZE ; dh++){
				if(receivedBuff[dh]!=EOF){
				printf("%c",receivedBuff[dh]);
				} else{
					break;
				}
			}
			printf("\n");

		}		
		if(receivedBuff[0]==10)//check if get from User
		{
			printf("Message received from ");

			for(int dh =1; dh <MSG_FILE_CONTENTS_OFFSET; dh ++){
				if(receivedBuff[dh]!=EOF){
				printf("%c",receivedBuff[dh]);
				}else{
					break;
				}
			}
			printf(": ");
			for(int dh = MSG_FILE_CONTENTS_OFFSET; dh < FILE_MAX_SIZE ; dh++){
				if(receivedBuff[dh]!=EOF){
				printf("%c",receivedBuff[dh]);
				}else{
					break;
				}
			}
			printf("\n");

		}
		if (receivedBuff[0] == 0){
			//message
			printf("%s", receivedBuff + MSG_ARGUMENT_OFFSET);
		}
			
		if (receivedBuff[0] == 6){
			//file
			size_t size = 0;
			size += receivedBuff[FILE_SIZE_OFFSET] * 100;
			size += receivedBuff[FILE_SIZE_OFFSET+1] * 10;
			size += receivedBuff[FILE_SIZE_OFFSET+2];
			writeFile(path, NULL, receivedBuff + MSG_FILE_CONTENTS_OFFSET, size);
		}
		if(receivedBuff[0] == 8){
			printf("online users: %s\n",&receivedBuff[1]);
		}
		

	}
	
	if (FD_ISSET(0, &tmpSet)){
		
		char outbuff[MESSAGE_SIZE];
		fgets(outbuff, MESSAGE_SIZE, stdin); //select
//		sscanf(outbuff,"%s %s %s", var1, var2, var3);
		//var3 will hold everything till end of line
		sscanf(outbuff, "%4096s %4096s %4096[^\n]", var1, var2, var3);
		if (strcmp(var1, "User:")==0) {//socket
			sendMSG(sockfd, 1, var2, var3);
		} else if (strcmp(var1, "Password:")==0) {
			sendMSG(sockfd, 2, var2, var3);
		} else 	if (strcmp(var1, "list_of_files")==0) {
			sendMSG(sockfd, 3, var2, var3);
		} else if (strcmp(var1, "delete_file")==0) {
			sendMSG(sockfd, 4, var2, var3);
		} else 	if (strcmp(var1, "add_file")==0) {
			FILE* file = fopen(var2, "rw");
			if (file == NULL) {
				printf("Can`t open the file");
				break;
			}
			sendMSGTXT(sockfd, 5, var3, file);
			fclose(file);
		}
		else if (strcmp(var1, "get_file")==0) {
			sendMSG(sockfd, 6, var2, var3);
		}
		
		else if (strcmp(var1, "users_online")==0) {
			sendMSG(sockfd, 8, var2, var3);
		}
		
			else if (strcmp(var1, "msg")==0) {
			//remove ":" from message
			if (var2[strlen(var2)-1] == ':'){
				var2[strlen(var2)-1] = '\0';
			}
//			printf("sending message, var2:%s, var3:%s\n", var2, var3);
			sendMSG(sockfd, 9, var2, var3);
		}
			else if (strcmp(var1, "read_msgs")==0) {
			sendMSG(sockfd, 10, var2, var3);
		}
			else if (strcmp(var1, "quit")==0) {
			sendMSG(sockfd, 7, var2, var3);
			exit(0);
		}
		
		else
		{
			printf("ERROR: wrong command\n");
		}
	 }
	}

	return 0;
}
