#include <stdlib.h>
#include "defs.h"

/* 
   Function: roomstack_init
   Purpose:  Initializes an empty room stack
   Params:   
    Input/Output: struct RoomStack* stack - pointer to the stack to initialize
   Return: void
*/
void roomstack_init(struct RoomStack* stack){
  stack->head=NULL;
}

/* 
   Function: roomstack_push
   Purpose:  Pushes a room onto the top of the stack.
   Params:   
    Input/Output: struct RoomStack* stack - the stack to push onto
    Input: struct Room* room - pointer to the room to push
   Return: void
*/
void roomstack_push(struct RoomStack* stack, struct Room* room){
  struct RoomNode* node = malloc(sizeof(struct RoomNode));

  //if the allocation does not work
  if(node == NULL){
    return;
  }

  //setup the node
  node->room = room;
  node->next = stack->head;

  //Make this node the head
  stack->head = node; 
}

/* 
   Function: roomstack_pop
   Purpose:  Pops a room from the top of the stack and returns it.
   Params:   
    Input/Output: struct RoomStack* stack - the stack to pop from
   Return: struct Room* - pointer to the popped room, or NULL if stack is empty
*/
struct Room* roomstack_pop(struct RoomStack *stack){
  //if the stack is empty
  if(stack->head == NULL){
    return NULL;
  }

  //Save the head node and room
  struct RoomNode* node = stack->head;
  struct Room* room = node->room;

  //Move the head
  stack->head = node->next;

  free(node);
  return room;
  
}

/* 
   Function: roomstack_is_empty
   Purpose:  Checks if the stack is empty.
   Params:   
    Input: struct RoomStack* stack - the stack to check
   Return: bool - true if stack is empty, false otherwise
*/
bool roomstack_is_empty(struct RoomStack* stack){
  return stack->head == NULL;
}

/* 
   Function: roomstack_clear
   Purpose:  Removes all nodes from the stack and frees their memory.
   Params:   
    Input/Output: struct RoomStack* stack - the stack to clear
   Return: void
*/
void roomstack_clear(struct RoomStack* stack){
  while(!roomstack_is_empty(stack)){
    roomstack_pop(stack);
  }
}


/* 
   Function: roomstack_cleanup
   Purpose:  Cleans up all resources used by the stack.
   Params:   
    Input/Output: struct RoomStack* stack - the stack to clean up
   Return: void
*/
void roomstack_cleanup(struct RoomStack* stack){
  roomstack_clear(stack);
}
