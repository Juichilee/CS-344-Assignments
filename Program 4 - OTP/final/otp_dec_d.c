#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) { perror(msg);} // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int listenSocketFD, establishedConnectionFD, portNumber, charsRead;
	socklen_t sizeOfClientInfo;
	char completeBuffer[1000000]; // Char buffer for storing incoming message from otp_dec
	char buffer[1000000]; // Char buffer for storing encrypted message
	char buffer2[1000000]; // Char buffer for storing key
	char buffer3[4]; // Char buffer for storing client flag
	char readBuffer[10]; // Helps read in completeBuffer in chunks
	char decrypted[1000000]; // Char buffer for storing decrypted message

	// Assisting variables for decrypting message
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
	while((establishedConnectionFD = accept(listenSocketFD, (struct sockaddr *)&clientAddress, &sizeOfClientInfo)) > 0){
		if(fork() == 0){
			close(listenSocketFD);
			if (establishedConnectionFD < 0) error("ERROR on accept");

			// Get the message from the client and display it
			memset(completeBuffer, '\0', sizeof(completeBuffer));
			
			int r;
			while(strstr(completeBuffer, "@@") == NULL){
				memset(readBuffer, '\0', sizeof(readBuffer));
				r = recv(establishedConnectionFD, readBuffer, sizeof(readBuffer)-1, 0);
				strcat(completeBuffer, readBuffer);
				if(r == -1){printf("ERROR: r == -1\n"); break;}
				if(r == 0){printf("ERROR: r == 0\n"); break;}
			}
			charsRead = r;
			if (charsRead < 0) error("ERROR reading from socket");

			int terminalLocation = strstr(completeBuffer, "@@") - completeBuffer;
			completeBuffer[terminalLocation] = '\0';
			
			memset(buffer, '\0', sizeof(buffer));
			memset(buffer2, '\0', sizeof(buffer2));
			memset(buffer3, '\0', sizeof(buffer3));		
	
			// Picking apart the formatted completeBuffer and storing relevant info into buffer, buffer2, and buffer3
			int bufferSize = strlen(completeBuffer);
			char delim[] = "@";
			char *ptr = strtok(completeBuffer, delim);
			int count = 0;
			while(ptr != NULL){
				if(count == 0){
					sprintf(buffer, "%s", ptr);
				}else if(count == 1){
					sprintf(buffer3, "%s", ptr);
				}else{
					sprintf(buffer2, "%s", ptr);
				}
				ptr = strtok(NULL, delim);
				count++;
			}
	
			if(charsRead > 0 && strstr(buffer3, "d") != NULL){ // If the recv function succeeded and message was received from the correct client
				memset(str, '\0', sizeof(str));
				memset(key, '\0', sizeof(key));
				strcpy(str, buffer);
				
				int i;
				for(i = 0; i < strlen(str); i++){
					if(str[i] == ' '){
						numstr[i] = 26;
					}else{
						numstr[i] = str[i]-'A';
					}
				}
			
				strcpy(key, buffer2);
			
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
			
				char ch;

				for(i = 0; i < strlen(str); i++){
					if(numcipher[i] == 26){
						strcat(decrypted, " ");
					}else{
						ch = (numcipher[i]+'A');
						strncat(decrypted, &ch, 1);
					}
				}
				strcat(decrypted, "@@");
			}else{ // If client was not correct, then send back their client flag
				strcpy(decrypted, buffer3);
				strcat(decrypted, "@@");
			}
			free(str);
			free(key);
			free(cipher);
			free(numstr);
			free(numkey);
			free(numcipher);	
			
			// Send a message back to the client
			charsRead = send(establishedConnectionFD, decrypted, sizeof(decrypted), 0); // Send message back
			if (charsRead < 0) error("ERROR writing to socket");
		}
		close(establishedConnectionFD);
	}
	return 0; 
}
