#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <string>
#include <map>
#include <iostream>
using namespace std;

unordered_map<string, int> flows;
map<int, int> fsd;



int main(int argc, char *argv[])
{
	if(argc != 3){
		exit(1);
	}

	FILE *fin = fopen(argv[1], "rb");

	char buf[25];
	int packet_cnt = 0;
	int Len = 4;
	while(fread(buf, Len, 1, fin))
	{
		string s(buf, Len);
		flows[s] += 1;
		packet_cnt++;
//		if(packet_cnt % 1000000 == 0)
//			printf("packet-cnt=%d\n", packet_cnt);
//		if(packet_cnt == 20000000)
//			break;
	}
	fclose(fin);

	for(unordered_map<string, int>::iterator it = flows.begin(); it != flows.end(); ++it)
	{
		fsd[it->second] += 1;
	}

	printf("%lu flows, %d packets\n", flows.size(), packet_cnt);


	FILE *fout = fopen(argv[2], "w");
	fprintf(fout, "%lu flows, %d packets\n", flows.size(), packet_cnt);
	fprintf(fout, "flow-size\t\tnum-of-flows\n");
	for(map<int, int>::iterator it = fsd.begin(); it != fsd.end(); ++it){
		fprintf(fout, "%-8d\t\t%d\n", it->first, it->second);
	}
	fclose(fout);

	return 0;
}
