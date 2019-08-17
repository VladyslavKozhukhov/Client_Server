//
// Created by arthur on 11/15/17.
//

#include <stdio.h>
#include <sys/socket.h>
#include <memory.h>
#include "TCP_helpers.h"
#include "constants.h"


/**
 * Reads from socket until full message is received.
 * @param socket
 * @param buffer
 * @param size
 * @return total bytes read or -1 if failed.
 */
ssize_t readEntireMessage(int socket, char *buffer, int size){
	ssize_t read_size;
	ssize_t total_read=0;
	while (total_read < size){
		read_size = recv(socket, buffer+total_read, MESSAGE_SIZE - total_read, 0);
		if (read_size < 0){
			return -1;
		}
		total_read += read_size;
	}
//	printf("DEBUG: received message of size %d: \n%s\n", total_read ,buffer+1);
	return total_read;
}


int sendFully(int socket, char *buf, size_t *len) {
	size_t total = 0; /* how many bytes we've sent */
	size_t bytesleft = *len; /* how many we have left to send */
	ssize_t n = 0;
	while(total < *len) {
		n = send(socket, buf+total, bytesleft, 0);
		if (n == -1) { break; }
		total += n;
		bytesleft -= n;
	}
	*len = total; /* return number actually sent here */
//	printf("DEBUG:Sent message of size:%d\n", total);
	return n == -1 ? -1:0; /*-1 on failure, 0 on success */
}
void sendMSGTXT(int sockfd, char msgType, char* newFileName, FILE *file) {
	size_t len = MESSAGE_SIZE;
	char filebuffer[FILE_MAX_SIZE] = { 0 };
	char sendBuffer[MESSAGE_SIZE] = { 0 };
	ssize_t read_size = 0;
	size_t total_read = 0;
	while (total_read < FILE_MAX_SIZE) {
		read_size = fread(filebuffer, 1, FILE_MAX_SIZE - total_read, file);
		if (read_size < 0) {
			return;
		}
		if (read_size == 0){ //eof
			total_read += read_size;
			break;
		}
		total_read += read_size;
	}
	sendBuffer[0] = msgType;
	strncpy(sendBuffer + MSG_ARGUMENT_OFFSET, newFileName, MAX_STRING_LEN);
	memcpy(sendBuffer + MSG_FILE_CONTENTS_OFFSET, filebuffer, FILE_MAX_SIZE);

	sendBuffer[FILE_SIZE_OFFSET] = (char) (0xFF & (total_read /100));
	sendBuffer[FILE_SIZE_OFFSET + 1] = (char) (0xFF & ((total_read /10) % 10));
	sendBuffer[FILE_SIZE_OFFSET + 2] = (char) (0xFF & (total_read)%10);

	sendFully(sockfd, sendBuffer, &len);
}

void sendMSG(int sockfd, char msgType, char* var1, char* var2)
{
	size_t len = MESSAGE_SIZE;
	char sendBuffer[MESSAGE_SIZE] = { 0 };
	if (msgType != 6 && msgType!=9 && msgType != 10) {
		sendBuffer[0] = msgType;
		strncpy(sendBuffer + MSG_ARGUMENT_OFFSET, var1, MAX_STRING_LEN);//add 9
	}
	else if(msgType == 9 || msgType == 10){
		sendBuffer[0] = msgType;
		strncpy(sendBuffer + MSG_ARGUMENT_OFFSET, var1,  strlen(var1));//add 9
		memcpy(sendBuffer + MSG_FILE_CONTENTS_OFFSET, var2, strlen(var2));
	}
	else{
		//sending a file
		sendBuffer[0] = msgType;
		strncpy(sendBuffer + MSG_ARGUMENT_OFFSET, var1, MAX_STRING_LEN);
		memcpy(sendBuffer + MSG_FILE_CONTENTS_OFFSET, var2, FILE_MAX_SIZE);
	}
	sendFully(sockfd, sendBuffer, &len);
}
