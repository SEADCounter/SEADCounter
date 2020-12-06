#pragma once
#include <algorithm>
#include <cstring>
#include <string.h>
#include <vector>
#include "sketch.h"
#include "bobhash.h"
#include "params.h"
#include "sac.h"
#include <iostream>



// including the operations of C: Insert, Delete and Query; including the operations of dynamic version of C: dynamic_sac_insert, dynamic_sac_query; 
// including the operations of static version of C: static_sac_insert, static_sac_query. check function is used to debug and output str's corresponding
// hash table content.


//icebucket becomes more complicated when supporting both decrementing counters and negative value. The corresponding author doesn't implement it in their python code, so we don't implement it to support C sketch and compare with ours. 

class CSketch:public class_sketches{	
public:
    BOBHash * bobhash[20 * 2];
    int index[20];
    int *counter[20];
    sead_c *sign_counter[20];
    
    
    int w, d;
    int base_num=1;
    
	
    int counter_index_size;
    uint64_t hash_value;

    //w is the size of the hash area, d is the number of hash functions in the Mediean trick
    CSketch(int _w, int _d){
	counter_index_size = 20;
        w = _w/base_num;              //divide each of the d hash aera into yet base_num sub sections
        d = _d/base_num;
		
        for(int i = 0; i < _d; i++){
            counter[i] = new int[_w];
            memset(counter[i], 0, sizeof(int) * _w);

            sign_counter[i] = new sead_c[_w];
            memset(sign_counter[i], 0, sizeof(sead_c) * _w);
	}
	
	
        for(int i = 0; i < _d * 2; i++)
	    bobhash[i] = new BOBHash(i + 1000);
		
    }
	
    //The insert function for int counters
    void Insert(const char * str,int c){
	int g = 0;
        for(int i = 0; i < d; i++){
	    index[i] = (bobhash[i]->run(str, strlen(str))) % w;
	    g = (bobhash[i + d]->run(str, strlen(str))) % 2;

	    if(g == 0){
                if(counter[i][index[i]]+c < MAX_INT_CNT)
                    counter[i][index[i]] += c;
                else cout<<"int overflow, counter value = "<< counter[i][index[i]]<<endl;
	    }
	    else{
                if(counter[i][index[i]]-c > MIN_INT_CNT)
                    counter[i][index[i]] -= c;
                else cout<<"int underflow, counter value = "<< counter[i][index[i]]<<endl;
	    }
	}
    }
    
    //the query function for int counters
    int Query(const char *str)
	{
		int temp;
		int res[20];
		int g;
		for (int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, strlen(str))) % w;
			temp = counter[i][index[i]];
			g = (bobhash[i + d]->run(str, strlen(str))) % 2;
			res[i] = (g == 0 ? temp : -temp);
		}
		sort(res, res + d);
		int r;
		if (d % 2 == 0)
		{
			r = (res[d / 2] + res[d / 2 - 1]) / 2;
		}
		else
		{
			r = res[d / 2];
		}
		return r;
	}
	//the delete function for int counters
	void Delete(const char * str)
	{
		int g = 0;
		for (int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, strlen(str))) % w;
			g = (bobhash[i + d]->run(str, strlen(str))) % 2;

			if (g == 1)
			{
				if (counter[i][index[i]] != MAX_INT_CNT)
				{
					counter[i][index[i]]++;
				}
			}
			else
			{
				if (counter[i][index[i]] != MIN_INT_CNT)
				{
					counter[i][index[i]]--;
				}
			}
		}
	}
	//the insert functions for dynamic sign bits version SAC
    void dynamic_sac_insert(const char *str,int c,LL *gamma,bool sp_negative=true){

        int g = 0;
        for(int i = 0; i < d; i++)
        {
            index[i] = (bobhash[i]->run(str, strlen(str))) % w;
            g = (bobhash[i + d]->run(str, strlen(str))) % 2;

            if(g == 0)
            {
                if(sign_counter[i][index[i]] != MAX_CNT_CO)
                    adding(sign_counter[i][index[i]],c,gamma);
            }
            else
            {
                if(sign_counter[i][index[i]] != MIN_CNT_CO)
                    subtracting(sign_counter[i][index[i]],c,gamma);
            }

        }
    }
    //the query function for dynamic sign bits SAC
	int dynamic_sac_query(const char *str, LL *gamma,bool sp_negative=true) {
		int temp;
		int res[20];
		int g;
		for (int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, strlen(str))) % w;
			temp = predict(sign_counter[i][index[i]], gamma);
			g = (bobhash[i + d]->run(str, strlen(str))) % 2;
			res[i] = (g == 0 ? temp : -temp);
		}
		sort(res, res + d);
		int r;
		if (d % 2 == 0)
		{
			r = (res[d / 2] + res[d / 2 - 1]) / 2;
		}
		else
		{
			r = res[d / 2];
		}
		return r;
	}
	//the insert function for static sign bits version SAC
	void static_sac_insert(const char *str, int l_sign, LL *gamma,bool sp_negative=true) {

		int g = 0;
		for (int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, strlen(str))) % w;
			g = (bobhash[i + d]->run(str, strlen(str))) % 2;

			if (g == 0)
			{
				if (sign_counter[i][index[i]] != MAX_CNT_CO)
					add_one(sign_counter[i][index[i]], l_sign, gamma);
			}
			else
			{
				if (sign_counter[i][index[i]] != MIN_CNT_CO)
					minus_one(sign_counter[i][index[i]], l_sign, gamma);
			}
		}
	}
	//query function for static version SAC
	int static_sac_query(const char *str,int l_sign, LL *gamma,bool sp_negative=true) {
		int temp;
		int res[20];
		int g;
		for (int i = 0; i < d; i++)
		{
			index[i] = (bobhash[i]->run(str, strlen(str))) % w;
			temp = predict(sign_counter[i][index[i]],l_sign, gamma);
			g = (bobhash[i + d]->run(str, strlen(str))) % 2;
			res[i] = (g == 0 ? temp : -temp);
		}
		sort(res, res + d);
		int r;
		if (d % 2 == 0)
		{
			r = (res[d / 2] + res[d / 2 - 1]) / 2;
		}
		else
		{
			r = res[d / 2];
		}
		return r;
	}
//function used to inspect the value of "d mapped counters"
    void check(const char *str,LL *gamma){     
        int temp;
		int result[22];
        cout<<"[[";
        //check str's correspond hash table content
        for(int i=0;i<d;++i){
            int pos=(bobhash[i]->run(str, strlen(str))) % w;
            result[i]=abs(predict(sign_counter[i][pos],4,gamma));
			
        }
		sort(result, result + d);
		for (auto x : result)cout << x << " ";
        cout<<"]]"<<endl;
    }

   
	~CSketch()
	{
		for(int i = 0; i < d; i++)	
		{
			delete []counter[i];
			delete [] sign_counter[i];
		}
		for(int i = 0; i < d * 2; i++)
		{
			delete bobhash[i];
		}
	}
};
 // CSKETCH_H
