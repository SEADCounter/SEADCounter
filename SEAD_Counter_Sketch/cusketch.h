#pragma once
#include "sac.h"
#include <algorithm>
#include <cstring>
#include <string.h>
#include "bobhash.h"
#include "params.h"
#include "sketch.h"
#include <iostream>
#include "ICE_bucket.h"
#include "SmallActiveCounter.h"



using namespace std;

// including the operations of CU: Insert, Delete and Query; including the operations of dynamic version of CU: dynamic_sac_insert, dynamic_sac_query; 
// including the operations of static version of CU: static_sac_insert, static_sac_query. check function is used to debug and output str's corresponding
// hash table content.
class CUSketch:public class_sketches
{
private:
    BOBHash * bobhash[20];
    int index[20];
    sead_c *sac_counter[20];
    int *counter[20];
    ICEBuckets *icebuc;
    SmallActiveCounter *SAC;
    int w,w1, d;
   
	
    int counter_index_size;
    uint64_t hash_value;

public:
    CUSketch(int _w, int _d)
    {
        counter_index_size = 20;
        w = _w;
        d = _d;

        for(int i = 0; i < d; i++)
        {
            counter[i] = new int[w];
            memset(counter[i], 0, sizeof(int) * w);
            
            sac_counter[i] = new sead_c[w];
            memset(sac_counter[i], 0, sizeof(sead_c) * w);
            bobhash[i] = new BOBHash(i + 1000);
        }

        w1=w*(double)per_estimator_int/per_estimator_bit;
        icebuc=new ICEBuckets(w1*d,1<<per_estimator_int,INT32_MAX,w1*d*(per_estimator_bit-per_estimator_int));
	SAC=new SmallActiveCounter[w*d];
	memset(SAC,0,sizeof(SmallActiveCounter)*w*d); 
        
    }
    //insert function for int counters
	void Insert(const char *str, int c) {
		int min_value = MAX_INT_CNT;
		int temp;
		for (int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, strlen(str))) % w;
			temp = counter[i][index[i]];
			min_value = temp < min_value ? temp : min_value;
		}
		if (min_value + c >= MAX_INT_CNT)
			for (int i = 0; i < d; i++)
			{
				counter[i][index[i]] = MAX_INT_CNT;
				return;
			}
		for (int i = 0; i < d; i++)
		{
			if (counter[i][index[i]] == min_value) {
				counter[i][index[i]] += c;
			}
			else if (counter[i][index[i]] < min_value + c) {
				counter[i][index[i]] = min_value + c;
			}
		}
	}
	//query function for counters of int data type
	int Query(const char *str)
	{
		int min_value = MAX_INT_CNT;
		int temp;
		for (int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, strlen(str))) % w;
			temp = counter[i][index[i]];
			min_value = temp < min_value ? temp : min_value;
		}
		return min_value;
	}
	//insert function for dynamic version SAC
	void dynamic_sac_insert(const char *str, int c, LL *gamma,bool sp_negative=true) { 
		int max_possible_value=sp_negative?MAX_CNT_CO:MAX_CNT_CO_P;
		int min_value = max_possible_value;
		int temp;
		int index_value;
		for (int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, strlen(str))) % w;
			temp = sac_counter[i][index[i]];
			min_value = temp < min_value ? temp : min_value;
		}
		for (int i = 0; i < d; i++)
		{
			if (sac_counter[i][index[i]]== min_value) {
				adding(sac_counter[i][index[i]], c, gamma,sp_negative);
				index_value = sac_counter[i][index[i]];
			}
		}
		for (int i = 0; i<d; ++i) {
			if (predict(sac_counter[i][index[i]], gamma,sp_negative) < predict(min_value,gamma,sp_negative) + c) {
				sac_counter[i][index[i]] = index_value;
			}
		}
	}
	//query function for dynamic version SAC
	int dynamic_sac_query(const char *str, LL *gamma,bool sp_negative=true) {
		int min_value = sp_negative?MAX_CNT_CO:MAX_CNT_CO_P;
		int temp;
		int ind = 0;
		for (int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, strlen(str))) % w;
			temp = sac_counter[i][index[i]];
			if (temp < min_value) {
				min_value = temp;
			}
		}
		return predict(min_value, gamma,sp_negative);
	}
	//insert function for static version SAC
	void static_sac_insert(const char *str, int l_sign, LL *gamma,bool sp_negative=true) { 
		int max_possible_value=sp_negative?MAX_CNT_CO:MAX_CNT_CO_P;
		int min_value = max_possible_value;
		int temp;
		for (int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, strlen(str))) % w;
			temp = sac_counter[i][index[i]];
			min_value = temp < min_value ? temp : min_value;
		}
		for (int i = 0; i < d; i++)
		{
			if (sac_counter[i][index[i]] == min_value) {
				add_one(sac_counter[i][index[i]], l_sign, gamma,sp_negative);
			}
		}
	}

	//query function for static version of SAC
	int static_sac_query(const char *str,int l_sign, LL *gamma,bool sp_negative=true) {
		int max_possible_value=sp_negative?MAX_CNT_CO:MAX_CNT_CO_P;
		int min_value = max_possible_value;
		int temp;
		for (int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, strlen(str))) % w;
			temp = sac_counter[i][index[i]];
			if (temp < min_value) {
				min_value = temp;
			}
		}
		return predict(min_value,l_sign, gamma,sp_negative);
	}
//debug function
	void check(const char *str) {
		int temp;
		int result[22];
		cout << "[[";
		//check str's correspond hash table content
		for (int i = 0; i<d; ++i) {
			int pos = (bobhash[i]->run(str, strlen(str))) % w;
			result[i] = counter[i][pos];

		}
		//sort(result, result + d);
		for (auto x : result)cout << x << " ";
		cout << "]]" << endl;
	}
	
	
	void icebuckets_insert(const char* str) {
		int min_value = MAX_INT_CNT;
		int temp;
		for (int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, strlen(str))) % w1;
			int buc_num=(i*w1+index[i])/icebuc->S;
			int buc_sym=(i*w1+index[i])%icebuc->S;
			temp = icebuc->estimate(buc_num,buc_sym);
			min_value = temp < min_value ? temp : min_value;
		}
		for (int i = 0; i < d; i++)
		{
			int buc_num=(i*w1+index[i])/icebuc->S;
			int buc_sym=(i*w1+index[i])%icebuc->S;
			temp = icebuc->estimate(buc_num,buc_sym);
			if (temp == min_value) {
				icebuc->inc(buc_num,buc_sym);
			}
		}	
	}
	//query function for icebuckets
	int icebuckets_query(const char* str) {
		int min_value = MAX_INT_CNT;
		int temp;

		for (int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, strlen(str))) % w1;
			int buc_num=(i*w1+index[i])/icebuc->S;
			int buc_sym=(i*w1+index[i])%icebuc->S;
			temp = icebuc->estimate(buc_num,buc_sym);
			min_value = temp < min_value ? temp : min_value;
		}
		return min_value;
	}
	
	// function for icebuckets
	void SmallActiveCounter_insert(const char* str) {
		int min_value = MAX_INT_CNT;
		int temp;
		for (int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, strlen(str))) % w;
			temp = QueryCounter(SAC,i*w+index[i]);
			min_value = temp < min_value ? temp : min_value;
		}
		for (int i = 0; i < d; i++)
		{
			temp = QueryCounter(SAC,i*w+index[i]);
			if (temp == min_value) {
				UpdateCounter(SAC,i*w+index[i],1,w*d);
			}
		}	
	}
	//query function for icebuckets
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

    ~CUSketch()
    {
        for(int i = 0; i < d; i++)
        {
            delete[] counter[i];
            delete[] sac_counter[i];
            delete bobhash[i];
        }
        delete icebuc;
	delete[] SAC;
    }
};
 // CUSKETCH_H
