#ifndef __PARTITION__H
#define __PARTITION__H

#include "comm.h"

int rangeInit(ushort rangeLen);
void rangeDestroy();
void setRange(ushort* whole, ushort range[2]);
ushort buildArea(ushort* whole);
ushort range2Areas(ushort* whole, ushort range[2], ushort* areas);

#endif