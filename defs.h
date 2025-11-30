#ifndef DEFS_H
#define DEFS_H

#include <stdbool.h>
#include <semaphore.h>
#include <pthread.h>

/*
  You are free to rename all of the types and functions defined here.

  The ghost ID must remain the same for the validator to work correctly.
*/

#define MAX_ROOM_NAME 64
#define MAX_HUNTER_NAME 64
#define MAX_ROOMS 24
#define MAX_ROOM_OCCUPANCY 8
#define MAX_CONNECTIONS 8
#define ENTITY_BOREDOM_MAX 15
#define HUNTER_FEAR_MAX 15
#define DEFAULT_GHOST_ID 68057

typedef unsigned char EvidenceByte; // Just giving a helpful name to unsigned char for evidence bitmasks

enum LogReason {
  LR_EVIDENCE = 0,
  LR_BORED = 1,
  LR_AFRAID = 2
};

enum EvidenceType {
  EV_EMF          = 1 << 0,
  EV_ORBS         = 1 << 1,
  EV_RADIO        = 1 << 2,
  EV_TEMPERATURE  = 1 << 3,
  EV_FINGERPRINTS = 1 << 4,
  EV_WRITING      = 1 << 5,
  EV_INFRARED     = 1 << 6,
};

enum GhostType {
  GH_POLTERGEIST  = EV_FINGERPRINTS | EV_TEMPERATURE | EV_WRITING,
  GH_THE_MIMIC    = EV_FINGERPRINTS | EV_TEMPERATURE | EV_RADIO,
  GH_HANTU        = EV_FINGERPRINTS | EV_TEMPERATURE | EV_ORBS,
  GH_JINN         = EV_FINGERPRINTS | EV_TEMPERATURE | EV_EMF,
  GH_PHANTOM      = EV_FINGERPRINTS | EV_INFRARED    | EV_RADIO,
  GH_BANSHEE      = EV_FINGERPRINTS | EV_INFRARED    | EV_ORBS,
  GH_GORYO        = EV_FINGERPRINTS | EV_INFRARED    | EV_EMF,
  GH_BULLIES      = EV_FINGERPRINTS | EV_WRITING     | EV_RADIO,
  GH_MYLING       = EV_FINGERPRINTS | EV_WRITING     | EV_EMF,
  GH_OBAKE        = EV_FINGERPRINTS | EV_ORBS        | EV_EMF,
  GH_YUREI        = EV_TEMPERATURE  | EV_INFRARED    | EV_ORBS,
  GH_ONI          = EV_TEMPERATURE  | EV_INFRARED    | EV_EMF,
  GH_MOROI        = EV_TEMPERATURE  | EV_WRITING     | EV_RADIO,
  GH_REVENANT     = EV_TEMPERATURE  | EV_WRITING     | EV_ORBS,
  GH_SHADE        = EV_TEMPERATURE  | EV_WRITING     | EV_EMF,
  GH_ONRYO        = EV_TEMPERATURE  | EV_RADIO       | EV_ORBS,
  GH_THE_TWINS    = EV_TEMPERATURE  | EV_RADIO       | EV_EMF,
  GH_DEOGEN       = EV_INFRARED     | EV_WRITING     | EV_RADIO,
  GH_THAYE        = EV_INFRARED     | EV_WRITING     | EV_ORBS,
  GH_YOKAI        = EV_INFRARED     | EV_RADIO       | EV_ORBS,
  GH_WRAITH       = EV_INFRARED     | EV_RADIO       | EV_EMF,
  GH_RAIJU        = EV_INFRARED     | EV_ORBS        | EV_EMF,
  GH_MARE         = EV_WRITING      | EV_RADIO       | EV_ORBS,
  GH_SPIRIT       = EV_WRITING      | EV_RADIO       | EV_EMF,
};

struct CaseFile {
  EvidenceByte collected; // Union of all of the evidence bits collected between all hunters
  bool         solved;    // True when >=3 unique bits set
  sem_t        mutex;     // Used for synchronizing both fields when multithreading
};

//Room node for singly-linked list
struct RoomNode {
  struct Room* room;
  struct RoomNode* next;
};

//Using a stack so when hunters enter a room we push it onto a stack
//And when they leave(return to the van) we pop them from the stack
struct RoomStack{
  struct RoomNode* head;
};

// Implement here based on the requirements, should all be allocated to the House structure
struct Room {
  //Room name and connections
  char name[MAX_ROOM_NAME];
  struct Room* connections[MAX_CONNECTIONS];
  int connection_count;

  //Ghost, point to it when in the room
  struct Ghost* ghost;

  //Hunters
  struct Hunter* hunters[MAX_ROOM_OCCUPANCY];
  int hunter_count;

  //is this the van/exit
  bool is_exit;

  //Evidence in this room, bitmask
  EvidenceByte evidence;

  //thread synchronization
  sem_t mutex;
};

//Hunter struct
struct Hunter {
  char name[MAX_HUNTER_NAME];
  int id;

  //Where is this hunter and evidence
  struct Room* current_room;
  struct CaseFile* casefile;

  //What device do they have
  enum EvidenceType device;

  //Trail to what they've went to
  struct RoomStack path;

  //Scary and Bored
  int fear;
  int boredom;

  //Should the hunter exit, are they? and if so why
  bool should_exit;
  bool return_to_van;
  enum LogReason exit_reason;
};

// Implement here based on the requirements, should be allocated to the House structure
struct Ghost {
  int id;
  enum GhostType type; //What ghost, and what evidence does it leave
  struct Room* current_room; //Where is this ghost
  int boredom;
  bool has_exited; //has the ghost left
};

// Can be either stack or heap allocated
struct House {
  struct Room rooms[MAX_ROOMS];
  int room_count;
  
  struct Room* starting_room; // Needed by house_populate_rooms, but can be adjusted to suit your needs.

  //Array of hunters
  struct Hunter* hunters;
  int hunter_count;
  int hunter_capacity;

  //Shared evidence
  struct CaseFile caseFile;

  //Which ghost is at this house
  struct Ghost ghost;
  
};

/* The provided `house_populate_rooms()` function requires the following functions.
   You are free to rename them and change their parameters and modify house_populate_rooms()
   as needed as long as the house has the correct rooms and connections after calling it.
*/

void room_init(struct Room* room, const char* name, bool is_exit);
void rooms_connect(struct Room* a, struct Room* b); // Bidirectional connection

//Evidence Functions
void evidence_set(EvidenceByte* ev, enum EvidenceType type);
void evidence_clear(EvidenceByte* ev, enum EvidenceType type);
bool evidence_has(EvidenceByte ev, enum EvidenceType type);
int evidence_count_bits(EvidenceByte ev);
bool evidence_has_three_unique(EvidenceByte mask);

//Room Functions
void room_init(struct Room* room, const char* name, bool is_exit);
void room_connect(struct Room* a, struct Room* b);
void room_add_evidence(struct Room* room, enum EvidenceType evidence);
bool room_add_hunter(struct Room* room, struct Hunter* hunter);
void room_remove_hunter(struct Room* room, struct Hunter* hunter);
bool room_has_hunters(struct Room* room);
void room_cleanup(struct Room* room);

//RoomStack Functions
void roomstack_init(struct RoomStack* stack);
void roomstack_push(struct RoomStack* stack, struct Room* room);
struct Room* roomstack_pop(struct RoomStack* stack);
bool roomstack_is_empty(struct RoomStack* stack);
void roomstack_clear(struct RoomStack* stack);
void roomstack_cleanup(struct RoomStack* stack);

//Hunter Functions
void hunter_init(struct Hunter* hunter, const char* name, int id, struct Room* starting_room, struct CaseFile* casefile);
bool hunter_move(struct Hunter* hunter, struct Room* target_room);
void hunter_update_stats(struct Hunter* hunter);
void hunter_check_van(struct Hunter* hunter);
void hunter_check_exit_conditions(struct Hunter* hunter);
void hunter_gather_evidence(struct Hunter* hunter);
void hunter_choose_move(struct Hunter* hunter);
void hunter_cleanup(struct Hunter* hunter);
void* hunter_thread(void* data);

//Ghost Functions
void ghost_init(struct Ghost* ghost, struct House* house);
void ghost_update_stats(struct Ghost* ghost);
bool ghost_check_exit(struct Ghost* ghost);
void ghost_leave_evidence(struct Ghost* ghost);
void ghost_move(struct Ghost* ghost);
void ghost_take_action(struct Ghost* ghost);
void* ghost_thread(void* data);

//House Functions
void house_init(struct House* house);
void house_add_hunter(struct House* house, const char* name, int id);
void house_cleanup(struct House* house);


#endif // DEFS_H
