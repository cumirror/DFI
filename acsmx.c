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
	int nodeSize;
	char acsmStateTable[0];
}QUEUE;

/*
*Init the Queue
*/ 
static void queue_init (QUEUE ** s, int num, int nodeSize) 
{
	*s = (QUEUE*)AC_MALLOC(sizeof(QUEUE) + nodeSize*num);
	(*s)->num = 0;
	(*s)->nodeSize = nodeSize;
}

static void queue_destroy (QUEUE* s) 
{
	AC_FREE(s);
}

static int queue_num (QUEUE * s) 
{
	return s->num;
}

static ACSM_STATETABLE* queue_elem (QUEUE * s, int index) 
{
	return (ACSM_STATETABLE*)(s->acsmStateTable + s->nodeSize*index);
}

/*
*  Add Tail Item to queue
*/ 
static void queue_add (QUEUE * s, ACSM_STATETABLE* state) 
{
	memcpy(s->acsmStateTable + s->nodeSize*s->num, state, s->nodeSize);
	s->num += 1;
}


#define MAX_SCAN_PACKETS 20
#define MAX_NFA_STATE (MAX_SCAN_PACKETS+1)

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
*Init the STACK
*/ 
static void stack_init (STACK ** s) 
{
	*s = (STACK*)AC_MALLOC(sizeof(STACK) + sizeof(uint)*MAX_NFA_STATE);
	(*s)->top = 0;
	(*s)->maxNum = MAX_NFA_STATE;
}

static uint stack_pop (STACK * s)
{
	assert(s->top > 0);

	return s->entity[--s->top];
}

static void stack_push (STACK * s, uint index)
{
	assert(s->top < s->maxNum);
	s->entity[s->top++] = index;
}

static void stack_destroy (STACK* s) 
{
	AC_FREE(s);
}

static char stack_empty (STACK * s)
{
	return (char)(s->top == 0);
}

static void stack_clear (STACK * s)
{
	s->top = 0;
}

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
		printf("\t state[%d], id %4d, flag %4d, branchNum %4d\n",
			i, stateNode->id, stateNode->flag, stateNode->branchNum);
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

STACK *newStack, *oldStack;
QUEUE *stateQue;
void acsmAssistInit(ACSM_STRUCT * acsm)
{
	stack_init(&newStack);
	stack_init(&oldStack);
	queue_init (&stateQue, MAX_NFA_STATE, sizeof(ACSM_STATETABLE)+ acsm->acsmMapSize * sizeof(ushort));

	MEMASSERT ((newStack!=NULL && oldStack!=NULL && stateQue!=NULL), "acsmAssistInit");
}

/* where should i call this? */
void acsmAssistDestroy(ACSM_STRUCT * acsm)
{
	stack_destroy(oldStack);
	stack_destroy(newStack);
	queue_destroy(stateQue);
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

	AC_FREE(acsm->acsmStateTable);
	AC_FREE(acsm->acsmMapTable);
	acsmPatterns = acsm->acsmPatterns;
	while(acsmPatterns != NULL){
		next = acsmPatterns->next;
		AC_FREE(acsmPatterns);
		acsmPatterns = next;
	}
	acsmAssistDestroy(acsm);
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
		stateNode->branchNum += 1;
		state = acsm->acsmNumStates;
	}
	/*set the flag of accept state*/
	stateNode = (ACSM_STATETABLE * )(dfa+state*dfa_offset);
	stateNode->flag = 0xab;
	stateNode->id = p->id;
	stateNode->branchNum = 1;/*as a nomal state although it's accept state, it should be one*/
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

	acsmAssistInit(acsm);
	return 0;
}


/*
*   Search Text or Binary Data for Pattern matches
*   mod: 0--flexible; 1--strict
*/
int acsmSearch (ACSM_STRUCT * acsm, ushort *Tx, int n, int mod) 
{
	ushort st, *Tend, *T;
	ushort *lenArray, *lenIndex; 
	int mapSize, dfa_offset;
	STACK *tmpStack;
	ACSM_STATETABLE *currentNode, *nextNode;
	int currentIndex, nextIndex;
	ushort *dfa;

	dfa = (ushort*)(acsm->acsmStateTable);
	dfa_offset = sizeof(ACSM_STATETABLE)/sizeof(ushort) + acsm->acsmMapSize;
	lenArray = acsm->acsmMapTable;
	mapSize = acsm->acsmMapSize;

	stack_clear(newStack);
	stack_clear(oldStack);

	/*add state 0*/
	currentIndex = 0;
	currentNode = (ACSM_STATETABLE * )(dfa+dfa_offset);
	if(currentNode->branchNum > 1){	
		currentIndex |= 0x10000;
		currentIndex |= queue_num(stateQue);
		stack_push(oldStack, currentIndex);
		queue_add(stateQue, currentNode);
	} else {
		stack_push(oldStack, currentIndex);
	}

	for (Tend = Tx + n, T = Tx; T < Tend; T++) {
		lenIndex = bsearch(T, lenArray, mapSize, sizeof(ushort), cmp);
		if(lenIndex == NULL)
			continue;

		while(!stack_empty(oldStack)){
			currentIndex = stack_pop(oldStack);
			nextIndex = 0;
			if(currentIndex & 0x10000){
				currentNode = queue_elem(stateQue, currentIndex&0xFFFF);
			} else {
				currentNode = (ACSM_STATETABLE*)(dfa + (currentIndex&0xFFFF)*dfa_offset);
			}
			st = currentNode->NextState[lenIndex-lenArray];

			if(st != 0 ){
				if(currentIndex & 0x10000 && currentNode->branchNum > 1){
					stack_push(newStack, currentIndex);
					/*avoid recored no jump branch*/
					currentNode->branchNum -= 1;
					/*avoid state jump at this length again*/
					currentNode->NextState[lenIndex-lenArray] = 0;
				}
				nextNode = (ACSM_STATETABLE*)(dfa + st*dfa_offset);
				if(nextNode->branchNum > 1){	
					nextIndex |= 0x10000;
					nextIndex |= queue_num(stateQue);
					stack_push(newStack, nextIndex);
					queue_add(stateQue, nextNode);
				} else {
					nextIndex = st;
					stack_push(newStack, nextIndex);
				}				
			} else {
				/*jump failed, keep record orignal state*/
				stack_push(newStack, currentIndex);
			}
		}

		tmpStack = newStack;
		newStack = oldStack;
		oldStack = tmpStack;
		stack_clear(newStack);
	}

	/*output the result*/
	while(!stack_empty(oldStack)){
		currentIndex = stack_pop(oldStack);
		if(currentIndex & 0x10000){
			currentNode = queue_elem(stateQue, currentIndex&0xFFFF);
		} else {
			currentNode = (ACSM_STATETABLE*)(dfa + (currentIndex&0xFFFF)*dfa_offset);
		}
		if(currentNode->flag == 0xab){
			printf("Matched %d\n", currentNode->id);
		}
	}
	return 0;

}
