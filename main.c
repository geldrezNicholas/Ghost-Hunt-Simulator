#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "helpers.h"

int main() {

    /*
    1. Initialize a House structure.
    2. Populate the House with rooms using the provided helper function.
    3. Initialize all of the ghost data and hunters.
    4. Create threads for the ghost and each hunter.
    5. Wait for all threads to complete.
    6. Print final results to the console:
         - Type of ghost encountered.
         - The reason that each hunter exited
         - The evidence collected by each hunter and which ghost is represented by that evidence.
    7. Clean up all dynamically allocated resources and call sem_destroy() on all semaphores.
    */

  printf("=== Ghost Hunt Simulator ===\n\n");

  //Initialize House structure
  struct House house;
  house_init(&house);

  //Populate the house with rooms
  house_populate_rooms(&house);
  printf("House initialized with %d rooms\n", house.room_count);

  //Initialize the ghost and hunters
  ghost_init(&house.ghost, &house);
  printf("Ghost Initialized: %s in %s\n\n",
	 ghost_to_string(house.ghost.type),
	 house.ghost.current_room->name);

  printf("Enter hunter information (type 'done' for the name when finished):\n");

  char name[MAX_HUNTER_NAME];
  int id;

  while(true){
    //Get hunter name
    printf("Hunter name: ");
    if (scanf("%s", name) != 1) {
      break;
    }
        
    //Check if user wants to stop
    if (strcmp(name, "done") == 0) {
      break;
    }
        
    //Get hunter ID
    printf("Hunter ID: ");
    if (scanf("%d", &id) != 1) {
      break;
    }
        
    //Add hunter to house
    house_add_hunter(&house, name, id);
    printf("Added hunter: %s (ID: %d)\n\n", name, id);
  }  

  printf("\n=== Starting Simulation ===\n");
  printf("Hunters: %d\n", house.hunter_count);
  printf("Ghost: %s\n\n", ghost_to_string(house.ghost.type));

  //Create thread array for hunters
  pthread_t ghost_thread_id;
  pthread_t* hunter_threads = malloc(house.hunter_count * sizeof(pthread_t));

  //Create ghost thread
  pthread_create(&ghost_thread_id, NULL, ghost_thread, &house.ghost);

  //Create one thread for each hunter
  for (int i = 0; i < house.hunter_count; i++) {
    pthread_create(&hunter_threads[i], NULL, hunter_thread, &house.hunters[i]);
  }

  //wait for all threads to complete
  //wait for ghost thread
  pthread_join(ghost_thread_id, NULL);

  //wait for all hunter threads
  for (int i = 0; i < house.hunter_count; i++) {
    pthread_join(hunter_threads[i], NULL);
  }

  //Free the thread array
  free(hunter_threads);
  
  //Display results
  printf("\n=== Simulation Complete ===\n\n");
    
  // Display why each hunter left
  printf("Hunter Results:\n");
  for (int i = 0; i < house.hunter_count; i++) {
    struct Hunter* hunter = &house.hunters[i];
    printf("  %s (ID: %d): %s\n", 
	   hunter->name, 
	   hunter->id, 
	   exit_reason_to_string(hunter->exit_reason));
  }
    
  //Display evidence collected
  printf("\nEvidence Collected: ");
  const enum EvidenceType* all_evidence = NULL;
  int count = get_all_evidence_types(&all_evidence);
  bool found_any = false;
    
  for (int i = 0; i < count; i++) {
    if (evidence_has(house.caseFile.collected, all_evidence[i])) {
      if (found_any) printf(", ");
      printf("%s", evidence_to_string(all_evidence[i]));
      found_any = true;
    }
  }
  if (!found_any) printf("None");
  printf("\n");
    
  //Display ghost type
  printf("\nActual Ghost: %s\n", ghost_to_string(house.ghost.type));
    
  //What does the evidence suggest?
  printf("Evidence Suggests: ");
  if (evidence_is_valid_ghost(house.caseFile.collected)) {
    //Find matching ghost
    const enum GhostType* ghost_types = NULL;
    int ghost_count = get_all_ghost_types(&ghost_types);
        
    for (int i = 0; i < ghost_count; i++) {
      if (house.caseFile.collected == (EvidenceByte)ghost_types[i]) {
	printf("%s\n", ghost_to_string(ghost_types[i]));
	break;
      }
    }
  } else {
    printf("Inconclusive (not enough or invalid evidence)\n");
  }
    
  //Cleanup
  printf("\nCleaning up...\n");
  house_cleanup(&house);
    
  printf("Game ended successfully!\n");  
 
  return 0;
}
