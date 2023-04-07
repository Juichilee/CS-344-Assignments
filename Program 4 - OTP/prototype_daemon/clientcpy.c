#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg) { perror(msg); exit(0); } // Error function used for reporting issues

int main(int argc, char *argv[])
{
	int socketFD, portNumber, charsWritten, charsRead;
	struct sockaddr_in serverAddress;
	struct hostent* serverHostInfo;
	char buffer[100000];
	char buffer2[100000];
    
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
	//printf("CLIENT: Enter text to send to the server, and then hit enter: ");
	int size1;
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer array
	FILE *fp = fopen(argv[1], "r");
	if(fp == NULL){fprintf(stderr, "Error: input contains bad characters\n"); exit(1);};
	fgets(buffer, sizeof(buffer), fp);	
	//printf("buffer: %s", buffer);	
	size1 = strcspn(buffer, "\0");
	buffer[strcspn(buffer, "\n")] = '\0'; // Remove the trailing \n that fgets adds
	//printf("size1: %d\n", size1);

	// Get key from user
	int size2;
	memset(buffer2, '\0', sizeof(buffer2));
	FILE *fp2 = fopen(argv[2], "r");
	if(fp2 == NULL){fprintf(stderr, "Error: input contains bad characters\n"); exit(1);};
	fgets(buffer2, sizeof(buffer2), fp2);
	//printf("buffer2: %s", buffer2);
	size2 = strcspn(buffer2, "\0");
	buffer2[strcspn(buffer2, "\n")] = '\0';	
	//printf("size2: %d\n", size2);

	// Check input message size and key size.
	if(size1 > size2){fprintf(stderr, "Error: key '%s' is too short\n", buffer2); exit(1); }	
	
	strcat(buffer, "@");
	strcat(buffer, buffer2);
	strcat(buffer, "@@");
/*
	int i;
	for(i = 0; i < strlen(buffer); i++){
		printf("%c", buffer[i]);
	}
*/

	charsWritten = -1;
	while(charsWritten == -1){
	// Send message to server
		charsWritten = send(socketFD, buffer, sizeof(buffer), 0); // Write to the server
		if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
		if (charsWritten < strlen(buffer)) printf("CLIENT: WARNING: Not all data written to socket!\n");
	}
	// Get return message from server
	memset(buffer, '\0', sizeof(buffer)); // Clear out the buffer again for reuse
	charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); // Read data from the socket, leaving \0 at end
	if (charsRead < 0) error("CLIENT: ERROR reading from socket");
	//printf("CLIENT: I received this from the server: \"%s\"\n", buffer);
	fprintf(stdout, buffer);
	fprintf(stdout, "\n");	

	close(socketFD); // Close the socket
	return 0;
}
