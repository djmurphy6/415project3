/*
 * string_parser.c
 *
 *  Created on: Nov 25, 2020
 *      Author: gguan, Monil
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "string_parser.h"

#define _GUN_SOURCE

int count_token (char* buf, const char* delim)
{
	//check for null string
	if(buf == NULL){
		return 0;
	}

	//copy buffer into string

	char *bufString = malloc(strlen(buf)+1);
	if (bufString == NULL) {
    return -1;
	}

	strcpy(bufString, buf);
	//bufString[strlen(buf)] = '\0';

	//Iterate through string counting tokens
	int tnum = 0;
	char *saveptr;
	char * myPtr = strtok_r(bufString, delim, &saveptr);
	while(myPtr != NULL){
  	tnum += 1;
  	myPtr = strtok_r(NULL, delim, &saveptr);
	}

	free(bufString);
	return tnum;
}

command_line str_filler (char* buf, const char* delim)
{
	//create command_line variable
	command_line clvar;
	for(int i=0; i < strlen(buf); i++){
		if(buf[i] == '\n')
			buf[i] = '\0';
	}
	clvar.num_token = count_token(buf, delim);
	//printf("String %s", buf);
	
	//malloc memory for token array
	clvar.command_list = (char **)malloc((clvar.num_token + 1)*sizeof(char *));

	char *saveptr;
	//printf("Buffer: %s", buf);
	char *token = strtok_r(buf, delim, &saveptr);
	for(int i=0; i<clvar.num_token; i++) {
		if (token == NULL) {
        // Handle the case where there are fewer tokens than expected
        return clvar; //i may want to handle this better
    }
		clvar.command_list[i] = (char *)malloc(((strlen(token))+1)*sizeof(char));
		strcpy(clvar.command_list[i], token);
		clvar.command_list[i][strlen(token)] = '\0';
		token = strtok_r(NULL, delim, &saveptr);
	}
	clvar.command_list[clvar.num_token] = NULL;

	return clvar;

	
	//TODO：
	/*
	*	#1.	create command_line variable to be filled and returned
	*	#2.	count the number of tokens with count_token function, set num_token. 
    *           one can use strtok_r to remove the \n at the end of the line.
	*	#3. malloc memory for token array inside command_line variable
	*			based on the number of tokens.
	*	#4.	use function strtok_r to find out the tokens 
    *   #5. malloc each index of the array with the length of tokens,
	*			fill command_list array with tokens, and fill last spot with NULL.
	*	#6. return the variable.
	*/
}


void free_command_line(command_line* command)
{
	if (command != NULL && command->command_list != NULL) {
		for(int i=0; i < command->num_token; i++) {
			free(command->command_list[i]);
		}
		free(command->command_list);
	}
}
