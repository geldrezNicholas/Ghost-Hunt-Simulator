#include <stdlib.h>
#include "defs.h"
#include "helpers.h"
/* 
   Function: ghost_init
   Purpose: Initializes the ghost with a random type and starting room.
   Params:   
    Input/Output: struct Ghost* ghost - pointer to the ghost to initialize
    Input: struct House* house - pointer to the house (for random room selection)
   Return: void
*/
void ghost_init(struct Ghost* ghost, struct House* house){
  //Set ghost ID
  ghost->id = DEFAULT_GHOST_ID;
    
  //Assign random ghost type
  const enum GhostType* ghost_types = NULL;
  int count = get_all_ghost_types(&ghost_types);
  int random_index = rand_int_threadsafe(0, count);
  ghost->type = ghost_types[random_index];
    
  //Start in a random room
  //Skip the Van (index 0), start from index 1
  random_index = rand_int_threadsafe(1, house->room_count);
  ghost->current_room = &house->rooms[random_index];
  ghost->current_room->ghost = ghost;  //Set the room's ghost pointer
    
  //Initialize stats
  ghost->boredom = 0;
  ghost->has_exited = false;
    
  //Log initialization
  log_ghost_init(ghost->id, ghost->current_room->name, ghost->type);
}

/* 
   Function: ghost_update_stats
   Purpose: Updates ghost boredom based on hunter presence.
   Params:   
   Input/Output: struct Ghost* ghost - the ghost to update
   Return: void
*/
void ghost_update_stats(struct Ghost* ghost){
  // Check if there are hunters in the room
  if(room_has_hunters(ghost->current_room)){
    // Hunters present - reset boredom
    ghost->boredom = 0;
  }else{
    // No hunters - increase boredom
    ghost->boredom++;
  }
}

/* 
   Function: ghost_check_exit
   Purpose: Checks if ghost should exit due to boredom.
   Params:   
    Input/Output: struct Ghost* ghost - the ghost to check
   Return: bool - true if ghost exited, false otherwise
*/
bool ghost_check_exit(struct Ghost* ghost){
  //Check if boredom exceeds maximum
  if(ghost->boredom > ENTITY_BOREDOM_MAX){
    ghost->has_exited = true;
        
    //Log the exit
    log_ghost_exit(ghost->id, ghost->boredom, ghost->current_room->name);
        
    // Remove ghost from room
    sem_wait(&ghost->current_room->mutex);
    ghost->current_room->ghost = NULL;
    sem_post(&ghost->current_room->mutex);
    
    return true;
  }
    
  return false;
}

/* 
   Function: ghost_leave_evidence
   Purpose: Drops a piece of evidence in the current room.
   Params:   
    Input/Output: struct Ghost* ghost - the ghost leaving evidence
   Return: void
*/
void ghost_leave_evidence(struct Ghost* ghost){
    
  //pick ONE of the three evidence types the ghost can leave
  //Build an array of the ghost's evidence types
  const enum EvidenceType* all_evidence = NULL;
  int total_count = get_all_evidence_types(&all_evidence);
    
  enum EvidenceType ghost_evidence[3];
  int ghost_ev_count = 0;
    
  //Find which evidence bits are set in the ghost's type
  for(int i = 0; i < total_count; i++){
    if(ghost->type & all_evidence[i]){
      ghost_evidence[ghost_ev_count] = all_evidence[i];
      ghost_ev_count++;
    }
  }
    
  //Pick one at random
  if(ghost_ev_count > 0){
    int random_index = rand_int_threadsafe(0, ghost_ev_count);
    enum EvidenceType evidence_to_leave = ghost_evidence[random_index];
        
    //lock before adding evidence
    sem_wait(&ghost->current_room->mutex);
    evidence_set(&ghost->current_room->evidence, evidence_to_leave);
    sem_post(&ghost->current_room->mutex);
        
    //Log it
    log_ghost_evidence(ghost->id, ghost->boredom, ghost->current_room->name, evidence_to_leave);
  }
}

/* 
   Function: ghost_move
   Purpose: Moves ghost to a random connected room.
   Params:   
    Input/Output: struct Ghost* ghost - the ghost to move
   Return: void
*/
void ghost_move(struct Ghost* ghost){
  //Cant move if hunters are in the room
  if(room_has_hunters(ghost->current_room)){
    return;
  }
    
  //Cant move if no connections
  if(ghost->current_room->connection_count == 0){
    return;
  }
    
  struct Room* from_room = ghost->current_room;
    
  //Pick a random connected room
  int random_index = rand_int_threadsafe(0, from_room->connection_count);
  struct Room* target_room = from_room->connections[random_index];

  //stops deadlocks - lock rooms in order by memory access
  struct Room* first = (from_room < target_room) ? from_room : target_room;
  struct Room* second = (from_room < target_room) ? target_room : from_room;

  //lock both rooms
  sem_wait(&first->mutex);
  sem_wait(&second->mutex);
    
  //Remove ghost from current room 
  from_room->ghost = NULL;
    
  //Move ghost to new room
  ghost->current_room = target_room;
  target_room->ghost = ghost;

  //unlock both rooms
  sem_post(&second->mutex);
  sem_post(&first->mutex);
    
  //Log the move
  log_ghost_move(ghost->id, ghost->boredom, from_room->name, target_room->name);
}

/* 
   Function: ghost_take_action
   Purpose: Ghost randomly chooses to idle, haunt, or move.
   Params:   
   Input/Output: struct Ghost* ghost - the ghost taking action
   Return: void
*/
void ghost_take_action(struct Ghost* ghost){
  //Randomly choose an action
  // 0 = does nothing, 1 = leave evidence, 2 = just move
  int action = rand_int_threadsafe(0, 3);
    
  if(action == 0){
    log_ghost_idle(ghost->id, ghost->boredom, ghost->current_room->name);
  }else if(action == 1){
    ghost_leave_evidence(ghost);
  }else{
    ghost_move(ghost);
  }
}

/* 
   Function: ghost_thread
   Purpose:  Thread function for the ghost. Runs the ghost's behavior loop
   until the ghost exits the simulation.
   Params:   
   Input: void* data - pointer to the Ghost structure (cast from void*)
   Return: void* - NULL when thread completes
*/
void* ghost_thread(void* data){
  //cast the pointer back to a Ghost pointer
  struct Ghost* ghost = (struct Ghost*)data;
    
  //keep running until ghost exits
  while(!ghost->has_exited){
    //Update boredom based on hunter presence
    ghost_update_stats(ghost);
        
    //Check if ghost should exit due to boredom 
    //Only take action if ghost hasn't exited
    if(!ghost_check_exit(ghost)){
      //Randomly choose to stau still, leave evidence, or move
      ghost_take_action(ghost);
    }
  }
    
  //Thread is done so return NULL
  return NULL;
}
