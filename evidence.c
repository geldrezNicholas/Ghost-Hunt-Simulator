#include "defs.h"
#include "helpers.h"

/* 
   Function: evidence_set
   Purpose:  Sets a specific evidence bit to 1 in the evidence byte.
   Params:   
    Input/Output: EvidenceByte* ev - pointer to the evidence byte to modify
    Input: enum EvidenceType type - the evidence type bit to set
   Return: void
*/
void evidence_set(EvidenceByte* ev, enum EvidenceType type){
  *ev |= type;
}

/* 
   Function: evidence_clear
   Purpose:  Clears a specific evidence bit (sets to 0) in the evidence byte.
   Params:   
    Input/Output: EvidenceByte* ev - pointer to the evidence byte to modify
    Input: enum EvidenceType type - the evidence type bit to clear
   Return: void
*/
void evidence_clear(EvidenceByte* ev, enum EvidenceType type){
  //AND with compliment to make bit 0
  *ev &= ~type;
}

/* 
   Function: evidence_has
   Purpose:  Checks if a specific evidence bit is set in the evidence byte.
   Params:   
    Input: EvidenceByte ev - the evidence byte to check
    Input: enum EvidenceType type - the evidence type bit to check for
   Return: bool - true if the bit is set, false otherwise
*/
bool evidence_has(EvidenceByte ev, enum EvidenceType type){
  return (ev & type) != 0;
}

/* 
   Function: evidence_count_bits
   Purpose:  Counts how many evidence bits are set in the evidence byte.
   Params:   
    Input: EvidenceByte ev - the evidence byte to count
   Return: int - the number of bits set to 1
*/
int evidence_count_bits(EvidenceByte ev){
  int count = 0;
  for(int i=0; i < 8; i++){
    if(ev & (1<<i)){
      count++;
    }
  }
  return count;
}

/* 
   Function: evidence_has_three_unique
   Purpose:  Checks if the evidence byte has at least 3 unique bits set.
   This is needed to identify a ghost type.
   Params:   
    Input: EvidenceByte mask - the evidence byte to check
   Return: bool - true if at least 3 bits are set, false otherwise
*/
bool evidence_has_three_unique(EvidenceByte mask){
  return evidence_count_bits(mask) >= 3;
}
