#include "acsmx.h"
#include <stdlib.h>

ushort packet_length1[18] = {181, 9, 137, 12, 18, 30, 18, 18, 30, 36, 8, 39, 14, 6, 56, 1460, 1460, 1460};
ushort packet_length2[18] = {12, 9, 137, 12, 18, 30, 18, 18, 30, 36, 8, 39, 14, 6, 56, 1460, 1460, 1460};
ushort packet_length3[18] = {9, 137, 12, 18, 30, 18, 18, 30, 36, 8, 39, 14, 6, 56, 1460, 1460, 1460};
ushort pattern1[] = {9, 18, 30, 18};
ushort pattern2[] = {181, 18, 56, 1460, 1460};
ushort pattern3[] = {9, 137, 12, 18};

int main (int argc, char **argv) 
{
	ACSM_STRUCT * acsm;
	int id;

	acsm = acsmNew();
	/*assume pattern is a int array!*/
	acsmAddPattern (acsm, 1, pattern1, sizeof(pattern1)/sizeof(ushort));
	acsmAddPattern (acsm, 2, pattern2, sizeof(pattern2)/sizeof(ushort));
	acsmAddPattern (acsm, 3, pattern3, sizeof(pattern3)/sizeof(ushort));	
	
	/* Generate GtoTo Table, no need for Fail Table */
	acsmCompile(acsm);

	acsmDump (acsm);
	
#if 1

	//mod不起作用
	printf("search1\n");
	acsmSearch(acsm, packet_length1, sizeof(packet_length1)/sizeof(ushort), 0);
	printf("search2\n");
	acsmSearch(acsm, packet_length2, sizeof(packet_length2)/sizeof(ushort), 0);
	printf("search3\n");
	acsmSearch(acsm, packet_length2, sizeof(packet_length2)/sizeof(ushort), 1);
	printf("search4\n");
	acsmSearch(acsm, packet_length3, sizeof(packet_length3)/sizeof(ushort), 1);

#else
	
	//mod起作用，看注释处代码
	id = acsmSearch(acsm, packet_length1, sizeof(packet_length1)/sizeof(ushort), 0);
	printf("flex mod match id %d\n", id);

	id = acsmSearch(acsm, packet_length2, sizeof(packet_length2)/sizeof(ushort), 0);
	printf("flex mod match id %d\n", id);
	
	id = acsmSearch(acsm, packet_length2, sizeof(packet_length2)/sizeof(ushort), 1);
	printf("strict mod match id %d\n", id);

	id = acsmSearch(acsm, packet_length3, sizeof(packet_length3)/sizeof(ushort), 1);
	printf("strict mod match id %d\n", id);
	
#endif
	acsmFree (acsm);

	system("pause");
	
	return (0);
}

