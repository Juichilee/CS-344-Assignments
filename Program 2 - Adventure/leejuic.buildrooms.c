#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>

struct Room{
	char roomName[9];
	int roomType;
	char outBoundConnections[6][9];
	int numConnections;
};

struct Room chosenRooms[7];
char roomNames[10][9] = {"XYZZY", "PLUGH", "PLOVER", "Twisty", "Zork", "Crowther", "Dungeon", "Fatroom", "Sealroom", "Elfroom" };

/*Functions for building a room directory and filling them with connected rooms*/
int IsGraphFull();
void AddRandomConnection();
struct Room* GetRandomRoom();
int CanAddConnectionFrom(struct Room);
int ConnectionAlreadyExists(struct Room, struct Room);
void ConnectRoom(struct Room*, struct Room*);
int IsSameRoom(struct Room, struct Room);

/* Returns 1 if all rooms have 3 to 6 outbound connections, false otherwise.*/
int IsGraphFull(){
	int i;
	for(i=0; i<7; i++){
		struct Room currentRoom = chosenRooms[i];
		if(currentRoom.numConnections < 3 || currentRoom.numConnections > 6){
			return 0;
		}	
	}
	return 1;
}

/* Adds a random, valid outbound connection from a room to another room.*/
void AddRandomConnection(){
	struct Room* A;
	struct Room* B;	
	while(1){
		A = GetRandomRoom();
		if(CanAddConnectionFrom(*A) == 1){
			break;
		}
	}	

	do{
		B = GetRandomRoom();
	}
	while(CanAddConnectionFrom(*B) == 0 || IsSameRoom(*A,*B) == 1 || ConnectionAlreadyExists(*A,*B) == 1);
	ConnectRoom(A,B);
	ConnectRoom(B,A);
	
}

/* Returns a random Room, does NOT validate if connection can be added.*/
struct Room* GetRandomRoom(){
	int randIndex = rand()%7;
	return &chosenRooms[randIndex];
}

/* Returns 1 if a connection can be added from Room x ( < 6 outbound connections), false otherwise.*/
int CanAddConnectionFrom(struct Room x){
	if(x.numConnections < 6){
		return 1;
	}
	return 0;
}

/* Returns 1 if a connection from Room x to Room y already exists, false otherwise.*/
int ConnectionAlreadyExists(struct Room x, struct Room y){
	int i;
	for(i = 0; i < x.numConnections; i++){
		int sameName = strcmp(x.outBoundConnections[i],y.roomName);
		if(sameName == 0){
			return 1;
		}
	}
	return 0;
}

/* Connects Rooms x and y together, does not check if this connection is valid.*/
void ConnectRoom(struct Room* x, struct Room* y){
	strcpy(x->outBoundConnections[x->numConnections],y->roomName);
	x->numConnections++;
}

/* Returns 1 if Rooms x and y are the same Room, false otherwise.*/
int IsSameRoom(struct Room x, struct Room y){
	int sameName = strcmp(x.roomName, y.roomName);
	if(sameName == 0){
		return 1;
	}
	return 0;
}

int main(){
	srand(time(0));
	/* Create dir to hold rooms*/
	char roomsDirName[] = "leejuic.rooms.";
	int processID = getpid();
	sprintf(roomsDirName, "%s%d", roomsDirName, processID);
	mkdir(roomsDirName, 0755);	
		
	/* Randomly create 7 rooms and randomly assign a name and type to them.*/
	int i;
	int j;
	int chosenNames[10] = {0,0,0,0,0,0,0,0,0,0}; /* Matches exact positions for selection of roomNames. 0 if unchosen and 1 if chosen*/
	int startUsed = 0;
	int endUsed = 0;
	
	for(i = 0; i < 7 ; i++){
		struct Room newRoom;
		int foundName = 0;
		while(foundName == 0){ /*While a unique name has not been found, keep looking for a name in chosenNames array.*/
			int chosenIndex = rand()%10;
			if(chosenNames[chosenIndex] == 0){
				strcpy(newRoom.roomName, roomNames[chosenIndex]);
				chosenNames[chosenIndex] = 1;
				foundName = 1;
			}
		}
		/*Assigns the room type to the room through integers: 0(start_room), 1(end_room), 2(mid_room).*/
		if(startUsed == 0){
			newRoom.roomType = 0;
			startUsed = 1;
		}else if(endUsed == 0){
			newRoom.roomType = 1;
			endUsed = 1;
		}else{
			newRoom.roomType = 2;
		}
		/*Initialize numConnections for each room and copy new room to an array index*/
		newRoom.numConnections = 0;
		chosenRooms[i] = newRoom;
	}
	
	/*Adding the randomized outbound connections to the chosenRooms*/
	while(IsGraphFull() == 0){
		AddRandomConnection();
	}

	/*Finish building rooms by writing each room to file with the proper format.*/
	for(j = 0; j < 7; j++){
		FILE *fp;
		char roomsFileName[20];
		/*Find file path to create new room file in*/
		strcpy(roomsFileName, roomsDirName);
		sprintf(roomsFileName, "%s%s%s", roomsFileName, "/", chosenRooms[j].roomName);
		fp=fopen(roomsFileName, "w+");
		fprintf(fp, "ROOM NAME: %s\n", chosenRooms[j].roomName);

		/*Format the room's information when writing to the file*/
		int s = 1;
		for(i = 0; i < chosenRooms[j].numConnections; i++){
			fprintf(fp, "CONNECTION %d: %s\n", s, chosenRooms[j].outBoundConnections[i]);	
			s++;
		}
		int type = chosenRooms[j].roomType;
		if(type == 0){
			fprintf(fp, "ROOM TYPE: START_ROOM\n\n");
		}else if(type == 1){
			fprintf(fp, "ROOM TYPE: END_ROOM\n\n");
		}else{
			fprintf(fp, "ROOM TYPE: MID_ROOM\n\n");
		}
		fclose(fp);
	}
	return 0;
}
