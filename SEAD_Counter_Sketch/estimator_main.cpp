#include <iostream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cstring>
#include <unordered_map>
#include <map>
#include <fstream>
#include <limits>
#include "params.h"
#include "sac.h"
#include "ICE_bucket.h"
#include "SmallActiveCounter.h"
#include <algorithm>
#include <vector>

using namespace std;

unordered_map<string, int> flows;
unordered_map<string, int> flows_id;
map<int, int> fsd;

int flow_num;
double memory;
double remain_memory_overhead;

// some const for sead_counter
int sead_counter[prealloc_flow_num];
const int max_estimator1 = (1 << per_estimator_int) - 1;

SmallActiveCounter SAC[prealloc_flow_num];

char SmallActiveCounter::q = per_estimator_int;
char SmallActiveCounter::mode = per_estimator_mode;
char SmallActiveCounter::A = per_estimator_int - per_estimator_mode;
int SmallActiveCounter::r = 1;

double error[3][KIND_OF_COUNTERS];
double error_aae[3][KIND_OF_COUNTERS];

double average_err[3 * KIND_OF_COUNTERS];
double average_sig[3 * KIND_OF_COUNTERS];
double average_time[KIND_OF_COUNTERS];

bool comp(pair<string, int> &a, pair<string, int> &b)
{
	return a.second < b.second;
}

int main(int argc, char *argv[])
{
	char filename[35];
	if (argc != 2)
	{
		printf("usage: exe_filename read_filename");
		exit(1);
	}

	FILE *fin = fopen(argv[1], "rb");

	greeting(false);

	char buf[25];
	int packet_cnt = 0;
	int Len = 13;
	int total_id = 0;
	int max_value1 = (int)predict(max_estimator1, gamma_2, false);

	while (fread(buf, Len, 1, fin))
	{
		for (int k = 0; k < Len; ++k)
			if (buf[k] == '\0')
				buf[k] = ' ';
		string s(buf, Len);
		flows[s] += 1;
		if (flows_id[s] == 0 && total_id != 0)
			flows_id[s] = total_id++;
		else if (total_id == 0)
			total_id++;
		packet_cnt++;
		if (packet_cnt % read_packet_print == 0)
			printf("packet-cnt=%d\n", packet_cnt);
		if (packet_cnt == read_packet_per_times)
			break;
	}
	flow_num = flows.size();
	printf("%d flows, %d packets\n\n", flow_num, packet_cnt);
	memory = per_estimator_bit * flow_num;
	remain_memory_overhead = (per_estimator_bit - per_estimator_int) * flow_num;
	memset(sead_counter, 0, sizeof(int) * flow_num);
	memset(SAC, 0, sizeof(SmallActiveCounter) * flow_num);

	timespec time1, time2;
	long long resns;

	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int t = 0; t < LOOP_NUM; t++)
	{
		memset(sead_counter, 0, sizeof(int) * flow_num);
		for (unordered_map<string, int>::iterator it = flows.begin(); it != flows.end(); ++it)
		{
			int flow_id = flows_id[it->first];
			for (int i = 0; i < it->second; ++i)
			{
				adding(sead_counter[flow_id], 1, gamma_2, false);
			}
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
	double throughput_sead = (double)1000.0 * LOOP_NUM * packet_cnt / resns;
	printf("throughput of SEAD (insert): %.6lf Mops\n", throughput_sead);

	
	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int t = 0; t < LOOP_NUM; t++)
	{
		memset(SAC, 0, sizeof(SmallActiveCounter) * flow_num);
		SmallActiveCounter::r = 1;
		for (unordered_map<string, int>::iterator it = flows.begin(); it != flows.end(); ++it)
		{
			int flow_id = flows_id[it->first];
			for (int i = 0; i < it->second; ++i)
				UpdateCounter(SAC, flow_id, 1, flow_num);
		}
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
	double throughput_sac = (double)1000.0 * LOOP_NUM * packet_cnt / resns;
	printf("throughput of SAC (insert): %.6lf Mops\n", throughput_sac);

	ICEBuckets *icebuc;

	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int t = 0; t < LOOP_NUM; t++)
	{
		icebuc = new ICEBuckets(flow_num, 1 << (per_estimator_int), INT32_MAX, remain_memory_overhead);
		for (unordered_map<string, int>::iterator it = flows.begin(); it != flows.end(); ++it)
		{
			int flow_id = flows_id[it->first];
			for (int i = 0; i < it->second; ++i)
				icebuc->inc(flow_id/ icebuc->S, flow_id % icebuc->S);
		}
		if(t!=LOOP_NUM-1)
			delete icebuc;
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
	double throughput_ice = (double)1000.0 * LOOP_NUM * packet_cnt / resns;
	printf("throughput of ICEBucket (insert): %.6lf Mops\n", throughput_ice);

	printf("*************************************\n\n");

	LL res_tmp = 0;
	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int t = 0; t < LOOP_NUM; t++)
	{
		for (unordered_map<string, int>::iterator it = flows_id.begin(); it != flows_id.end(); ++it)
			res_tmp = (LL)predict(sead_counter[it->second], gamma_2, false);
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
	throughput_sead = (double)1000.0 * LOOP_NUM * flow_num / resns;
	printf("throughput of SEAD (query): %.6lf Mops\n", throughput_sead);

	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int t = 0; t < LOOP_NUM; t++)
	{
		for (unordered_map<string, int>::iterator it = flows_id.begin(); it != flows_id.end(); ++it)
			res_tmp = QueryCounter(SAC, it->second);
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);
	
	throughput_sac = (double)1000.0 * LOOP_NUM * flow_num / resns;
	printf("throughput of SAC (query): %.6lf Mops\n", throughput_sac);

	clock_gettime(CLOCK_MONOTONIC, &time1);
	for (int t = 0; t < LOOP_NUM; t++)
	{
		for (unordered_map<string, int>::iterator it = flows_id.begin(); it != flows_id.end(); ++it)
			res_tmp = (LL)icebuc->estimate(it->second / icebuc->S, it->second % icebuc->S);
	}
	clock_gettime(CLOCK_MONOTONIC, &time2);
	resns = (long long)(time2.tv_sec - time1.tv_sec) * 1000000000LL + (time2.tv_nsec - time1.tv_nsec);

	throughput_ice = (double)1000.0 * LOOP_NUM * flow_num / resns;
	printf("throughput of ICEBucket (query): %.6lf Mops\n", throughput_ice);

	printf("*************************************\n\n");

	int c = 0;
	

	vector<pair<string, int>> flows_num_vec(flows.begin(), flows.end());
	sort(flows_num_vec.begin(), flows_num_vec.end(), comp);

	double aae_sead, aae_sac, aae_ice, are_sead, are_sac, are_ice = 0;
	double aae_sead_m, aae_sac_m, aae_ice_m, are_sead_m, are_sac_m, are_ice_m = 0;
	double aae_sead_e, aae_sac_e, aae_ice_e, are_sead_e, are_sac_e, are_ice_e = 0;
	
	for (auto it = flows_num_vec.begin(); it != flows_num_vec.end(); ++it)
	{
		long long t1 = (LL)predict(sead_counter[flows_id[it->first]], gamma_2, false);
		long long t2 = QueryCounter(SAC, flows_id[it->first]);
		long long t3 = (LL)icebuc->estimate(flows_id[it->first] / icebuc->S, flows_id[it->first] % icebuc->S);

		if (c++ < 0.99 * flow_num)
		{
			aae_sead_m += (abs)(t1 - it->second);
			aae_sac_m += (abs)(t2 - it->second);
			aae_ice_m += (abs)(t3 - it->second);
			are_sead_m += (abs)(t1 - it->second) / (double)it->second;
			are_sac_m += (abs)(t2 - it->second) / (double)it->second;
			are_ice_m += (abs)(t3 - it->second) / (double)it->second;
		}
		else
		{
			aae_sead_e += (abs)(t1 - it->second);
			aae_sac_e += (abs)(t2 - it->second);
			aae_ice_e += (abs)(t3 - it->second);
			are_sead_e += (abs)(t1 - it->second) / (double)it->second;
			are_sac_e += (abs)(t2 - it->second) / (double)it->second;
			are_ice_e += (abs)(t3 - it->second) / (double)it->second;
		}

		aae_sead += (abs)(t1 - it->second);
		aae_sac += (abs)(t2 - it->second);
		aae_ice += (abs)(t3 - it->second);
		are_sead += (abs)(t1 - it->second) / (double)it->second;
		are_sac += (abs)(t2 - it->second) / (double)it->second;
		are_ice += (abs)(t3 - it->second) / (double)it->second;
		//	fprintf(fout, "%d   %f   %f   %f\n", it->second, (t1 - it->second) / (double)it->second, (t2 - it->second) / (double)it->second, (t3 - it->second) / (double)it->second);
	}
	//printf("are_sac: %f\n",are_sac);
	aae_sead = aae_sead / flow_num;
	aae_sac = aae_sac / flow_num;
	aae_ice = aae_ice / flow_num;
	are_sead = are_sead / flow_num;
	are_sac = are_sac / flow_num;
	are_ice = are_ice / flow_num;

	aae_sead_m = aae_sead_m / flow_num / 0.99;
	aae_sac_m = aae_sac_m / flow_num / 0.99;
	aae_ice_m = aae_ice_m / flow_num / 0.99;
	are_sead_m = are_sead_m / flow_num / 0.99;
	are_sac_m = are_sac_m / flow_num / 0.99;
	are_ice_m = are_ice_m / flow_num / 0.99;

	aae_sead_e = aae_sead_e / flow_num / 0.01;
	aae_sac_e = aae_sac_e / flow_num / 0.01;
	aae_ice_e = aae_ice_e / flow_num / 0.01;
	are_sead_e = are_sead_e / flow_num / 0.01;
	are_sac_e = are_sac_e / flow_num / 0.01;
	are_ice_e = are_ice_e / flow_num / 0.01;

	printf("aae_sead = %lf\n", aae_sead);
	printf("aae_sead_m = %lf\n", aae_sead_m);
	printf("aae_sead_e = %lf\n\n", aae_sead_e);

	printf("aae_sac = %lf\n", aae_sac);
	printf("aae_sac_m = %lf\n", aae_sac_m);
	printf("aae_sac_e = %lf\n\n", aae_sac_e);

	printf("aae_ice = %lf\n", aae_ice);
	printf("aae_ice_m = %lf\n", aae_ice_m);
	printf("aae_ice_e = %lf\n\n", aae_ice_e);

	printf("*************************************\n\n");

	printf("are_sead = %lf\n", are_sead);
	printf("are_sead_m = %lf\n", are_sead_m);
	printf("are_sead_e = %lf\n\n", are_sead_e);

	printf("are_sac = %lf\n", are_sac);
	printf("are_sac_m = %lf\n", are_sac_m);
	printf("are_sac_e = %lf\n\n", are_sac_e);

	printf("are_ice = %lf\n", are_ice);
	printf("are_ice_m = %lf\n", are_ice_m);
	printf("are_ice_e = %lf\n\n", are_ice_e);

	printf("Evaluation Ends!\n");
	printf("******************************************************************************\n\n");

	fclose(fin);

	

	return 0;
}
