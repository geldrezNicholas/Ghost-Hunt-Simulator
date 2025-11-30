#include <string.h>
#include "defs.h"
#include "helpers.h"

/* 
   Function: room_init
   Purpose:  Initializes a room structure with default values.
   Params:   
    Input/Output: struct Room* room - pointer to the room to initialize
    Input: const char* name - the name of the room
    Input: bool is_exit - true if this room is the exit/van
   Return: void
*/
void room_init(struct Room* room, const char* name, bool is_exit){
  //Copy the name with null terminator
  strncpy(room->name, name, MAX_ROOM_NAME -1);
  room->name[MAX_ROOM_NAME-1] = '\0';

  //Initializing all connections
  room->connection_count = 0;
  for(int i=0; i < MAX_CONNECTIONS; i++){
    room->connections[i] = NULL;
  }

  //Same with ghost and hunters
  room->ghost = NULL;
  room->hunter_count = 0;
  for(int i=0; i < MAX_ROOM_OCCUPANCY; i++){
    room->hunters[i] = NULL;
  }

  //Initialize the rest of the info
  room->is_exit = is_exit;
  room->evidence = 0;

  //Initialize sempahore
  sem_init(&room->mutex,0,1);
 
}

/* 
   Function: rooms_connect
   Purpose:  Creates a bidirectional connection between two rooms.
   Params:   
    Input/Output: struct Room* a - first room to connect
    Input/Output: struct Room* b - second room to connect
   Return: void
*/
void room_connect(struct Room* a, struct Room* b){
  //Add room b to room a's connections
  if(a->connection_count < MAX_CONNECTIONS){
    a->connections[a->connection_count] = b;
    a->connection_count++;
  }
  
  //Add room a to room b's connections
  if(b->connection_count < MAX_CONNECTIONS){
    b->connections[b->connection_count] = a;
    b->connection_count++;
  }
}

/* 
   Function: room_add_evidence
   Purpose:  Adds a specific type of evidence to a room.
   Params:   
    Input/Output: struct Room* room - the room to add evidence to
    Input: enum EvidenceType evidence - the type of evidence to add
   Return: void
*/
void room_add_evidence(struct Room* room, enum EvidenceType evidence){
  evidence_set(&room->evidence, evidence);
}

/* 
   Function: room_add_hunter
   Purpose:  Adds a hunter to a room if there is space available.
   Params:   
    Input/Output: struct Room* room - the room to add the hunter to
    Input: struct Hunter* hunter - pointer to the hunter to add
   Return: bool - true if hunter was added, false if room is full
*/
bool room_add_hunter(struct Room* room, struct Hunter* hunter){
  //if the room is full
  if(room->hunter_count >= MAX_ROOM_OCCUPANCY){
    return false;
  }

  //add the hunter
  room->hunters[room->hunter_count] = hunter;
  room->hunter_count++;
  return true;
}

/* 
   Function: room_remove_hunter
   Purpose:  Removes a specific hunter from a room.
   Params:   
    Input/Output: struct Room* room - the room to remove the hunter from
    Input: struct Hunter* hunter - pointer to the hunter to remove
   Return: void
*/
void room_remove_hunter(struct Room* room, struct Hunter* hunter){
  //find the hunter in the array
  for(int i = 0; i < room->hunter_count; i++){
    if(room->hunters[i] == hunter){
      //shift the hunters down
      for(int j = i; j < room->hunter_count-1; j++){
	room->hunters[j] = room->hunters[j+1];
      }
      room->hunters[room->hunter_count-1] = NULL;
      room->hunter_count--;
      return;
    }
  }
}

/* 
   Function: room_has_hunters
   Purpose:  Checks if there are any hunters currently in the room.
             Locks the room mutex before reading
   Params:   
    Input: struct Room* room - the room to check
   Return: bool - true if at least one hunter is in the room, false otherwise
*/
bool room_has_hunters(struct Room* room){
  sem_wait(&room->mutex);
  bool result = (room->hunter_count > 0);
  sem_post(&room->mutex);
  return result;
}

/* 
   Function: room_cleanup
   Purpose:  Cleans up resources allocated for a room (destroys semaphore).
   Params:   
    Input/Output: struct Room* room - the room to clean up
   Return: void
*/
void room_cleanup(struct Room* room){
  sem_destroy(&room->mutex);
}

