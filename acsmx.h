/*
**   ACSMX.H 
**
**
*/
#ifndef ACSMX_H
#define ACSMX_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned char uchar;

#define true 1
#define false 0

/*
*   Prototypes
*/

#define ACSM_FAIL_STATE  0     

typedef struct _acsm_pattern {
    struct  _acsm_pattern *next;
    uint 	id;
    uint	num;
    ushort	patrn[0];
} ACSM_PATTERN;


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
    int acsmMapSize;
    ushort* acsmMapTable;
    ACSM_PATTERN    *acsmPatterns;
    ACSM_STATETABLE *acsmStateTable;
}ACSM_STRUCT;

/*
*   Prototypes
*/
ACSM_STRUCT * acsmNew ();
int acsmAddPattern( ACSM_STRUCT* p, uint id, ushort* pat, uint n);
int acsmCompile ( ACSM_STRUCT* acsm );
int acsmSearch ( ACSM_STRUCT* acsm, ushort* T, int n, int mod);
void acsmFree ( ACSM_STRUCT* acsm );
void acsmDump ( ACSM_STRUCT* acsm );

#endif
