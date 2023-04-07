#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	char completeBuffer[1000000];
	char buffer[1000000];
	char buffer2[1000000];
	char readBuffer[10];
	char encrypted[1000000];

	char* str = (char*)malloc(1000000 * sizeof(char));
	char* key = (char*)malloc(1000000 * sizeof(char));
	char* cipher = (char*)malloc(1000000 * sizeof(char));
	int* numstr = (int*)malloc(1000000 * sizeof(int));
	int* numkey = (int*)malloc(1000000 * sizeof(int));
	int* numcipher = (int*)malloc(1000000 * sizeof(int));
	
	struct sockaddr_in serverAddress, clientAddress;

	if (argc < 2) { fprintf(stderr,"USAGE: %s port\n", argv[0]); exit(1); } // Check usage & args

	// Set up the address struct for this process (the server)
	memset((char *)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[1]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverAddress.sin_addr.s_addr = INADDR_ANY; // Any address is allowed for connection to this process

	// Set up the socket
	listenSocketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (listenSocketFD < 0) error("ERROR opening socket");

	// Enable the socket to begin listening
	if (bind(listenSocketFD, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to port
		error("ERROR on binding");
	listen(listenSocketFD, 5); // Flip the socket on - it can now receive up to 5 connections

	// Accept a connection, blocking if one is not available until one connects
	sizeOfClientInfo = sizeof(clientAddress); // Get the size of the address for the client that will connect
	establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo); // Accept
	if (establishedConnectionFD < 0) error("ERROR on accept");

	// Get the message from the client and display it
	memset(completeBuffer, '\0', sizeof(completeBuffer));
	
	int r;
	while(strstr(completeBuffer, "@@") == NULL){
		memset(readBuffer, '\0', sizeof(readBuffer));
		r = recv(establishedConnectionFD, readBuffer, sizeof(readBuffer)-1, 0);
		strcat(completeBuffer, readBuffer);
		//printf("PARENT: message received from child: %s, total: %s\n", readBuffer, buffer);
		if(r == -1){printf("ERROR: r == -1\n"); break;}
		if(r == 0){printf("ERROR: r == 0\n"); break;}
	}
	charsRead = r;
	//charsRead = recv(establishedConnectionFD, buffer, sizeof(buffer), 0); // Read the client's message from the socket
	if (charsRead < 0) error("ERROR reading from socket");
	printf("SERVER: I received this from the client: \"%s\"\n", completeBuffer);
	int terminalLocation = strstr(completeBuffer, "@@") - completeBuffer;
	completeBuffer[terminalLocation] = '\0';
	
	memset(buffer, '\0', sizeof(buffer));
	memset(buffer2, '\0', sizeof(buffer2));

	int bufferSize = strlen(completeBuffer);
	char delim[] = "@";
	char *ptr = strtok(completeBuffer, delim);
	int count = 0;
	while(ptr != NULL){
		if(count == 0){
			sprintf(buffer, "%s", ptr);
		}else{
			sprintf(buffer2, "%s", ptr);
		}
		ptr = strtok(NULL, delim);
		count++;
	}
	//printf("Done splitting string\n");
	//printf("buffer: %s\n", buffer);
	//printf("buffer2: %s\n", buffer2);	
	if(charsRead > 0){

		memset(str, '\0', sizeof(str));
		memset(key, '\0', sizeof(key));
		strcpy(str, buffer);
		//printf("str: %s\n", str);	
/*
		int size;
		size = strcspn(str, "\0");
		str[strcspn(str, "\n")] = '\0'; 
*/	
		int i;
		for(i = 0; i < strlen(str); i++){
			if(str[i] == ' '){
				numstr[i] = 27;
			}else{
				numstr[i] = str[i]-'A';
			}
		}
		
		//printf("Enter random text for key\n");
		//fgets(key, sizeof(key), stdin);
		strcpy(key, buffer2);	

		for(i = 0; i < strlen(key); i++){
			if(key[i] == ' '){
				numkey[i] = 27;
			}else{
				numkey[i] = key[i]-'A';
			}
		}

		for(i = 0; i < strlen(str); i++){
			numcipher[i] = numstr[i]+numkey[i];
		}
		for(i = 0; i < strlen(str); i++){
			if(numcipher[i] > 26){
				numcipher[i] = numcipher[i]-27;
			}
		}
		
		char ch;
		//printf("Ciphertext is\n");
		for(i = 0; i < strlen(str); i++){
			if(numcipher[i] == 26){
				strcat(encrypted, " ");
			}else{
				ch = (numcipher[i]+'A');
				strncat(encrypted, &ch, 1);
				//strcat(encrypted, "%c", (numcipher[i]+'A'));
			}
		}
		//printf("encrypted: %s\n", encrypted);

	}
	free(str);
	free(key);
	free(cipher);
	free(numstr);
	free(numkey);
	free(numcipher);	
	
	printf("Encrypted String size: %d\n", strlen(encrypted));
	// Send a Success message back to the client
	charsRead = send(establishedConnectionFD, encrypted, sizeof(encrypted), 0); // Send success back
	if (charsRead < 0) error("ERROR writing to socket");
	close(establishedConnectionFD); // Close the existing socket which is connected to the client
	close(listenSocketFD); // Close the listening socket
	return 0; 
}
