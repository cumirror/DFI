#include <stdlib.h>
#include "acsmx.h"

void* kmalloc(int size, int module)
{
	return malloc(size);
}

void kfree(void* p)
{
	free(p);
}

void* kzalloc(int size, int module)
{
	return calloc(1, size);
}

int main(int argc, char** argv)
{
	acsm_init();
	acsmDump(acmTCPTree);
	acsmDump(acmUDPTree);
	acsmAssistInit();
	dfiSearch();
	acsmAssistDestroy();
	acsmFree();
	return 0;
}
