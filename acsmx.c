/*
**
** Multi-Pattern Search Engine
**
** use NFA for DFI
**              
** tongjin
**	
*/  

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "acsmx.h"
#include "partition.h"
//#include "common/acsmx.h"
//#include "slub.h"

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
 * as pattern was discribed as a range array, and a range maybe split to some little areas, 
 * so we need combine different areas, and output all the possible combinations as the real pattern! 
 */
void getPatterns(ushort* arrays, ushort* num, ushort count, ushort* patterns){
	int a, i, j, k, x, y, z, offset;
	int size = 1; 
	ushort *data, *array_data;
	for(a = 0; a < count; a++){
		size *= num[a];
	}
	x = 1;
	z = size;
	data = patterns;
	array_data = arrays;

	for(a = 0; a < count; a++){
		z /= num[a];
		offset = 0;

		for(i = 0; i < x; i++){
			for(j = 0, y = num[a]; j < y; j++){
				for(k = 0; k < z; k++){
					data[offset+a] = array_data[j];
					offset += count;
				}
			}
		}
		x *= num[a];
		array_data += num[a];
	}
}

/*
*Init the Queue
*/ 
static void queue_init (QUEUE ** s, int num, int nodeSize) 
{
	*s = (QUEUE*)kmalloc(sizeof(QUEUE) + nodeSize*num, MOD_DFI);
	if(*s){
		(*s)->num = 0;
		(*s)->maxNum = num;
		(*s)->nodeSize = nodeSize;
	}
}

static void queue_destroy (QUEUE* s) 
{
	if(s) kfree(s);
}

static int queue_clear (QUEUE * s) 
{
	return s->num = 0;
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
	if(s->num == s->maxNum)
		return;
	memcpy(s->acsmStateTable + s->nodeSize*s->num, state, s->nodeSize);
	s->num += 1;
}

/*
*Init the STACK
*/ 
static void stack_init (STACK ** s, int num) 
{
	*s = (STACK*)kmalloc(sizeof(STACK) + sizeof(uint)*num, MOD_DFI);
	if(*s) {	
		(*s)->top = 0;
		(*s)->maxNum = num;
	}
}

static uint stack_pop (STACK * s)
{
	assert(s->top > 0);
	return s->entity[--s->top];
}

static void stack_push (STACK * s, uint index)
{
	assert(s->top < s->maxNum);
	if(s->top == s->maxNum)
		return;
	s->entity[s->top++] = index;
}

static void stack_destroy (STACK* s) 
{
	if(s) kfree(s);
}

static char stack_empty (STACK * s)
{
	return (char)(s->top == 0);
}

static void stack_clear (STACK * s)
{
	s->top = 0;
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

void acsmDump (ACSM_STRUCT* acsmTree)
{
	if(!acsmTree)
		return;
	printf("ACSM info:\n");
	printf("\t acsmMaxStates: %d\n",   acsmTree->acsmMaxStates);
	printf("\t acsmNumStates: %d\n",   acsmTree->acsmNumStates);

//	mapTableDump(acsmTree->acsmMapTable, MAX_PAYLOAD_LENGTH);
	stateTableDump(acsmTree->acsmStateTable, acsmTree->acsmNumStates, acsmTree->acsmMapSize);
}

ACSM_STRUCT * acmTCPTree = NULL;
ACSM_STRUCT * acmUDPTree = NULL;
STACK *newStack = NULL;
STACK *oldStack = NULL;
QUEUE *stateQue = NULL;

void acsmAssistInit()
{
	int maxStateNum, maxMapSize;
	maxStateNum = maxMapSize = 0;
	if(!acmTCPTree && !acmUDPTree){
		printf("error, acsm Tree build failed!\n");
		return;
	}
	if(acmTCPTree){
		maxStateNum =  acmTCPTree->acsmPatternNum;
		maxMapSize =  acmTCPTree->acsmMapSize;
	}
	if(acmUDPTree){
		maxStateNum =  acmUDPTree->acsmPatternNum > maxStateNum ?
			acmUDPTree->acsmPatternNum : maxStateNum;
		maxMapSize =  acmUDPTree->acsmMapSize > maxMapSize ? 
			acmUDPTree->acsmMapSize : maxMapSize;
	}

	stack_init(&newStack, maxStateNum);
	stack_init(&oldStack, maxStateNum);
	queue_init(&stateQue, maxStateNum, sizeof(ACSM_STATETABLE)+ maxMapSize * sizeof(ushort));

	assert(newStack != NULL && oldStack != NULL && stateQue != NULL);
	if(newStack == NULL || oldStack == NULL || stateQue == NULL){
		printf("acsmAssistInit failed\n");
	}
}

void acsmAssistDestroy()
{
	stack_destroy(oldStack);
	stack_destroy(newStack);
	queue_destroy(stateQue);
	newStack = oldStack = NULL;
	stateQue = NULL;
}

void acsmTreeFree (ACSM_STRUCT* acsmTree) 
{
	if(!acsmTree)return;	
	
	if(acsmTree->acsmMapTable != NULL){
		kfree(acsmTree->acsmMapTable);
		acsmTree->acsmMapTable = NULL;
	}
	
	if(acsmTree->acsmStateTable != NULL){
		kfree(acsmTree->acsmStateTable);
		acsmTree->acsmStateTable = NULL;
	}
	kfree(acsmTree);
}

void acsmFree() 
{
	acsmTreeFree(acmTCPTree); 
	acsmTreeFree(acmUDPTree); 
	acmTCPTree = NULL;
	acmUDPTree = NULL;
}

/* 
 * Add Pattern States
 */ 
static void AddPatternStates (ACSM_STRUCT * acsm, ushort* patrn, int num, int id) 
{
	ushort *pattern;
	int state=0, next, n;
	ushort* dfa = NULL;
	int dfa_offset;
	ACSM_STATETABLE * stateNode;

	dfa = (ushort*)(acsm->acsmStateTable);
	dfa_offset = sizeof(ACSM_STATETABLE)/sizeof(ushort) + acsm->acsmMapSize;

	/*Match up pattern with existing states*/
	for (n = num, pattern = patrn; n > 0; pattern++, n--){
		assert(*pattern != 0);
		stateNode = (ACSM_STATETABLE * )(dfa+state*dfa_offset);
		next = stateNode->NextState[*pattern];
		if (next == ACSM_FAIL_STATE)
			break;
		state = next;
	}

	/* Add new states for the rest of the pattern*/ 
	for (; n > 0; pattern++, n--){
		assert(*pattern != 0);
		acsm->acsmNumStates++;
		stateNode = (ACSM_STATETABLE * )(dfa+state*dfa_offset);
		stateNode->NextState[*pattern] = acsm->acsmNumStates;
		stateNode->branchNum += 1;
		state = acsm->acsmNumStates;
	}

	/*set the flag of accept state*/
	if(stateNode->flag == 0xab){
		printf("warning: appid:%4d's pattern is the same with appid:%4d\n",
				stateNode->id, id);
	}
	stateNode = (ACSM_STATETABLE * )(dfa+state*dfa_offset);
	stateNode->flag = 0xab;
	stateNode->id = id;
	stateNode->branchNum += 1;
}

/*
*   Compile State Machine
*/ 
int acsmCompile(ACSM_STRUCT* acsmTree, PATTERN_STRUCT* patterns, int n) 
{
	PATTERN_STRUCT* p;
	ushort* pattern;
	int num, stateSize, i, j;

	/*build the state table*/
	acsmTree->acsmMaxStates += 1;	/*add state 0*/
	stateSize = sizeof(ACSM_STATETABLE) + (acsmTree->acsmMapSize)*sizeof(ushort);
	acsmTree->acsmStateTable = (ACSM_STATETABLE *)
		kzalloc(stateSize * (acsmTree->acsmMaxStates), MOD_DFI);
	if(!acsmTree->acsmStateTable){
		printf("allocate stateTable error!\n");
		return 1;
	}

	/* Initialize state zero as a branch */
	acsmTree->acsmNumStates = 0;

	/* Add each Pattern to the State Table */
	for(i = 0, p = patterns; i < n; i++, p++){
		for(j = 0, num = p->combineNum, pattern = p->patterns; j < num; j++){
			AddPatternStates(acsmTree, pattern, p->matchNum, i);
			pattern += p->matchNum;
		}
	}

	return 0;
}

/*
 *   Search packet length!
 */
int acsmSearch(ACSM_STRUCT* acsmTree, ushort *Tx, int n, ushort matched_index[], ushort max_matched_num) {
	ushort st, *Tend, *T;
	ushort *mapTable, index; 
	int mapSize, dfa_offset;
	STACK *tmpStack;
	ACSM_STATETABLE *currentNode, *nextNode;
	int currentIndex, nextIndex;
	ushort *dfa;
	int nmatched = 0;

	if(!acsmTree){
		return nmatched;
	}
	dfa = (ushort*)(acsmTree->acsmStateTable);
	dfa_offset = sizeof(ACSM_STATETABLE)/sizeof(ushort) + acsmTree->acsmMapSize;
	mapTable = acsmTree->acsmMapTable;
	mapSize = acsmTree->acsmMapSize;

	stack_clear(newStack);
	stack_clear(oldStack);
	queue_clear(stateQue); 
	
	/*add state 0*/
	currentIndex = 0;
	currentNode = (ACSM_STATETABLE * )dfa;
	if(currentNode->branchNum > 1){	
		currentIndex |= 0x10000;
		currentIndex |= queue_num(stateQue);
		stack_push(oldStack, currentIndex);
		queue_add(stateQue, currentNode);
	} else {
		stack_push(oldStack, currentIndex);
	}

	for (Tend = Tx + n, T = Tx; T < Tend; T++) {
		assert(*T <= 1500);
		if(*T > 1500) continue;
		index = mapTable[*T];
		if(index == 0)
			continue;

		while(!stack_empty(oldStack)){
			currentIndex = stack_pop(oldStack);
			nextIndex = 0;
			if(currentIndex & 0x10000){
				currentNode = queue_elem(stateQue, currentIndex&0xFFFF);
			} else {
				currentNode = (ACSM_STATETABLE*)(dfa + (currentIndex&0xFFFF)*dfa_offset);
			}
			st = currentNode->NextState[index];

			if(st != 0 ){
				if(currentIndex & 0x10000 && currentNode->branchNum > 1){
					stack_push(newStack, currentIndex);
					/*avoid recored no jump branch*/
					currentNode->branchNum -= 1;
					/*avoid state jump at this length again*/
					currentNode->NextState[index] = 0;
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

				/*record the accept node*/
				if(nextNode->flag == 0xab){
					matched_index[nmatched] = nextNode->id;
					nmatched++;
					if (nmatched == max_matched_num)
						goto ret;
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

ret:
	return nmatched;
}

void dfiSearch()
{
	unsigned short lenArray1[DFI_SCAN_NUM_PER_ORDER*2+1] = {12, 32, 55, 78, 21, 92, 115, 135, 128, 150, 156, 161, 170, 180, 134};/* 3 time */
	unsigned short lenArray2[DFI_SCAN_NUM_PER_ORDER*2+1] = {12, 42, 47, 221, 36, 52, 61, 72, 128, 1500, 42, 49, 50, 136, 127};/* 4 time */
	unsigned short lenArray3[DFI_SCAN_NUM_PER_ORDER*2+1] = {12, 42, 47, 221, 36, 52, 61, 72, 128, 1500, 42, 49, 50, 46, 46};/* 0 time */
	int match_num;
	int size = 15;
	unsigned short matched[MAX_MATCHED_NUM]; 
	if(!acmTCPTree && !acmUDPTree){
		printf("acsm tree not build!\n");
		return ;
	}
	if(newStack ==NULL || oldStack ==NULL || stateQue ==NULL){
		printf("acsmAssistInit allocate failed\n");
		return ;
	}
	
	match_num = acsmSearch (acmTCPTree, lenArray1, size, matched, MAX_MATCHED_NUM);
	if(match_num > 0){
		printf("\nmatch %d time\n", match_num);
	} else {
		printf("\n no match!\n");
	}

	match_num = acsmSearch (acmUDPTree, lenArray2, size, matched, MAX_MATCHED_NUM);
	if(match_num > 0){
		printf("\nmatch %d time\n", match_num);
	} else {
		printf("\n no match!\n");
	}

	match_num = acsmSearch (acmTCPTree, lenArray3, size, matched, MAX_MATCHED_NUM);
	if(match_num > 0){
		printf("\nmatch %d time\n", match_num);
	} else {
		printf("\n no match!\n");
	}
	match_num = acsmSearch (acmUDPTree, lenArray3, size, matched, MAX_MATCHED_NUM);
	if(match_num > 0){
		printf("\nmatch %d time\n", match_num);
	} else {
		printf("\n no match!\n");
	}
}

ushort rangePattern1[45][2] = {{40,43},{47,49},{220,222},{50,54},{60,64},{1500,1500},{40,43},{49,54},{127,129},
							   {40,43},{47,49},{220,222},{50,54},{60,64},{1500,1500},{40,43},{49,54},{136,138},
							   {40,43},{20,23},{220,222},{50,54},{60,64},{1500,1500},{40,43},{49,54},{134,134},
							   {10,20},{30,40},{50,60},{70,80},{90,100},{110,120},{130,140},{150,160},{170,180},
							   {10,20},{30,40},{50,60},{70,80},{90,100},{110,120},{130,140},{155,175},{180,190}
							  };
typedef ushort (*RANGE)[2];
int acsmTreeInit(ACSM_STRUCT ** acsmTree, ushort (*rangePatterns)[2], int psNum)
{
	int i, j;
	int ret = 0;
	ushort areaIndex = 0;
	ushort *indexArrays, *nums, *Array, *patterns, *lenMapTable;
	ushort patternNum, prefixNum;
	ushort (*rangePattern)[2];
	PATTERN_STRUCT* mapPatterns;
	ACSM_STRUCT * tree = NULL;

	tree = (ACSM_STRUCT *) kzalloc(sizeof(ACSM_STRUCT), MOD_DFI);
	*acsmTree = tree;
	if(tree == NULL){
		ret = 1;
		goto err;
	}
	lenMapTable = (ushort *) kzalloc(MAX_PAYLOAD_LENGTH * sizeof(ushort), MOD_DFI);
	if(lenMapTable == NULL){
		ret = 2;
		goto err;
	}
	tree->acsmMapTable = lenMapTable;
	mapPatterns = (PATTERN_STRUCT*)kzalloc(sizeof(PATTERN_STRUCT)*psNum, MOD_DFI);
	if(mapPatterns == NULL){
		ret = 3;
		goto err;
	}

	if(rangeInit(MAX_PAYLOAD_LENGTH) != 0){
		ret = 4;
		goto err;
	}
	
	for(j = 0; j < psNum; j++){
		rangePattern = (RANGE)(rangePatterns[9*j]);
		/*get the min area*/
		for(i = 0; i < 9; i++){
			setRange(lenMapTable, rangePattern[i]);
		}
		mapPatterns[j].matchNum = 9;
	}
	areaIndex = buildArea(lenMapTable);
	tree->acsmMapSize = areaIndex + 1;
#if 0
	printf("\nArea num %d\n", areaIndex);
	for(i = 0; i < MAX_PAYLOAD_LENGTH; i++){
		if(i%10 == 0)
			printf("\n");
		printf("%d  ", lenMapTable[i]);
	}
#endif
	for(i = 0; i < psNum; i++){
		mapPatterns[i].indexArray = indexArrays = (ushort*)kmalloc
			(areaIndex * sizeof(ushort) * mapPatterns[i].matchNum, MOD_DFI);
		mapPatterns[i].areaNum = nums = (ushort*)kmalloc
			(sizeof(ushort) *mapPatterns[i].matchNum, MOD_DFI);
		if(indexArrays == NULL || nums == NULL){
			ret = 5;
			goto err;
		}
		Array = indexArrays;
		rangePattern = (RANGE)(rangePatterns[i*9]);
		for(j = 0; j < 9; j++){
			nums[j] = range2Areas(lenMapTable, rangePattern[j], Array);
			Array += nums[j];
		}
		j = 0;
		mapPatterns[i].combineNum = 1;
		patternNum = 1;
		while(j < mapPatterns[i].matchNum && nums[j] == 1){
			mapPatterns[i].combineNum *= nums[j];
			j++;
		}
		prefixNum = j;
		for( ; j < mapPatterns[i].matchNum; j++){
			mapPatterns[i].combineNum *= nums[j];
			patternNum *= nums[j];
		}
		tree->acsmMaxStates += prefixNum + patternNum*(mapPatterns[i].matchNum - prefixNum);
		patternNum = mapPatterns[i].combineNum;
		mapPatterns[i].patterns = patterns = (ushort*)kmalloc(
				(mapPatterns[i].matchNum) * patternNum * sizeof(ushort), MOD_DFI);
		if(patterns == NULL){
			ret = 6;
			goto err;
		}
		getPatterns(indexArrays, nums, mapPatterns[i].matchNum, patterns);
		tree->acsmPatternNum += patternNum;
	}

	ret = acsmCompile(tree, mapPatterns, psNum);

err:
	rangeDestroy();
	
	if(mapPatterns != NULL){
		for(i = 0; i < psNum; i++){
			if(mapPatterns[i].indexArray != NULL){
				kfree(mapPatterns[i].indexArray);
			}
			if(mapPatterns[i].areaNum != NULL){
				kfree(mapPatterns[i].areaNum);
			}
			if(mapPatterns[i].patterns != NULL){
				kfree(mapPatterns[i].patterns);
			}
		}
		free(mapPatterns);
	}
	if(ret != 0){
		acsmTreeFree(tree);
		*acsmTree = NULL;
	}
	return ret;
}

int acsm_init()
{
	if(acsmTreeInit(&acmUDPTree, rangePattern1, 3) != 0){
		return 1;
	}
	if(acsmTreeInit(&acmTCPTree, rangePattern1+3*9, 2) != 0){
		return 2;
	}
	return 0;
}
