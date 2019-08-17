#define _GNU_SOURCE

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <limits.h>
#include <dirent.h>

#include "constants.h"
#include "TCP_helpers.h"
#include "_Aux.h"
#include "user_data.h"

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

//this holds information specific per socket
static user_data users_sock_data[MAX_CLIENTS] = {{.user={0},.pwd={0},.socket=0,.isLoggedIn=0, .userPath={0}}};
static int next_sock_data_idx = 0;
static const char *users_base_dirname;

int readUsersFile(char users[MAX_USERS][2][MAX_STRING_LEN], const char *fpath){
	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	char curUser[MAX_STRING_LEN], curPass[MAX_STRING_LEN];
	fp = fopen(fpath, "r");
	if (fp == NULL)
		return -1;
	int idx = 0;
	while ((getline(&line, &len, fp)) != -1) {
		if (sscanf(line, "%s %s", curUser, curPass) != 2){
			//poorly formatted file
			return -1;
		}
		strcpy(users[idx][0], curUser);
		strcpy(users[idx][1], curPass);
		idx ++;
	}

	fclose(fp);
	if (line)
		free(line);
	return 0;
}

bool login(char users[MAX_USERS][2][MAX_STRING_LEN], char *user, char *password){
	int i=0;
	for (;i<MAX_USERS;i++){
		if (users[i][0][0] == '\0'){
			//didnt find the user
			return false;
		}
		if (strcmp(users[i][0], user) == 0){
			//user found!
			return (strcmp(users[i][1], password) == 0);
		}
	}
	return false;
}

//sends a string message to given socket
ssize_t sendMessage(int socket, char message[]){
	sendMSG(socket, 0, message, NULL);
	return 0;
}
//sends a file to given socket
ssize_t sendFile(int socket, FILE *contents){
	sendMSGTXT(socket, 6, "", contents);
	return 0;
}

//sends a chat message to given socket. if offline is given then msg type is 10, otherwise 9
ssize_t sendChat(int socket, char *origin_uname ,char contents[MAX_CHAT_LEN], int offline){
	sendMSG(socket, offline? (char)10:(char)9, origin_uname, contents);
	return 0;
}

//sends a users online message to given socket
ssize_t sendUsersOnline(int socket, char charlist[MAX_CHAT_LEN]){
	sendMSG(socket, 8, charlist, "");
	return 0;
}

/**
 * Count number of files in provided user dir
 * @param pathToDir
 * @param buffer
 */
int fileCount(char *pathToDir){
	DIR           *d;
	struct dirent *dir;
	d = opendir(pathToDir);
	int count = 0;
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0){
				//dont add . and ..
				continue;
			}
			count ++;
		}

		closedir(d);
	}
	return count;
}

void createFileList(char *pathToDir, char *buffer){
	DIR           *d;
	struct dirent *dir;
	d = opendir(pathToDir);
	//first clear buffer:
	buffer[0] = '\0';
	size_t offset=0;
	if (d)
	{
		while ((dir = readdir(d)) != NULL)
		{
			if (strcmp(dir->d_name, ".") == 0 || strcmp(dir->d_name, "..") == 0){
				//dont add . and ..
				continue;
			}
			offset += sprintf(buffer + offset, "%s\n", dir->d_name);
		}

		closedir(d);
	}
}

//find user index in the users_sock_data struct.
int getUserIndex(char *uname){
	for (int i=0;i<next_sock_data_idx;i++){
		if (users_sock_data[i].socket != 0 && (strcmp(users_sock_data[i].user, uname) == 0)){
			return i;
		}
	}
	return -1;
}

//return if given uname is online
int isUserOnline(char *uname){
	for (int i=0;i<next_sock_data_idx;i++){
		if (users_sock_data[i].socket != 0 && (strcmp(users_sock_data[i].user, uname) == 0)
			&& users_sock_data[i].isLoggedIn){
			return 1;
		}
	}
	return 0;
}


//Handle a message that the given client received. return-1 on error, 1 if user logged out and closed the socked, 0 otherwise.
ssize_t handleClient(user_data *sock_data, char users[MAX_USERS][2][50],const char *baseDir) {
	int socket = sock_data->socket;
	char buffer[MESSAGE_SIZE] = {0};
	char chat_buffer[MAX_CHAT_LEN] = {0};
	char tmp[PATH_MAX] = {0};
	ssize_t shouldLogout = 0;
	ssize_t res = readEntireMessage(socket, buffer, MESSAGE_SIZE);
	if (res < 0){
		return -1 ;
	}
	char msgType = *buffer;
	char *uname;
	int res2;
	FILE *fd = NULL;
	char *line;
	size_t len;
	if (msgType > 2 && !sock_data->isLoggedIn){
		sendMessage(socket, "Must be logged in to do that.\n");
		return 0;
	}
	switch (msgType){
		case 0:
			//message - do nothing
			break;
		case 1:
			//provided username.
			if (sock_data->isLoggedIn){
				sendMessage(socket, "Already logged in.\n");
				break;
			}
			strncpy(sock_data->user, buffer+MSG_ARGUMENT_OFFSET, MAX_STRING_LEN);
			break;
		case 2:
			//password
			if (sock_data->isLoggedIn){
				sendMessage(socket, "Already logged in.\n");
				break;
			}
			strncpy(sock_data->pwd, buffer+MSG_ARGUMENT_OFFSET, MAX_STRING_LEN);
			//try to authenticate:
			sock_data->isLoggedIn = login(users, sock_data->user, sock_data->pwd);
			if (sock_data->isLoggedIn) {
				//define cur user path
				strcpy(sock_data->userPath, baseDir);

				strcat(sock_data->userPath,"/");
				strcat(sock_data->userPath, sock_data->user);
				char msg[MAX_STRING_LEN];
				sprintf(msg, "Hi %s, you have %d files stored\n", sock_data->user, fileCount(sock_data->userPath));
				strncpy(tmp, msg, MAX_STRING_LEN);
			} else{
				strncpy(tmp, "Log in failed.\n", MAX_STRING_LEN);
			}
			sendMessage(socket, tmp);
			break;
		case 3:
			//list files
			createFileList(sock_data->userPath, tmp);
			sendMessage(socket, tmp);
			break;
		case 4:
			//remove file
			strncpy(tmp, buffer + MSG_ARGUMENT_OFFSET, MAX_STRING_LEN);
			res2 = removeFile(sock_data->userPath, tmp);
			if (res2 == 0) {
				strncpy(tmp, "File removed\n", MAX_STRING_LEN);
			} else{
				strncpy(tmp, "No such file exists!\n", MAX_STRING_LEN);
			}
			sendMessage(socket, tmp);
			break;
		case 5:
			//add file
			strncpy(tmp, buffer + MSG_ARGUMENT_OFFSET, MAX_STRING_LEN);
			//calc file size
			size_t size = 0;
			size += buffer[FILE_SIZE_OFFSET] * 100;
			size += buffer[FILE_SIZE_OFFSET+1] * 10;
			size += buffer[FILE_SIZE_OFFSET+2];
			writeFile(sock_data->userPath, tmp, buffer + MSG_FILE_CONTENTS_OFFSET, size);
			sendMessage(socket, "File added\n");
			break;
		case 6:
			strcpy(tmp, sock_data->userPath);
			strcat(tmp, "/");
			strncpy(tmp + strlen(tmp), buffer + MSG_ARGUMENT_OFFSET, MAX_STRING_LEN);
			FILE* file = fopen(tmp, "r");
			if (file ==NULL){
				printf("Could not open file\n");
				exit(1);
			}
			//send the file
			sendFile(socket, file);
			break;
		case 7:
			//logout
			sock_data->isLoggedIn = false;
			shouldLogout = true;
			break;
		case 8://get online user list
			memset(chat_buffer, 0, sizeof(chat_buffer));
			int isfirst = 1;
			for (int i=0;i<MAX_USERS;i++){
				if (isUserOnline(users[i][0])){
					//add user to online users
					if (isfirst){
						isfirst = 0;
						strcat(chat_buffer, users[i][0]);
						continue;
					}
					strcat(chat_buffer, ",");
					strcat(chat_buffer, users[i][0]);
				}
			}
			//finally, send the message:
			sendUsersOnline(socket, chat_buffer);
			break;
		case 9:
			//send message to user
			uname = &(buffer[MSG_ARGUMENT_OFFSET]);
			if (isUserOnline(uname)){
				int user_sock = users_sock_data[getUserIndex(uname)].socket;
				sendChat(user_sock, sock_data->user, &(buffer[MSG_FILE_CONTENTS_OFFSET]), false);
			}else{
				//user is offline, find his file and write message to it.
				//generate the user dir path:
				strcpy(tmp, users_base_dirname);
				strcat(tmp, uname);
				strcat(tmp,"/");
				strcat(tmp, OFFLINE_MSG_FILENAME);
				fd = fopen(tmp, "a");
				if (fd == NULL){
					perror("ERROR opening file");
					//dont fail- maybe user doesnt exist.
					return (0);
				}
				writeFully(fd, sock_data->user, (unsigned int) strlen(sock_data->user));
				writeFully(fd, " ", 1);
				writeFully(fd, &(buffer[MSG_FILE_CONTENTS_OFFSET]), (unsigned int)strlen(&(buffer[MSG_FILE_CONTENTS_OFFSET])));
				writeFully(fd, "\n", 1);
				fclose(fd);
			}
			break;
		case 10:
			//read messages
			//generate the user dir path:
			strcpy(tmp, users_base_dirname);
			strcat(tmp, sock_data->user);
			strcat(tmp,"/");
			strcat(tmp, OFFLINE_MSG_FILENAME);
			fd = fopen(tmp, "r");
			line = NULL;
			if (fd == NULL){
				perror("ERROR opening file");
				return (-1);
			}
			while(getline(&line, &len, fd) > 0){
				//line should now hold "uname message"
				char *msg = strchr(line, ' ');
				if (msg == NULL){
					printf("Poorly formatted file");
					return -1;
				}
				memcpy(buffer, line, msg-line);
				buffer[msg-line] = '\0';
				sendChat(socket, buffer, msg, true);
			}
			//delete file
			freopen(tmp, "w", fd);
			//close handlers
			if (line != NULL) {
				free(line);
			}
			fclose(fd);
			break;
		default:
			//unknown command
			sendMessage(socket, "Unknown command");
			break;
	}
	return shouldLogout;
}



int main(int argc, char** argv) {
	int sock = socket(PF_INET, SOCK_STREAM, 0);
	struct sockaddr_in myaddr, client_adrr;
	int port = 1337; //default port
	struct timeval timeout;
	//user-password
	char users[MAX_USERS][2][MAX_STRING_LEN];
	if (argc == 4){
		// port is provided
		port = atoi(argv[3]);
	}
	/////////init///////////////
	users_base_dirname = argv[2];
	const char *usersFile = argv[1];
	readUsersFile(users, usersFile);
	char tmp[PATH_MAX];
	//create users folders
	int i=0;
	for (;i<MAX_USERS;i++){
		if (users[i][0][0] == '\0'){
			//end of the users array
			break;
		}
		//else - create the folder
		strcpy(tmp, users_base_dirname);

		strcat(tmp,"/");
		strcat(tmp, users[i][0]);
		//check if dir exists already
		DIR* dir = opendir(tmp);
		if (dir)
		{
			/* Directory exists. */
			closedir(dir);
		}
		else {
			if (mkdir(tmp, S_IRUSR | S_IWUSR | S_IXUSR) != 0) {
				printf("Failed to mkdir '%s'\n", tmp);
				exit(1);
			}
		}
		//create offline msgfile
		strcat(tmp, "/");
		strcat(tmp, OFFLINE_MSG_FILENAME);
		FILE* file = fopen(tmp, "w+");
		if (file ==NULL){
			printf("Could not create offline messages file!\n");
			return -1;
		}
		fclose(file);

	}

	////////////////////////////
	myaddr.sin_family = AF_INET;
	myaddr.sin_port = htons(port);
	myaddr.sin_addr.s_addr = INADDR_ANY;
	bind(sock, (struct sockaddr *)&myaddr, sizeof(myaddr));
	listen(sock, 5);
	while (1) {
		//getFromSockData

		socklen_t sin_size = sizeof(struct sockaddr_in);
		//generate fd list for select
		fd_set set;
		int rv;
		FD_ZERO(&set); /* clear the set */
		FD_SET(sock, &set); /* add our file descriptor to the set */
		int maxSock = sock;
		//add all non zero user_data
		for (int j=0;j<next_sock_data_idx; j++){
			if (users_sock_data[j].socket != 0){
				FD_SET(users_sock_data[j].socket, &set);
				maxSock = MAX(maxSock, users_sock_data[j].socket);
			}
		}
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		rv = select(maxSock + 1, &set, NULL, NULL, &timeout);
		if(rv == -1)
		{
			perror("Error in select"); /* an error occurred */
			return 1;
		} else{
			//can read at least some sockets.
			//first try to accept new client
			if (FD_ISSET(sock, &set)){
				int new_sock = accept(sock, (struct sockaddr *) &client_adrr, &sin_size);
				users_sock_data[next_sock_data_idx].socket = new_sock;
				//send please log in message
				sendMessage(new_sock, "Welcome! Please log in.\n");
				next_sock_data_idx++;
			}
			//now see if we received any messages
			for (int j=0;j<next_sock_data_idx; j++){
				if (users_sock_data[j].socket != 0 && FD_ISSET(users_sock_data[j].socket, &set)){
					//handle a message in this socket
					ssize_t shouldClose;
					shouldClose = handleClient(&(users_sock_data[j]), users, users_base_dirname);
					if (shouldClose){
						close(users_sock_data[j].socket );
						users_sock_data[j].socket = 0;
					}
				}
			}
		}
	}
}