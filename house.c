#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "helpers.h"

/* 
   Function: house_init
   Purpose:  Initializes a house structure with default values and allocates
   memory for the hunter array.
   Params:   
   Input/Output: struct House* house - pointer to the house to initialize
   Return: void
*/
void house_init(struct House* house){
  house->room_count = 0;
  house->starting_room = NULL;
    
  //initialize hunter array (start with max of 4)
  house->hunter_capacity = 4;
  house->hunter_count = 0;
  house->hunters = malloc(house->hunter_capacity * sizeof(struct Hunter));
    
  //initialize casefile
  house->caseFile.collected = 0;
  house->caseFile.solved = false;
  sem_init(&house->caseFile.mutex, 0, 1);
    
  //ghost will be initialized separately
  house->ghost.has_exited = false;
}

/* 
   Function: house_add_hunter
   Purpose:  Adds a hunter to the house's dynamic array, growing it if necessary.
   Params:   
   Input/Output: struct House* house - pointer to the house
   Input: const char* name - the hunter's name
   Input: int id - the hunter's ID
   Return: void
*/
void house_add_hunter(struct House* house, const char* name, int id){
  //check if we need to grow the array
  if(house->hunter_count >= house->hunter_capacity){
    //double the capacity
    house->hunter_capacity *= 2;
        
    //reallocate the array
    struct Hunter* new_hunters = realloc(house->hunters, house->hunter_capacity * sizeof(struct Hunter));

    //if the reallocate doesnt work
    if(new_hunters == NULL){
      return;
    }
        
    house->hunters = new_hunters;
  }
    
  //initialize the new hunter
  hunter_init(&house->hunters[house->hunter_count], name, id, house->starting_room, &house->caseFile);
    
  house->hunter_count++;
}

/* 
   Function: house_cleanup
   Purpose:  Cleans up all  allocated resources in the house,
   including rooms, hunters, and semaphores.
   Params:   
   Input/Output: struct House* house - pointer to the house to clean up
   Return: void
*/
void house_cleanup(struct House* house){
  //clean up all rooms
  for(int i = 0; i < house->room_count; i++){
    room_cleanup(&house->rooms[i]);
  }
    
  //clean up all hunters
  for(int i = 0; i < house->hunter_count; i++){
    hunter_cleanup(&house->hunters[i]);
  }
    
  //free hunter array
  free(house->hunters);
    
  //destroy casefile semaphore
  sem_destroy(&house->caseFile.mutex);
}
