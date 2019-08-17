//
// Created by arthur on 11/17/17.
//

#include "_Aux.h"
#include "constants.h"
#include <limits.h>
#include <memory.h>

//writes exactly nbytes to fd, unless an error occured. returns -1 on error, or #of written bytes if no error.
ssize_t writeFully(FILE *fd, char buffer[], unsigned int nbytes){
	size_t totalWritten=0;
	ssize_t bytesWritten;
	while ((bytesWritten = fwrite(buffer + totalWritten,1, nbytes - totalWritten, fd)) != 0) {
		if (bytesWritten < 0){
			return -1;
		}
		totalWritten += bytesWritten;
	}
	return (ssize_t) totalWritten;
}

//write up to len bytes of the contents to the file fname.
int writeFile(char *userPath, char *fname, char *contents, size_t len){
	char tmp[PATH_MAX] = {0};
	strcpy(tmp, userPath);
	if (fname != NULL) {
		strcat(tmp, "/");
		strcat(tmp, fname);
	}
	FILE* file = fopen(tmp, "w+");
	if (file ==NULL){
		printf("could not create file!\n");
		return -1;
	}
	size_t total_write = 0;
	ssize_t write_size=0;
	while (total_write<len)
	{
		write_size=fwrite(contents, 1, len-total_write, file);
		if (write_size < 0) {
			return -1;
		} else if (write_size == 0){ //eof
			break;
		}
		total_write += write_size;
	}
	fclose(file);
	return 0;
}


//remove given file. return 0 on success, 1 if file doesnt exist.
int removeFile(char *userPath, char *name){
	char tmp[PATH_MAX] = {0};
	strcpy(tmp, userPath);
	if (name != NULL){
		strcat(tmp, "/");
		strcat(tmp, name);
	}
	return remove(tmp);
}