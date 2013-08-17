#ifndef __COMM__H
#define __COMM__H

typedef unsigned short ushort;
typedef unsigned int uint;

#define MOD_DFI 10

extern void* kmalloc(int size, int module);
extern void kfree(void* p);
extern void* kzalloc(int size, int module);

#endif
