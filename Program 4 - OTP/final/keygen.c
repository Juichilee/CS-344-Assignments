#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

// Function Name: main
// Description: the main function. 
int main(int argv, char** argc){
	
	int length; // length of the key.
	char key[100000]; // char array to hold the key.
	memset(key, 0, sizeof(key));	// Reset the char array.

	length = atoi(argc[1]); // Turn first argument into integer and store into length.
	
	if(length == 0){ // Error handling length.
		fprintf(stderr, "Not valid integer or no integer input.\n");
		return 1;
	}
	
	int i;
	srand(time(0)); // Seed the random function.
	for(i = 0; i < length; i++){
		char randomLetter = "ABCDEFGHIJKLMNOPQRSTUVWXYZ "[rand()%27]; // choose a random letter or space.
		strncat(key, &randomLetter, 1); // append random letter to the key.
	}
	
        fprintf(stdout, "%s\n", key);	// fprint to stdout.
	return 0;
}
