/*
**   ACSMX.H 
**
**
*/
#ifndef ACSMX_DFI_H
#define ACSMX_DFI_H

#include "comm.h"

#define ACSM_FAIL_STATE  0     
#define DFI_SCAN_NUM_PER_ORDER 10
/*
 * actually payload don't include the tcp/udp head
 * the length should be 1500 - 20 - [20|8] 
 * */
#define MAX_PAYLOAD_LENGTH 1501
#define MAX_MATCHED_NUM 8
/*
*   Prototypes
*/
typedef struct _acsm_pattern {
    ushort	num;
    ushort	patrn[DFI_SCAN_NUM_PER_ORDER*2];
} ACSM_PATTERN;

typedef struct {
    ushort matchNum;
    ushort combineNum;
    ushort* indexArray;  
    ushort* areaNum;  
    ushort* patterns;
}PATTERN_STRUCT;

typedef struct  {
 	uint id;				/* the appid */
 	uint flag;				/* if it's a accept state, flag = 1 */
	uint branchNum;
	/*ushort(65535 is enough for state num)*/
 	ushort NextState[0];	/* the size of nextstate table is decided by num of len_list */
}ACSM_STATETABLE; 

/*
* State machine Struct
*/
typedef struct {
    ushort acsmMaxStates;  
    ushort acsmNumStates;  
    ushort acsmPatternNum;  
    ushort acsmMapSize;
    ushort* acsmMapTable;
    ACSM_STATETABLE *acsmStateTable;
}ACSM_STRUCT;

/*
*   Prototypes
*/
extern ACSM_STRUCT * acmTCPTree;
extern ACSM_STRUCT * acmUDPTree;
ushort acsmRangeMap(ushort len);
int acsmCompile(ACSM_STRUCT* acsmTree, PATTERN_STRUCT* patterns, int n);
int acsm_init();
int acsmSearch (ACSM_STRUCT* acsmTree, ushort *Tx, int n, ushort matched_index[], ushort max_matched_num); 
void acsmFree ();
void acsmTreeFree (ACSM_STRUCT * acsmTree);
void acsmDump (ACSM_STRUCT* acsmTree);
void acsmAssistInit();
void acsmAssistDestroy();
	
/*
*    Simple STACK NODE
*/ 
typedef struct _stack
{
	int top;
	int maxNum;
	uint entity[0];
}STACK;

/*
*    Simple QUEUE NODE
*/ 
typedef struct _queue
{
	int num;
	int maxNum;
	int nodeSize;
	char acsmStateTable[0];
}QUEUE;

extern QUEUE *stateQue;
extern STACK *newStack;
extern STACK *oldStack;
#endif
