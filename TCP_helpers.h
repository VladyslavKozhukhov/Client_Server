//
// Created by arthur on 11/15/17.
//

#ifndef SERVER_TCP_HELPERS_H
#define SERVER_TCP_HELPERS_H

ssize_t readEntireMessage(int socket, char *buffer, int size);
int sendFully(int socket, char *buf, size_t *len) ;
void sendMSGTXT(int sockfd, char msgType, char* newFileName, FILE *file);
void sendMSG(int sockfd, char msgType, char* var1, char* var2);
#endif //SERVER_TCP_HELPERS_H
