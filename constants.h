//
// Created by arthur on 11/15/17.
//

#ifndef SERVER_CONSTANTS_H
#define SERVER_CONSTANTS_H

#define MAX_STRING_LEN 50
#define MAX_USERS 15

#define MAX_CLIENTS 100

#define MAX_CHAT_LEN 500
//this is how the message is structured:

#define MSG_ARGUMENT_OFFSET 1
#define MSG_FILE_CONTENTS_OFFSET 51
#define FILE_MAX_SIZE 800
#define FILE_SIZE_OFFSET (FILE_MAX_SIZE + MSG_FILE_CONTENTS_OFFSET + MSG_ARGUMENT_OFFSET)

#define MESSAGE_SIZE (FILE_SIZE_OFFSET + 3)

#define OFFLINE_MSG_FILENAME "Messages_received_offline.txt"

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#endif //SERVER_CONSTANTS_H
