#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "partition.h"

static ushort* shadow = NULL;
static ushort rangeSize = 0;

static void refresh(ushort* map, ushort len)
{
	int i;
	for(i = 0; i < len; i++){
		if(map[i] == 1)
			map[i] = 0;
		else if(map[i] == 0)
			map[i] = 1;
	}
}

int rangeInit(ushort rangeLen)
{
	shadow = (ushort*)kzalloc(rangeLen * sizeof(ushort), MOD_DFI);
	if(!shadow){
		return 1;
	}
	rangeSize = rangeLen;
	return 0;
}

void rangeDestroy()
{
	if(shadow){
		kfree(shadow);
		rangeSize = 0;
		shadow = NULL;
	}
}

/*
  * all the range start from 1, not 0
  * if start from 0, whole[left-1]will be error.
  *
  */
void setRange(ushort* whole, ushort range[2])
{
	ushort left, right, i, color, state;
	left = range[0];
	right = range[1];

	color = whole[left-1];
	color = !color;
	state = 0;
	for(i = left; i <= right; i++){
		if(shadow[i] == 0)
			shadow[i] = 1;
	}

	for(i = left, state = 0; i <= right; i++){
		//if overlapping occur at the beginning, all we need is refresh!
		if(whole[i] == color && state == 0){ 
			break;
		}
		state = 1;

		if(whole[i] == color){
			color = !color;
		}
		whole[i] = color;
	}

	if(right < (rangeSize - 1) && whole[right+1] == whole[right])
		refresh(whole+right+1, rangeSize-right-1);
}

ushort buildArea(ushort* whole)
{
	int i, state;
	int count = 0;

	state = 2;		/* not 0 or 1 */
	for(i = 0; i < rangeSize; i++){
		while(i < rangeSize && shadow[i] == 0){ 
			state = whole[i];
			whole[i] = 0;			/* not used area */
			i++;
		}
		if(i == rangeSize)break;
		if(state != whole[i]){	
			count++;
			state = whole[i];
		}
		if(state == whole[i]){
			whole[i] = count;		/* set area index, begin from 1 */
		}
	}
	return count;
}

ushort range2Areas(ushort* whole, ushort range[2], ushort* areas)
{
	ushort indexNum = 0;
	ushort index = rangeSize+1;		/* a impossible index value*/
	ushort i;
	for(i = range[0]; i <= range[1]; i++){
		if(index != whole[i]){
			/* record new area in areas */
			areas[indexNum++] = index = whole[i];
		}
	}
	return indexNum;
}