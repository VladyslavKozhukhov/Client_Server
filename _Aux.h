//
// Created by arthur on 11/17/17.
//

#ifndef CLIENT_AUX_H
#define CLIENT_AUX_H


#include <sys/types.h>
#include <stdio.h>

#define MAX(a,b) (a>b ? (a):(b))

//write exactly nbytes to fd.
ssize_t writeFully(FILE *fd, char buffer[], unsigned int nbytes);

//write up to len bytes of the contents to the file fname.
int writeFile(char *userPath, char *fname, char *contents, size_t len);

//remove given file. return 0 on success, 1 if file doesnt exist.
int removeFile(char *userPath, char *name);
#endif //CLIENT_AUX_H
