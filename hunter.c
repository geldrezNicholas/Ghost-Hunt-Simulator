#include <string.h>
#include <stdlib.h>
#include "defs.h"
#include "helpers.h"

/* 
   Function: hunter_init
   Purpose: Initializes a hunter with a name, ID, device, and starting room.
   Params:   
   Input/Output: struct Hunter* hunter - pointer to the hunter to initialize
    Input: const char* name - the hunter's name
    Input: int id - the hunter's ID
    Input: struct Room* starting_room - pointer to the starting room (Van)
    Input: struct CaseFile* casefile - pointer to the shared case file
   Return: void
*/
void hunter_init(struct Hunter* hunter, const char* name, int id, 
                 struct Room* starting_room, struct CaseFile* casefile){
  //copy hunter with null terminator
  strncpy(hunter->name, name, MAX_HUNTER_NAME - 1);
  hunter->name[MAX_HUNTER_NAME - 1] = '\0';
    
  hunter->id = id;
  hunter->current_room = starting_room;
  hunter->casefile = casefile;
    
  //assigning a random device
  const enum EvidenceType* evidence_types = NULL;
  int count = get_all_evidence_types(&evidence_types);
  int random_index = rand_int_threadsafe(0, count);
  hunter->device = evidence_types[random_index];
    
  //initializing the hunter path
  roomstack_init(&hunter->path);
    
  //initialize stats
  hunter->fear = 0;
  hunter->boredom = 0;
  hunter->should_exit = false;
  hunter->return_to_van = false;
  hunter->exit_reason = LR_BORED;
    
  //Log initialization
  log_hunter_init(id, starting_room->name, name, hunter->device);
}

/* 
   Function: hunter_move
   Purpose:  Moves a hunter from their current room to a target room.
   Handles room capacity checks and proper add/remove operations.
   Uses semaphore locking to prevent race conditions
   Params:   
   Input/Output: struct Hunter* hunter - the hunter to move
    Input: struct Room* target_room - the room to move to
   Return: bool - true if move was successful, false if room was full
*/
bool hunter_move(struct Hunter* hunter, struct Room* target_room){

  //where is the hunter coming from
  struct Room* from_room = hunter->current_room;
    
  //lock rooms in consistent order by memory address
  //stops deadlock
  struct Room* first = (from_room < target_room) ? from_room : target_room;
  struct Room* second = (from_room < target_room) ? target_room : from_room;
    
  //lock both rooms
  sem_wait(&first->mutex);
  sem_wait(&second->mutex);
    
  //check room count
  if(target_room->hunter_count >= MAX_ROOM_OCCUPANCY){
    //room became full
    sem_post(&second->mutex);
    sem_post(&first->mutex);
    return false;
  }
    
  //do the move
  room_remove_hunter(from_room, hunter);
    
  //update hunter current room
  hunter->current_room = target_room;
    
  //add the hunter to the new room
  room_add_hunter(target_room, hunter);
    
  //unlock both rooms 
  sem_post(&second->mutex);
  sem_post(&first->mutex);
    
  //log the move
  log_move(hunter->id, hunter->boredom, hunter->fear, 
	   from_room->name, target_room->name, hunter->device);
    
  return true;
}

/* 
   Function: hunter_update_stats
   Purpose: Updates hunter's fear and boredom based on ghost presence.
   Params:   
    Input/Output: struct Hunter* hunter - the hunter to update
   Return: void
*/
void hunter_update_stats(struct Hunter* hunter){
  //lock room to check if ghost is present
  sem_wait(&hunter->current_room->mutex);
  bool ghost_present = (hunter->current_room->ghost != NULL);
  sem_post(&hunter->current_room->mutex);
  
  //check if ghost is in the same room
  if(ghost_present){
    //ghost is here! Reset boredom, increase fear
    hunter->boredom = 0;
    hunter->fear++;
  }else{
    //no ghost, increase boredom
    hunter->boredom++;
  }
}

/* 
   Function: hunter_check_van
   Purpose: Checks if hunter is in the van/exit room and handles victory checking and device swapping.
   Params:   
   Input/Output: struct Hunter* hunter - the hunter to check
   Return: void
*/
void hunter_check_van(struct Hunter* hunter){
  //check if in exit room
  if(!hunter->current_room->is_exit){
    return;  //Not in van, nothing to do
  }
    
  //clear breadcrumb stack
  roomstack_clear(&hunter->path);
    
  //clear the return flag since we're here now
  if(hunter->return_to_van){
    hunter->return_to_van = false;
    log_return_to_van(hunter->id, hunter->boredom, hunter->fear,
		      hunter->current_room->name, hunter->device, false);
  }
    
  //check for victory
  //lock the casefile to check safely
  sem_wait(&hunter->casefile->mutex);
    
  //check if we have enough evidence to identify the ghost
  if(evidence_has_three_unique(hunter->casefile->collected) &&
      evidence_is_valid_ghost(hunter->casefile->collected)){
        
    hunter->casefile->solved = true;
        
    //unlock before exiting
    sem_post(&hunter->casefile->mutex);
        
    //remove from room and exit, lcok the room first
    sem_wait(&hunter->current_room->mutex);
    room_remove_hunter(hunter->current_room, hunter);
    sem_post(&hunter->current_room->mutex);
    
    hunter->should_exit = true;
    hunter->exit_reason = LR_EVIDENCE;
        
    log_exit(hunter->id, hunter->boredom, hunter->fear, hunter->current_room->name, hunter->device, LR_EVIDENCE);
    return;
  }
    
  sem_post(&hunter->casefile->mutex);
    
  //swap to a new device
  enum EvidenceType old_device = hunter->device;
    
  //pick a random new device
  const enum EvidenceType* evidence_types = NULL;
  int count = get_all_evidence_types(&evidence_types);
  int random_index = rand_int_threadsafe(0, count);
  hunter->device = evidence_types[random_index];
    
  //log the swap
  log_swap(hunter->id, hunter->boredom, hunter->fear, old_device, hunter->device);
}

/* 
   Function: hunter_check_exit_conditions
   Purpose: Checks if hunter should exit due to fear or boredom.
   Params:   
   Input/Output: struct Hunter* hunter - the hunter to check
   Return: void
*/
void hunter_check_exit_conditions(struct Hunter* hunter){
  //check boredom
  if(hunter->boredom > ENTITY_BOREDOM_MAX){
    sem_wait(&hunter->current_room->mutex);
    room_remove_hunter(hunter->current_room, hunter);
    sem_post(&hunter->current_room->mutex);
    
    hunter->should_exit = true;
    hunter->exit_reason = LR_BORED;
        
    log_exit(hunter->id, hunter->boredom, hunter->fear,
	     hunter->current_room->name, hunter->device, LR_BORED);
    return;
  }
    
  //check fear
  if(hunter->fear > HUNTER_FEAR_MAX){
    sem_wait(&hunter->current_room->mutex);
    room_remove_hunter(hunter->current_room, hunter);
    sem_post(&hunter->current_room->mutex);
    
    hunter->should_exit = true;
    hunter->exit_reason = LR_AFRAID;
        
    log_exit(hunter->id, hunter->boredom, hunter->fear, hunter->current_room->name, hunter->device, LR_AFRAID);
    return;
  }
}

/* 
   Function: hunter_gather_evidence
   Purpose: Attempts to gather evidence from the current room. 
   Params:   
   Input/Output: struct Hunter* hunter - the hunter gathering evidence
   Return: void
*/
void hunter_gather_evidence(struct Hunter* hunter){
  //skip if already in van or returning to van
  if(hunter->current_room->is_exit){
    return;
  }

  //lockroom and check if room has evidence matching device
  sem_wait(&hunter->current_room->mutex);
  bool has_matching_evidence = evidence_has(hunter->current_room->evidence, hunter->device);
    
  //check if room has evidence matching our device
  if(has_matching_evidence){
    //found matching evidence!
        
    //remove from room
    evidence_clear(&hunter->current_room->evidence, hunter->device);

    //unlock room before locking
    sem_post(&hunter->current_room->mutex);
        
    //add to shared casefile
    sem_wait(&hunter->casefile->mutex);
    evidence_set(&hunter->casefile->collected, hunter->device);
    sem_post(&hunter->casefile->mutex);
        
    //log the evidence collection
    log_evidence(hunter->id, hunter->boredom, hunter->fear, hunter->current_room->name, hunter->device);
        
    //set flag to return to van
    if(!hunter->current_room->is_exit){
      hunter->return_to_van = true;
      log_return_to_van(hunter->id, hunter->boredom, hunter->fear, hunter->current_room->name, hunter->device, true);
    }
  }else{
    //no matching evidence
    sem_post(&hunter->current_room->mutex);
    
    //small chance to return anyway
    int random = rand_int_threadsafe(0, 100);
    if(random < 10){  //10% chance
      hunter->return_to_van = true;
      log_return_to_van(hunter->id, hunter->boredom, hunter->fear, hunter->current_room->name, hunter->device, true);
    }
  }
}

/* 
   Function: hunter_choose_move
   Purpose: Chooses which room to move to next (exploring or returning).
   Params:   
   Input/Output: struct Hunter* hunter - the hunter choosing a move
   Return: void
*/
void hunter_choose_move(struct Hunter* hunter){
  struct Room* target_room = NULL;
  struct Room* old_room = hunter->current_room;
    
  //check if returning to van
  if(hunter->return_to_van){
    //pop from breadcrumb stack
    target_room = roomstack_pop(&hunter->path);
        
    if(target_room == NULL){
      //stack was empty, we must be at van already
      return;
    }
  } else{
    //exploring: pick random connected room
    if(hunter->current_room->connection_count == 0){
      return; //no connections
    }
        
    int random_index = rand_int_threadsafe(0, hunter->current_room->connection_count);
    target_room = hunter->current_room->connections[random_index];
  }
    
  //attempt the move
  bool success = hunter_move(hunter, target_room);
    
  //if exploring and move succeeded, push old room to stack
  if(success && !hunter->return_to_van){
    roomstack_push(&hunter->path, old_room);
  }
}

/* 
   Function: hunter_cleanup
   Purpose: Cleans up all resources allocated for a hunter.
   Params:   
   Input/Output: struct Hunter* hunter - pointer to the hunter to clean up
   Return: void
*/
void hunter_cleanup(struct Hunter* hunter){
  // Clean up the stack
  roomstack_cleanup(&hunter->path);
}

/* 
   Function: hunter_thread
   Purpose:  Thread function for a hunter. Runs the hunter's behavior loop.
   Params:   
   Input: void* data - pointer to the Hunter structure
   Return: void* - NULL when thread completes
*/
void* hunter_thread(void* data){
  //cast the generic pointer back to a Hunter pointer
  struct Hunter* hunter = (struct Hunter*)data;

  //Keep running until hunter decides to exit
  while(!hunter->should_exit){
    //update fear or boredom based on the ghost and check if hunter is in the va
    hunter_update_stats(hunter);
    hunter_check_van(hunter);

    //Only continue if hunter exited
    if(!hunter->should_exit){
      hunter_check_exit_conditions(hunter);
    }

    //only continue if hunter has NOT exited, hes tuff
    if(!hunter->should_exit){
      hunter_gather_evidence(hunter);
      hunter_choose_move(hunter);
    }
  }
    
  return NULL;
}


