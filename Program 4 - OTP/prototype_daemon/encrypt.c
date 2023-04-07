#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

int main(int argc, char *argv[])
{
	char str[100], key[100], cipher[100];
	int numstr[100], numkey[100], numcipher[100];
	char buffer[100];
	char buffer2[100];
	printf("Enter string to encrypt\n");
	memset(str, '\0', sizeof(str));
	memset(key, '\0', sizeof(key));
	fgets(str, sizeof(str), stdin);
/*	
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array
	FILE *fp = fopen(argv[1], "r");
	if(fp == NULL){fprintf(stderr, "Error: input contains bad characters\n"); exit(1);};
	fgets(buffer, sizeof(buffer), fp);	
	//printf("buffer: %s", buffer);	
	buffer[strcspn(buffer, "\n")] = '\0'; // Remove the trailing \n that fgets adds
	//printf("size1: %d\n", size1);

	strcpy(str, argv[1]);
*/	
	int size;
	size = strcspn(str, "\0");
	str[strcspn(str, "\n")] = '\0'; 
	
	int i;
	for(i = 0; i < strlen(str); i++){
		if(str[i] == ' '){
			numstr[i] = 26;
		}else{
			numstr[i] = str[i]-'A';
		}
	}
	
	printf("Enter random text for key\n");
	fgets(key, sizeof(key), stdin);
/*	
	memset(buffer2, '\0', sizeof(buffer2)); // Clear out the buffer array
	FILE *fp2 = fopen(argv[2], "r");
	if(fp2 == NULL){fprintf(stderr, "Error: input contains bad characters\n"); exit(1);};
	fgets(buffer2, sizeof(buffer2), fp2);	
	//printf("buffer: %s", buffer);	
	buffer2[strcspn(buffer2, "\n")] = '\0'; // Remove the trailing \n that fgets adds
	//printf("size1: %d\n", size1);

	strcpy(key, argv[2]);
*/	
	for(i = 0; i < strlen(key); i++){
		if(key[i] == ' '){
			numkey[i] = 26;
		}else{
			numkey[i] = key[i]-'A';
		}	
	}

	for(i = 0; i < strlen(str); i++){
		numcipher[i] = numstr[i]+numkey[i];	
		printf("%d ", numstr[i]);
	}

	printf("\n");
	for(i = 0; i < strlen(str); i++){
		if(numcipher[i] > 26){
			numcipher[i] = numcipher[i]-27;
		}
	}

	printf("Ciphertext is\n");
	for(i = 0; i < strlen(str); i++){
		if(numcipher[i] == 26){
			printf(" ");
		}else{
			printf("%c", (numcipher[i]+'A'));
		}
	}
	printf("\n");

	return 0;
}
