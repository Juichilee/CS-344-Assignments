#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg) { perror(msg); exit(1); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;

	char buffer[1000000]; // char buffer for storing encrypted string to be decrypted
	char buffer2[1000000]; // char buffer for storing key string
	char completeBuffer[1000000];  // char buffer for storing complete decrypted message from decryption daemon
	char readBuffer[10]; // char buffer for reading in completeBuffer in chunks

	if (argc < 4) { fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(0); } // Check usage & args

	// Set up the server address struct
	memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
	portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
	serverAddress.sin_family = AF_INET; // Create a network-capable socket
	serverAddress.sin_port = htons(portNumber); // Store the port number
	serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
	if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
	memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

	// Set up the socket
	socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
	if (socketFD < 0) error("CLIENT: ERROR opening socket");
	
	// Connect to server
	if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to address
		error("CLIENT: ERROR connecting");

	// Get input message file from user
	int size1;
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array
	FILE *fp = fopen(argv[1], "r");
	if(fp == NULL){fprintf(stderr, "Error: input contains bad characters\n"); exit(1);}; // Error handling for file
	fgets(buffer, sizeof(buffer), fp);	

	size1 = strcspn(buffer, "\0"); // Get the size of the message string
	buffer[strcspn(buffer, "\n")] = '\0'; // Remove the trailing \n that fgets adds


	// Get key from user
	int size2;
	memset(buffer2, '\0', sizeof(buffer2));
	FILE *fp2 = fopen(argv[2], "r");
	if(fp2 == NULL){fprintf(stderr, "Error: input contains bad characters\n"); exit(1);}; // Error handling for file
	fgets(buffer2, sizeof(buffer2), fp2);

	size2 = strcspn(buffer2, "\0");
	buffer2[strcspn(buffer2, "\n")] = '\0';	

	// Check input message size and key size.
	if(size1 > size2){fprintf(stderr, "Error: key '%s' is too short\n", buffer2); exit(1); }	
	
	// Formatting buffer to be sent to decryption daemon
	strcat(buffer, "@");
	strcat(buffer, "d"); 
	strcat(buffer, "@");
	strcat(buffer, buffer2);
	strcat(buffer, "@@");

	// Send message to server
	charsWritten = send(socketFD, buffer, sizeof(buffer), 0); // Write to the server
	if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
	if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");
	
	// Get return message from server
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	memset(completeBuffer, '\0', sizeof(completeBuffer));
	
	int r;
	while(strstr(completeBuffer, "@@") == NULL){
		memset(readBuffer, '\0', sizeof(readBuffer));
		r = recv(socketFD, readBuffer, sizeof(readBuffer)-1, 0);
		strcat(completeBuffer, readBuffer);
		if(strstr(completeBuffer, "d") != NULL){ // Error occurs if the server connected to is not the decryption daemon
			r = -2;
			break;
		}

		if(r == -1){error("ERROR: r == -1\n"); break;}
		if(r == 0){error("ERROR: r == 0\n"); break;}
	}
	charsRead = r;
	if (charsRead == -2) error("ERROR: cannot connect to encryption daemon\n");
	if (charsRead < 0) error("ERROR reading from socket");

	int terminalLocation = strstr(completeBuffer, "@@") - completeBuffer;
	completeBuffer[terminalLocation] = '\0';

	if (charsRead < 0) error("CLIENT: ERROR reading from socket");

	fprintf(stdout, completeBuffer);
	fprintf(stdout, "\n");	

	close(socketFD); // Close the socket
	return 0;
}
