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
	printf("Enter string to decrypt\n");
	memset(str, '\0', sizeof(str));
	memset(key, '\0', sizeof(key));
	fgets(str, sizeof(str), stdin);

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
	
	for(i = 0; i < strlen(key); i++){
		if(key[i] == ' '){
			numkey[i] = 26;
		}else{
			numkey[i] = key[i]-'A';
		}
	}

	for(i = 0; i < strlen(str); i++){
		numcipher[i] = numstr[i]-numkey[i];
		if(numcipher[i] < 0){
			numcipher[i] = numcipher[i] + 27;
		}
		numcipher[i] = numcipher[i]%27;
	}
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
