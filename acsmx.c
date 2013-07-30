/*
**
** Multi-Pattern Search Engine
**
** Aho-Corasick State Machine -  uses a Deterministic Finite Automata - DFA
**              
**  tongjin : modify it for DFI
**	
*/  

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "acsmx.h"

#define MEMASSERT(p,s) if(!p){fprintf(stderr,"ACSM-No Memory: %s!\n",s);exit(0);}

int rm_dup(ushort* array, ushort num)
{
	ushort *a, *b;
	a = array;
	b = a + 1;	
	while(b - array != num){
		if(*a != *b) 
			*(++a) = *(b++);
		else	
			b++;	
	}	
	return (int)(a - array + 1);
}

int cmp ( const void *a , const void *b )
{
	return *(ushort *)a - *(ushort *)b;
}

/*
* Malloc the AC Memory
*/ 
static void *AC_MALLOC (int n) 
{
	void *p;
	p = malloc (n);

	return p;
}

/*
*Free the AC Memory
*/ 
static void AC_FREE (void *p) 
{
	if (p)
		free (p);
}

/*
*    Simple QUEUE NODE
*/ 
typedef struct _queue
{
	int num;
	int newNum;
	ushort entity[0];
}QUEUE;

/*
*Init the Queue
*/ 
static void queue_init (QUEUE ** s, int num) 
{
	*s = (QUEUE*)AC_MALLOC(sizeof(QUEUE) + sizeof(ushort)*num);
	(*s)->num = 0;
	(*s)->newNum = 0;
}

static int queue_num (QUEUE * s) 
{
	return s->num;
}

static int queue_elem (QUEUE * s, int index) 
{
	return s->entity[index];
}

/*
*  Add Tail Item to queue
*/ 
static void queue_add (QUEUE * s, ushort state) 
{
	s->entity[s->num + s->newNum] = state;
	s->newNum += 1;
}

static void queue_addFin (QUEUE * s) 
{
	s->num += s->newNum;
	s->newNum = 0;
}

/*
static char empty (STACK * s)
{
	return (char)(s->top < 0);
}

static void clear (STACK * s)
{
	s->top = 0;
}
*/
void patternDump(ACSM_PATTERN * pattern)
{
	ACSM_PATTERN * mlist = pattern;
	uint i;

	printf("Patterns info:\n");
	for (;mlist!=NULL;mlist=mlist->next){
		printf("\t Pattern info:\n");
		for(i = 0; i < mlist->num; i++){
			printf("%4d  ", mlist->patrn[i]);
		}
		printf("\n");  
	}
}

void nextTableDump(ushort * nextTable, int nextSize)
{
	int i;
	for(i = 0; i < nextSize; i++){
		printf("%4d  ", nextTable[i]);
	}
	printf("\n");

}

void stateTableDump(ACSM_STATETABLE * stateTable, int num, int nextSize)
{
	int i;
	ushort* dfa = NULL;
	int offset, dfa_offset;
	ACSM_STATETABLE * stateNode;

	dfa = (ushort *)stateTable;
	offset = sizeof(ACSM_STATETABLE)/sizeof(ushort);
	dfa_offset = offset + nextSize;
	
	printf("StateTable info:\n");
	for(i = 0; i < num; i++){
		stateNode = (ACSM_STATETABLE *)dfa;
		printf("\t state[%d], id %4d, flag %4d\n", i, stateNode->id, stateNode->flag);
		printf("\t nextTable:\n");
		nextTableDump(stateNode->NextState, nextSize);
		dfa += dfa_offset;
	}
}

void mapTableDump(ushort * a, uint num)
{
	uint i;
	printf("mapTableSize: %d\n",   num);

	for(i = 0; i < num; i++){
		printf("%4d  ", a[i]);
	}
	printf("\n");
}

void acsmDump (ACSM_STRUCT * acsm)
{
	patternDump(acsm->acsmPatterns);

	printf("ACSM info:\n");
	printf("\t acsmMaxStates: %d\n",   acsm->acsmMaxStates);
	printf("\t acsmNumStates: %d\n",   acsm->acsmNumStates);

	mapTableDump(acsm->acsmMapTable, acsm->acsmMapSize);
	stateTableDump(acsm->acsmStateTable, acsm->acsmNumStates, acsm->acsmMapSize);
}

/*
* Init the acsm DataStruct
*/ 
ACSM_STRUCT * acsmNew () 
{
	ACSM_STRUCT * p;
	p = (ACSM_STRUCT *) AC_MALLOC (sizeof (ACSM_STRUCT));
	MEMASSERT (p, "acsmNew");
	if (p) memset (p, 0, sizeof (ACSM_STRUCT));
	return p;
}

void acsmFree (ACSM_STRUCT * acsm) 
{
    	ACSM_PATTERN *acsmPatterns, *next;

	free(acsm->acsmStateTable);
	free(acsm->acsmMapTable);
	acsmPatterns = acsm->acsmPatterns;
	while(acsmPatterns != NULL){
		next = acsmPatterns->next;
		free(acsmPatterns);
		acsmPatterns = next;
	}
}

/* 
* Add Pattern States
*/ 
static void AddPatternStates (ACSM_STRUCT * acsm, ACSM_PATTERN * p) 
{
	ushort *pattern;
	ushort *index, *lenArray;
	int state=0, next, n;
	int size;
	ushort* dfa = NULL;
	int dfa_offset;
	ACSM_STATETABLE * stateNode;

	dfa = (ushort*)(acsm->acsmStateTable);
	dfa_offset = sizeof(ACSM_STATETABLE)/sizeof(ushort) + acsm->acsmMapSize;

	lenArray = acsm->acsmMapTable;
	size = acsm->acsmMapSize;
	/*Match up pattern with existing states*/
	for (n = p->num, pattern = p->patrn; n > 0; pattern++, n--){
		index = bsearch(pattern, lenArray, size, sizeof(ushort), cmp);
		assert(index != NULL);
		stateNode = (ACSM_STATETABLE * )(dfa+state*dfa_offset);
		next = stateNode->NextState[index-lenArray];
		if (next == ACSM_FAIL_STATE)
			break;
		state = next;
	}

 	/* Add new states for the rest of the pattern*/ 
	for (; n > 0; pattern++, n--){
		index = bsearch(pattern, lenArray, size, sizeof(ushort), cmp);
		assert(index != NULL);
		acsm->acsmNumStates++;
		stateNode = (ACSM_STATETABLE * )(dfa+state*dfa_offset);
		stateNode->NextState[index-lenArray] = acsm->acsmNumStates;
		state = acsm->acsmNumStates;
	}
	/*set the flag of accept state*/
	stateNode = (ACSM_STATETABLE * )(dfa+state*dfa_offset);
	stateNode->flag = 0xab;
	stateNode->id = p->id;
}



/*
*   Add a pattern to the list of patterns for this state machine
*/ 
int acsmAddPattern (ACSM_STRUCT * p, uint id, ushort *pat, uint n) 
{
	ACSM_PATTERN * plist;
	plist = (ACSM_PATTERN *) AC_MALLOC (sizeof (ACSM_PATTERN) + sizeof(ushort) * n);
	MEMASSERT (plist, "acsmAddPattern");
	memcpy (plist->patrn, pat, sizeof(ushort)*n);
	plist->num = n;
	plist->id = id;

	/*Add the pattern into the pattern list, insert from head*/
	plist->next = p->acsmPatterns;
	p->acsmPatterns = plist;
	p->acsmMaxStates += n;
	return 0;
}

/*
*   Compile State Machine
*/ 
int acsmCompile (ACSM_STRUCT * acsm) 
{
	ACSM_PATTERN * plist;
	ushort* len_list;
	int num, stateSize;
	
	/*sort the length and remove duplicate*/
	len_list = (ushort *) AC_MALLOC (sizeof (ushort) * (acsm->acsmMaxStates));
	MEMASSERT (len_list, "acsmCompile: mapTable");
	for (num = 0, plist = acsm->acsmPatterns; plist != NULL; plist = plist->next){
		memcpy (len_list+num, plist->patrn, sizeof(ushort)*(plist->num));
		num += plist->num;
	}
	assert(num == acsm->acsmMaxStates);
	qsort(len_list, acsm->acsmMaxStates, sizeof(ushort), cmp);
	num = rm_dup(len_list, acsm->acsmMaxStates);

	/*we can use the len_list as the mapping table*/
	/*the index in the array used for the state's next*/
	acsm->acsmMapTable = len_list;
	acsm->acsmMapSize = num;
	
	/*build the state table*/
	acsm->acsmMaxStates += 1;	/*add state 0*/
	assert(acsm->acsmMaxStates < 65535);
	stateSize = sizeof(ACSM_STATETABLE) + num*sizeof(ushort);
	acsm->acsmStateTable = (ACSM_STATETABLE *) AC_MALLOC (stateSize*(acsm->acsmMaxStates));
	MEMASSERT (acsm->acsmStateTable, "acsmCompile: stateTable");
	memset (acsm->acsmStateTable, 0, stateSize*(acsm->acsmMaxStates));

	/* Initialize state zero as a branch */
	acsm->acsmNumStates = 0;

	/* Add each Pattern to the State Table */ 
	for (plist = acsm->acsmPatterns; plist != NULL; plist = plist->next){
		AddPatternStates (acsm, plist);
	}
	
	return 0;
}

/*
*   Search Text or Binary Data for Pattern matches
*   mod: 0--flexible; 1--strict
*/ 
int acsmSearch (ACSM_STRUCT * acsm, ushort *Tx, int n, int mod) 
{
	ushort state, st;
	ushort *Tend, *T;
	ushort* lenArray, *lenIndex; 
	int size = acsm->acsmMapSize;
	ushort* dfa;
	int dfa_offset;
	ACSM_STATETABLE * stateNode;

	int i;
	QUEUE *stateQue;
	char* stateExist;

	dfa = (ushort*)(acsm->acsmStateTable);
	dfa_offset = sizeof(ACSM_STATETABLE)/sizeof(ushort) + acsm->acsmMapSize;
	lenArray = acsm->acsmMapTable;

	Tend = Tx + n;
	T = Tx;
	state = st = 0;
	stateNode = (ACSM_STATETABLE * )(dfa+state*dfa_offset);

	
	queue_init(&stateQue, acsm->acsmNumStates + 1);

	stateExist = (char*)AC_MALLOC(sizeof(char)*(acsm->acsmNumStates + 1));
	memset(stateExist, 0, sizeof(char)*(acsm->acsmNumStates + 1));

	queue_add(stateQue, 0);
	queue_addFin(stateQue);

#if 1
	for (; T < Tend; T++) {
		lenIndex = bsearch(T, lenArray, size, sizeof(ushort), cmp);
		if(lenIndex == NULL)
			continue;

		/*还能优化，只保存有分支的节点*/
		for(i = 0; i < queue_num(stateQue); i++){

			state = queue_elem(stateQue, i);
			stateNode = (ACSM_STATETABLE *)(dfa+state*dfa_offset);
			st = stateNode->NextState[lenIndex-lenArray];

			if(st != 0 && !stateExist[st]){
				queue_add(stateQue, st);
				stateExist[st] = true;
			}				
		}
		queue_addFin(stateQue);
	}

	for(i = 0; i < queue_num(stateQue); i++){
		state = queue_elem(stateQue, i);
		stateNode = (ACSM_STATETABLE *)(dfa+state*dfa_offset);

		if(stateNode->flag == 0xab){
			printf("Matched %d\n", stateNode->id);
		}
	}
	return 0;

#else
	
	for (state = 0; T < Tend; T++) {
		lenIndex = bsearch(T, lenArray, size, sizeof(ushort), cmp);
		if(lenIndex == NULL)
			continue;

		stateNode = (ACSM_STATETABLE * )(dfa+state*dfa_offset);
		st = stateNode->NextState[lenIndex-lenArray];

		if(st == 0){
			if(mod)
				break;
			else 
				continue;
		} else { 
			state = st;
		}
		
		stateNode = (ACSM_STATETABLE * )(dfa+state*dfa_offset);
		// State is a accept state? 
		if( stateNode->flag == 0xab ) {
			break;
		}
	}
	return stateNode->id;

#endif	
}
