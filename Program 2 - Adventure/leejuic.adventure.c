#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>

struct Room{
	char roomName[9];
	int roomType;
	char outBoundConnections[6][9];
	int numConnections;
};

struct node{
	int roomIndex;
	struct node *next;
};

char newestDirName[256];
struct Room rooms[7];
int currentRoomIndex = 0;
struct node *front;
struct node *rear;
pthread_mutex_t lock;

/*Linked List Functions for tracking room path*/
void insert(int);
void delete();
void display();

/*File Reading and Game functions*/
void GetRecentDirectory();
void ReadFiles();
void ReadFormattedLines(FILE*);
void* PrintTime();
void PlayGame();
int GameDisplay(struct Room);

/*Inserts the index(0-7) of a room into the linked list.*/
void insert(int nextRoomIndex){
	struct node *ptr;
	ptr = (struct node*)malloc(sizeof(struct node));
	if(ptr == NULL){
		printf("\nOVERFLOW\n");
		return;	
	}else{
		ptr->roomIndex = nextRoomIndex;
		if(front == NULL){
			front = ptr;
			rear = ptr;
			front->next = NULL;
			rear->next = NULL;
		}else{
			rear->next = ptr;
			rear = ptr;
			rear->next = NULL;
		}
	}
}

/*Deletes and frees the pointers in the linked list*/
void delete(){
	struct node *ptr;
	while(front != NULL){
		ptr = front;
		front = front->next;
		free(ptr);
	}
}

/*Displays the names of the rooms stored in the linked list to the screen*/
void display(){
	struct node *ptr;
	ptr = front;
	if(front == NULL){
		printf("\nEmpty queue\n");
	}else{
		while(ptr != NULL){
			printf("%s\n", rooms[ptr->roomIndex].roomName);
			ptr = ptr->next;
		}
	}
}

/*Finds the most recent room directory using stat to access the directory's attributes.*/
/*Saves the most recent directory's name to newestDirName.*/
void GetRecentDirectory(){
	int newestDirTime = -1;
	char targetDirPrefix[32] = "leejuic.rooms.";
	DIR* dirToCheck;
	struct dirent* fileInDir;
	struct stat dirAttributes;
	dirToCheck = opendir(".");
	
	if(dirToCheck > 0){
		while((fileInDir = readdir(dirToCheck))!= NULL){
			
			if(strstr(fileInDir->d_name, targetDirPrefix) != NULL){
				stat(fileInDir->d_name, &dirAttributes);
				if((int)dirAttributes.st_mtime > newestDirTime){
					newestDirTime = (int)dirAttributes.st_mtime;
					strcpy(newestDirName, fileInDir->d_name);
				}
			}
		}
	}
	closedir(dirToCheck);
}

/*Reads the files in the newest directory by finding their file path in relation to the current directory*/
/*the program is running in.*/
void ReadFiles(){
	DIR* roomsDir;
	struct dirent* roomFileInDir;
	FILE *fptr;
	char filePath[200];
	roomsDir = opendir(newestDirName);
	while((roomFileInDir = readdir(roomsDir)) != NULL){
		if(!strcmp(roomFileInDir->d_name, ".")){
			continue;
		}
		if(!strcmp(roomFileInDir->d_name, "..")){
			continue;
		}
		memset(filePath,0,sizeof(filePath));
		strcat(filePath, newestDirName); strcat(filePath, "/"); strcat(filePath, roomFileInDir->d_name);
		fptr = fopen(filePath, "r");

		ReadFormattedLines(fptr);	
	}	
	closedir(roomsDir);
}

/*Takes a file and reads its formatted room data into a room struct, and then stores the room struct into rooms.*/
void ReadFormattedLines(FILE* fptr){
	struct Room newRoom;
	char line[100];
	char dataLine[30];
	int curConnectionIndex = 0;
	while(fgets(line, sizeof line, fptr) != NULL){
			
		sscanf(line, "%*s%*s%s", dataLine);
		if(strstr(line, "ROOM NAME") != NULL){
			strcpy(newRoom.roomName, dataLine);
				
		}else if(strstr(line, "CONNECTION") != NULL){
			strcpy(newRoom.outBoundConnections[curConnectionIndex], dataLine);
			curConnectionIndex++;				
		}else if(strstr(line,"ROOM TYPE")){
			if(strstr(dataLine, "START_ROOM") != NULL){
				newRoom.roomType = 0;
			}else if(strstr(dataLine, "END_ROOM") != NULL){
				newRoom.roomType = 1;
			}else{
				newRoom.roomType = 2;
			}
		}
	}
	newRoom.numConnections = curConnectionIndex;
	rooms[currentRoomIndex] = newRoom;
	currentRoomIndex++;
	fclose(fptr);
}

/*Displays the Game State and prompts the user for next room they want to move to.*/
/*When the user types in "time", a new thread is created to process the time.*/
int GameDisplay(struct Room currentRoom){
	char inputLocation[10];	
	int roomIndex = -1;
	int i;
	printf("CURRENT LOCATION: %s\n", currentRoom.roomName);
	printf("POSSIBLE LOCATIONS:");
	for(i = 0; i < currentRoom.numConnections; i++){
		if(i == (currentRoom.numConnections-1)){
			printf(" %s.\n", currentRoom.outBoundConnections[i]);
		}else{
			printf(" %s,", currentRoom.outBoundConnections[i]);
		}
	}
	printf("WHERE TO? > ");	
	scanf("%s", inputLocation);

	if(strcmp(inputLocation, "time") == 0){
		pthread_t tid;
		if(pthread_mutex_init(&lock, NULL) != 0){
			printf("\nMutex init has failed\n");
		}
		pthread_create(&tid, NULL, PrintTime, NULL);	
		
		pthread_join(tid, NULL);	
		return roomIndex;	
	}
	
	for(i = 0; i < currentRoom.numConnections; i++){
		if(strcmp(inputLocation, currentRoom.outBoundConnections[i]) == 0){
			int j;
			for(j = 0; j < 7; j++){
				if(strcmp(rooms[j].roomName, currentRoom.outBoundConnections[i])== 0){
					roomIndex = j;
					break;
				}
			}
			break;
		}
	}
	if(roomIndex == -1){
		printf("\nHUH? I DON'T UNDERSTAND THAT ROOM. TRY AGAIN.\n\n");
	}else{
		printf("\n");
	}
	return roomIndex;
}

/*The function used by the thread in GameDisplay() to calculate the current time, print it to the screen,*/
/*and save it to currentTime.txt. Uses mutex to access global variables.*/
void* PrintTime(){
	pthread_mutex_lock(&lock);
	time_t t;
	struct tm *tmp;
	char time_str[200];
	
	time(&t);
	tmp = localtime(&t);
	strftime(time_str, 100, " %I:%M%p, %A, %B %d, %Y\n", tmp);
	printf("\n%s\n", time_str);

	FILE *fptr;
	fptr = fopen("currentTime.txt", "w+");
		fprintf(fptr, time_str);
	fclose(fptr);

	pthread_mutex_unlock(&lock);	
	return NULL;
}

/*Initiates the Game State and checks if the user has reached the win condition.*/ 
/*Also keeps track of the user's current room, the number */
/*of steps they have taken, and the room path they took to reach the end.*/
void PlayGame(){
	int curRoomIndex = 0;
	int numSteps = 0;
	
	while(1){	
		struct Room currentLocation = rooms[curRoomIndex];
		
		if(currentLocation.roomType == 1){
			break;
		}
		
		int roomIndex =	GameDisplay(currentLocation);
		if(roomIndex == -1){
			continue;
		}
		curRoomIndex = roomIndex;
		insert(curRoomIndex);
		numSteps++;	
	}
	printf("YOU HAVE FOUND THE END ROOM. CONGRATULATIONS!\nYOU TOOK %d STEPS. YOUR PATH TO VICTORY WAS:\n", numSteps);	
	display();
	delete();
	
}

int main(){
	GetRecentDirectory();
	ReadFiles();
	PlayGame();	
	
	return 0;
}
