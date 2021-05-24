#pragma once 
#include <algorithm>
#include <cstring>
#include "sketch.h"
#include <string.h>
#include "bobhash.h"
#include "sead.h"
#include <iostream>
#include "params.h"
#include "ICE_bucket.h"
#include "SmallActiveCounter.h"


using namespace std;
class CMSketch :public class_sketches {
private:
	BOBHash* bobhash[20];
	int index[20];
	sead_c* counter[20];
	int* int_counter[20];
	ICEBuckets *icebuc;
	SmallActiveCounter *SAC;
	int w,w1, d; 
	
	int counter_index_size;
	uint64_t hash_value;
public:
	CMSketch(int _w, int _d) { //w is the size of the hash area, d is the number of hash functions in the Mediean trick
		counter_index_size = 20;
		w = _w;
		d = _d;

		for (int i = 0; i < d; i++)
		{
			counter[i] = new sead_c[w];
			memset(counter[i], 0, sizeof(sead_c) * w);
			bobhash[i] = new BOBHash(i + 1000);

			int_counter[i] = new int[w];
			memset(int_counter[i], 0, sizeof(int) * w);
		}
		w1=w*(double)per_estimator_int/per_estimator_bit;
		icebuc=new ICEBuckets(w1*d,1<<per_estimator_int,INT32_MAX,w1*d*(per_estimator_bit-per_estimator_int));
		SAC=new SmallActiveCounter[w*d];
		memset(SAC,0,sizeof(SmallActiveCounter)*w*d);
	}
	//insert function for int counters
	void Insert(const char* str, int c) {
		for (int i = 0; i < d; i++) {
			index[i] = (bobhash[i]->run(str, strlen(str))) % w;
			if (int_counter[i][index[i]] + c <= MAX_INT_CNT) {
				int_counter[i][index[i]] += c;
			}
		}
	}
	//query function for int counters
	int Query(const char* str) {
		int min_value = MAX_INT_CNT;
		int temp;

		for (int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, strlen(str))) % w;
			temp = int_counter[i][index[i]];
			min_value = temp < min_value ? temp : min_value;
		}
		return min_value;
	}

	//the insert function for dynamic sign bits SEAD, the function adds c to "d mapped counters"
	void dynamic_sead_insert(const char* str, int c, LL* gamma,bool sp_negative=true) {
		for (int i = 0; i < d; i++) {
			index[i] = (bobhash[i]->run(str, strlen(str))) % w;
			adding(counter[i][index[i]], c, gamma,sp_negative);
		}
	}
	//the query function for dynamic sign bits SEAD
	int dynamic_sead_query(const char* str, LL* gamma,bool sp_negative=true)
	{
		sead_c min_value = sp_negative?MAX_CNT_CO:MAX_CNT_CO_P;
		sead_c temp;

		for (int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, strlen(str))) % w;
			temp = counter[i][index[i]];
			min_value = min(min_value, temp);
		}
		//printf("predict: %lld \n",predict(min_value,gamma));
		return predict(min_value, gamma,sp_negative);

	}
	//the insert function for fixed sign bits SEAD, the function adds c to "d mapped counters"
	void static_sead_insert(const char* str, int l_sign, LL* gamma,bool sp_negative=true) {
		for (int i = 0; i < d; i++) {
			index[i] = (bobhash[i]->run(str, strlen(str))) % w;
			add_one(counter[i][index[i]], l_sign, gamma,sp_negative );
		}
	}
	//the query function for fixed sign bits SEAD
	int static_sead_query(const char* str, int l_sign, LL* gamma,bool sp_negative=true)
	{
		sead_c min_value = sp_negative?MAX_CNT_CO:MAX_CNT_CO_P;
		sead_c temp;

		for (int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, strlen(str))) % w;
			temp = counter[i][index[i]];
			min_value = min(min_value, temp);
		}
		return predict(min_value, l_sign, gamma,sp_negative);

	}
	//the debug function used to print values in "d mapped counters" correspond with str
	void check(const char* str, int l_sign, LL* gamma) {
		int temp;
		int result[22];
		cout << "[[";
		//check str's correspond hash table content
		for (int i = 0; i < d; ++i) {
			int pos = (bobhash[i]->run(str, strlen(str))) % w;
			
			result[i] = abs(predict(counter[i][pos], gamma,false));
			int buc_num=(i*w1+index[i])/icebuc->S;
			int buc_sym=(i*w1+index[i])%icebuc->S;
			//int est=icebuc->estimate(buc_num,buc_sym);
			//cout<< est<<endl;
			//bprint(sead_c(est));
		}
		//sort(result, result + d);
		for (int i = 0; i < d; ++i) {
			cout << result[i] << " ";
			bprint(sead_c(result[i]));
		}
		cout << "]]" << endl;
	}
	
	// function for icebuckets
	void icebuckets_insert(const char* str) {
		for (int i = 0; i < d; i++) {
			index[i] = (bobhash[i]->run(str, strlen(str))) % w1;
			//printf("index[i]: %d   w1: %d\n",index[i],w1);
			int buc_num=(i*w1+index[i])/icebuc->S;
			//printf("buc_num %d  \n",buc_num);
			
			int buc_sym=(i*w1+index[i])%icebuc->S;
			icebuc->inc(buc_num,buc_sym);
		}
	}
	//query function for icebuckets
	int icebuckets_query(const char* str) {
		int min_value = MAX_INT_CNT;
		int temp;

		for (int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, strlen(str))) % w1;
			//printf("index[i]: %d   w1: %d\n",index[i],w1);
			int buc_num=(i*w1+index[i])/icebuc->S;
			int buc_sym=(i*w1+index[i])%icebuc->S;
			temp = icebuc->estimate(buc_num,buc_sym);
			min_value = temp < min_value ? temp : min_value;
		}
		return min_value;
	}
	
	// function for SmallActiveCounter
	void SmallActiveCounter_insert(const char* str) {
		for (int i = 0; i < d; i++) {
			index[i] = (bobhash[i]->run(str, strlen(str))) % w;
			UpdateCounter(SAC,i*w+index[i],1,w*d);
		}
	}
	//query function for SmallActiveCounter
	int SmallActiveCounter_query(const char* str) {
		int min_value = MAX_INT_CNT;
		int temp;

		for (int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, strlen(str))) % w;
			temp = QueryCounter(SAC,i*w+index[i]);
			min_value = temp < min_value ? temp : min_value;
		}
		return min_value;
	}
	
	~CMSketch() {
		for (int i = 0; i < d; ++i)
		{
			delete[] counter[i];
			delete[] int_counter[i];
			delete bobhash[i];
		}
		delete icebuc;
		delete[] SAC;
	}
};

// CMSKETCH_H
