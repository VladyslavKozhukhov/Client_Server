//
// Created by arthur on 1/4/18.
//

#ifndef PRACTICAL_2_USER_DATA_H
#define PRACTICAL_2_USER_DATA_H

#include <stdbool.h>
#include "constants.h"

typedef  struct u_data_st{
	char user[MAX_STRING_LEN];
	char pwd[MAX_STRING_LEN];
	char userPath[PATH_MAX];
	bool isLoggedIn;
	int socket;
} user_data;


#endif //PRACTICAL_2_USER_DATA_H
