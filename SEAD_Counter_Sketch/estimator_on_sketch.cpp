#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>
#include <unordered_map>
#include <map>
#include <fstream>
#include <chrono>
#include <limits>
#include "sketch.h"
#include "cmsketch.h"
#include "cusketch.h"
#include "params.h"
#include "sac.h"
#include "ICE_bucket.h"
#include "SmallActiveCounter.h"
#include "counter_tree.h"
#include <algorithm>
#include <vector>

using namespace std;

unordered_map<string, int> flows;
map<int, int> fsd;

const int max_estimator1 = (1 << per_estimator_int) - 1;

char SmallActiveCounter::q = per_estimator_int;
char SmallActiveCounter::mode = per_estimator_mode;
char SmallActiveCounter::A = per_estimator_int - per_estimator_mode;
int SmallActiveCounter::r = 1;

double average_err[3 * KIND_OF_COUNTERS];
double average_sig[3 * KIND_OF_COUNTERS];
double average_time[KIND_OF_COUNTERS];

char insert[30000000 + 10000000 / 5][15];
char query[30000000 + 10000000 / 5][15];

bool comp(pair<string, int> &a1, pair<string, int> &a2)
{
	return a1.second < a2.second;
}

int main(int argc, char *argv[])
{
	char filename[35];
	if (argc != 4)
	{
		printf("usage: exe_filename read_filename sketch_number sketch_array\n\n");
		printf("sketch_number: 0: CM, 1, CU\n");
		printf("sketch_array should be a postive integer less than 18\n");
		exit(1);
	}

	FILE *fin = fopen(argv[1], "rb");

	int sketch_number, sketch_array;

	if (sscanf(argv[2], "%i", &sketch_number) != 1)
	{
		printf("sketch_number is invalid!");
		exit(1);
	}
	if (sketch_number != 0 && sketch_number != 1)
	{
		printf("sketch_number: 0: CM, 1: CU");
		exit(1);
	}
	if (sscanf(argv[3], "%i", &sketch_array) != 1)
	{
		printf("sketch_array is invalid!");
		exit(1);
	}
	if (sketch_array <= 0 || sketch_array >= 19)
	{
		printf("sketch_array: between 1~18!");
		exit(1);
	}

	int memory_use = 1024;

	char buf[25];
	int packet_num = 0;
	int Len = 13;
	int total_id = 0;
	int max_value1 = (int)predict(max_estimator1, gamma_2, false);
	int flow_num;

	greeting(false);

	printf("dataset: %s\nsketch_number: %d(0: CM, 1: CU)\n", argv[1], sketch_number);
	printf("memory use: %dKB\n", memory_use);

	class_sketches *s[LOOP_NUM];
	CT *ct[LOOP_NUM];

	if (sketch_number == 0)
	{
		for (int m = 0; m < LOOP_NUM; ++m)
			s[m] = new CMSketch(memory_use * 1024 * 8 / (per_estimator_int * sketch_array), sketch_array);
	}
	else
	{
		for (int m = 0; m < LOOP_NUM; ++m)
			s[m] = new CUSketch(memory_use * 1024 * 8 / (per_estimator_int * sketch_array), sketch_array);
	}

	double left = 0, right = 2 * memory_use * 1024, mid = (left + right) / 2;
	while ((right - left) > 1e-5)
	{
		mid = (left + right) / 2;
		ct[0] = new CT(mid);
		if (ct[0]->get_memory() <= memory_use * 1024)
			left = mid;
		else
			right = mid;
		delete ct[0];
	}
	for (int i = 0; i < LOOP_NUM; ++i)
		ct[i] = new CT(mid);
	
	for (int m = 0; m < LOOP_NUM; ++m)
		s[m] = new CUSketch(memory_use * 1024 * 8 / (per_estimator_int * sketch_array), sketch_array);

	while (fread(insert[packet_num], Len, 1, fin))
	{
		for (int k = 0; k < Len; ++k)
			if (insert[packet_num][k] == '\0')
				insert[packet_num][k] = ' ';
		string s(insert[packet_num], Len);
		flows[s] += 1;
		packet_num++;
		if (packet_num % read_packet_print == 0)
			printf("packet-cnt=%d\n", packet_num);
		if (packet_num == read_packet_per_times)
			break;
	}
	flow_num = flows.size();
	printf("%d flows, %d packets\n", flow_num, packet_num);

	int max_freq = 0;
	unordered_map<string, int>::iterator it = flows.begin();

	for (int i = 0; i < flows.size(); i++, it++)
	{
		strcpy(query[i], it->first.c_str());
		//cout<<it->first<<endl;
		int temp2 = it->second;
		max_freq = max_freq > temp2 ? max_freq : temp2;
	}
	printf("max_freq = %d\n", max_freq);

	printf("\n*************************************\n\n");

	timespec time1, time2;
	long long resns;

	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int t = 0; t < LOOP_NUM; t++)
	{
		for (int i = 0; i < packet_num; i++)
			s[t]->dynamic_sac_insert(insert[i], 1, gamma_2, false);
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
	double throughput_sead = (double)1000.0 * LOOP_NUM * packet_num / resns;
	printf("throughput of SEAD on sketch (insert): %.6lf Mops\n", throughput_sead);


	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int t = 0; t < LOOP_NUM; t++)
	{
		for (int i = 0; i < packet_num; i++)
			ct[t]->Insert(insert[i]);
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
	double throughput_ct = (double)1000.0 * LOOP_NUM * packet_num / resns;
	printf("throughput of SEAD on CT (insert): %.6lf Mops\n", throughput_ct);

	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int t = 0; t < LOOP_NUM; t++)
	{
		SmallActiveCounter::r = 1;
		for (int i = 0; i < packet_num; i++)
			s[t]->SmallActiveCounter_insert(insert[i]);
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
	double throughput_sac = (double)1000.0 * LOOP_NUM * packet_num / resns;
	printf("throughput of SAC on sketch(insert): %.6lf Mops\n", throughput_sac);

	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int t = 0; t < LOOP_NUM; t++)
	{
		for (int i = 0; i < packet_num; i++)
			s[t]->icebuckets_insert(insert[i]);
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
	double throughput_ice = (double)1000.0 * LOOP_NUM * packet_num / resns;
	printf("throughput of ICEBucket on sketch (insert): %.6lf Mops\n", throughput_ice);

	printf("*************************************\n\n");

	LL res_tmp = 0;
	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int t = 0; t < LOOP_NUM; t++)
	{
		for (int i = 0; i < flow_num; i++)
			res_tmp = (LL)s[t]->dynamic_sac_query(query[i], gamma_2, false);
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
	throughput_sead = (double)1000.0 * LOOP_NUM * flow_num / resns;
	printf("throughput of SEAD on sketch (query): %.6lf Mops\n", throughput_sead);

	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int t = 0; t < LOOP_NUM; t++)
	{
		for (int i = 0; i < flow_num; i++)
			res_tmp = s[t]->SmallActiveCounter_query(query[i]);
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);

	throughput_sac = (double)1000.0 * LOOP_NUM * flow_num / resns;
	printf("throughput of SAC on sketch (query): %.6lf Mops\n", throughput_sac);


	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int t = 0; t < LOOP_NUM; t++)
	{
		for (int i = 0; i < flow_num; i++)
			res_tmp = ct[t]->Query(query[i],read_packet_per_times);
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);

	throughput_sac = (double)1000.0 * LOOP_NUM * flow_num / resns;
	printf("throughput of SAC on sketch (query): %.6lf Mops\n", throughput_sac);

	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int t = 0; t < LOOP_NUM; t++)
	{
		for (int i = 0; i < flow_num; i++)
			res_tmp = (LL)s[t]->icebuckets_query(query[i]);
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);

	throughput_ice = (double)1000.0 * LOOP_NUM * flow_num / resns;
	printf("throughput of ICEBucket on sketch (query): %.6lf Mops\n", throughput_ice);

	printf("*************************************\n\n");

	int c = 0;

	vector<pair<string, int>> flows_num_vec(flows.begin(), flows.end());
	sort(flows_num_vec.begin(), flows_num_vec.end(), comp);

	double aae_ct,aae_sead, aae_sac, aae_ice, are_ct,are_sead, are_sac, are_ice = 0;
	double aae_ct_m,aae_sead_m, aae_sac_m, aae_ice_m, are_ct_m,are_sead_m, are_sac_m, are_ice_m = 0;
	double aae_ct_e,aae_sead_e, aae_sac_e, aae_ice_e, are_ct_e,are_sead_e, are_sac_e, are_ice_e = 0;

	for (auto it = flows_num_vec.begin(); it != flows_num_vec.end(); ++it)
	{
		const char *str = (it->first).c_str();
		long long t1 = (LL)s[0]->dynamic_sac_query(str, gamma_2, false);
		long long t2 = s[0]->SmallActiveCounter_query(str);
		long long t3 = (LL)s[0]->icebuckets_query(str);
		long long t4 = (LL)ct[0]->Query(str,read_packet_per_times);

		if (c++ < 0.99 * flow_num)
		{
			aae_sead_m += (abs)(t1 - it->second);
			aae_sac_m += (abs)(t2 - it->second);
			aae_ice_m += (abs)(t3 - it->second);
			aae_ct_m+= (abs)(t4 - it->second);
			are_sead_m += (abs)(t1 - it->second) / (double)it->second;
			are_sac_m += (abs)(t2 - it->second) / (double)it->second;
			//printf("t2: %lld, second: %d, are_sac_m: %f\n",t2,it->second,are_sac_m);
			are_ice_m += (abs)(t3 - it->second) / (double)it->second;
			are_ct_m+= (abs)(t4 - it->second) / (double)it->second;
		}
		else
		{
			if (aae_ice_e < 0)
				printf("floating point error?\n Please retry\n");
			aae_sead_e += (abs)(t1 - it->second);
			//printf("t1: %lld, second: %d, aae_sead_e: %f\n", t1, it->second, aae_sead_e);
			aae_sac_e += (abs)(t2 - it->second);
			//printf("t2: %lld, second: %d, aae_sac_e: %f\n", t2, it->second, aae_sac_e);
			aae_ice_e += (abs)(t3 - it->second);
			aae_ct_e+= (abs)(t4 - it->second);
			are_sead_e += (abs)(t1 - it->second) / (double)it->second;
			are_sac_e += (abs)(t2 - it->second) / (double)it->second;
			are_ice_e += (abs)(t3 - it->second) / (double)it->second;
			are_ct_e+= (abs)(t4 - it->second) / (double)it->second;
		}

		aae_sead += (abs)(t1 - it->second);
		aae_sac += (abs)(t2 - it->second);
		aae_ice += (abs)(t3 - it->second);
		aae_ct+= (abs)(t4 - it->second);
		are_sead += (abs)(t1 - it->second) / (double)it->second;
		are_sac += (abs)(t2 - it->second) / (double)it->second;
		are_ice += (abs)(t3 - it->second) / (double)it->second;
		are_ct+= (abs)(t4 - it->second) / (double)it->second;
		//	fprintf(fout, "%d   %f   %f   %f\n", it->second, (t1 - it->second) / (double)it->second, (t2 - it->second) / (double)it->second, (t3 - it->second) / (double)it->second);
	}
	//printf("are_sac: %f\n",are_sac);
	aae_sead = aae_sead / flow_num;
	aae_sac = aae_sac / flow_num;
	aae_ice = aae_ice / flow_num;
	aae_ct = aae_ct / flow_num;
	are_sead = are_sead / flow_num;
	are_sac = are_sac / flow_num;
	are_ice = are_ice / flow_num;
	are_ct = are_ct / flow_num;


	aae_sead_m = aae_sead_m / flow_num / 0.99;
	aae_sac_m = aae_sac_m / flow_num / 0.99;
	aae_ice_m = aae_ice_m / flow_num / 0.99;
	aae_ct_m = aae_ct_m / flow_num / 0.99;
	are_sead_m = are_sead_m / flow_num / 0.99;
	are_sac_m = are_sac_m / flow_num / 0.99;
	are_ice_m = are_ice_m / flow_num / 0.99;
	are_ct_m = are_ct_m / flow_num / 0.99;

	//printf("are_sead:%f\n", are_sead_e);

	aae_sead_e = aae_sead_e / flow_num / 0.01;
	aae_sac_e = aae_sac_e / flow_num / 0.01;
	aae_ice_e = aae_ice_e / flow_num / 0.01;
	aae_ct_e = aae_ct_e / flow_num / 0.01;
	are_sead_e = are_sead_e / flow_num / 0.01;
	are_sac_e = are_sac_e / flow_num / 0.01;
	are_ice_e = are_ice_e / flow_num / 0.01;
	are_ct_e = are_ct_e / flow_num / 0.01;

	printf("aae_sead = %f\n", aae_sead);
	printf("aae_sead_m = %f\n", aae_sead_m);
	printf("aae_sead_e = %f\n\n", aae_sead_e);

	printf("aae_sac = %f\n", aae_sac);
	printf("aae_sac_m = %f\n", aae_sac_m);
	printf("aae_sac_e = %f\n\n", aae_sac_e);

	printf("aae_ice = %f\n", aae_ice);
	printf("aae_ice_m = %f\n", aae_ice_m);
	printf("aae_ice_e = %f\n\n", aae_ice_e);

	printf("aae_ct = %f\n", aae_ct);
	printf("aae_ct_m = %f\n", aae_ct_m);
	printf("aae_ct_e = %f\n\n", aae_ct_e);

	printf("*************************************\n\n");

	printf("are_sead = %f\n", are_sead);
	printf("are_sead_m = %f\n", are_sead_m);
	printf("are_sead_e = %f\n\n", are_sead_e);

	printf("are_sac = %f\n", are_sac);
	printf("are_sac_m = %f\n", are_sac_m);
	printf("are_sac_e = %f\n\n", are_sac_e);

	printf("are_ice = %f\n", are_ice);
	printf("are_ice_m = %f\n", are_ice_m);
	printf("are_ice_e = %f\n\n", are_ice_e);

	printf("are_ct = %f\n", are_ct);
	printf("are_ct_m = %f\n", are_ct_m);
	printf("are_ct_e = %f\n\n", are_ct_e);

	printf("Evaluation Ends!\n");
	printf("******************************************************************************\n\n");

	fclose(fin);

	return 0;
}
