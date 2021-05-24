//
//  VICBF-SEAD.h
//  CBF
//
//  Created by xy on 2019/2/22.
//  Copyright © 2019 xy. All rights reserved.
//

#ifndef VI_CBF_SEAD_h
#define VI_CBF_SEAD_h


#include <algorithm>
#include <cstring>
#include "sketch.h"
#include <string.h>
#include "bobhash.h"
#include "sead.h"
#include <iostream>

using namespace std;

#define L 4



class VI_CBF:public class_sketches{
private:
    BOBHash *bobhash[40];
    int index[20];
    sead_c *counter[20];
    int *int_counter[20];
    int VI_Counter[L];
    int w,d;
    int MAX_CNT=2147483647;  
    int counter_index_size;
    uint64_t hash_value;
public:
    VI_CBF(int _w,int _d){ //w is the size of the hash area, d is the number of hash functions in the Mediean trick
//VI-CBF的d相比于CBF要为原来�?4/7. 同理针对于SEAD_VI-CBF的版本�?
        counter_index_size = 20;
        w = _w;
        d = _d;
        
	for (int i = 0; i < L; ++i)
	    VI_Counter[i] = L + i;

        for(int i = 0; i < d; i++)
        {
            counter[i] = new sead_c[w];
            memset(counter[i], 0, sizeof(sead_c) * w);
            bobhash[i] = new BOBHash(i + 1000);
	        bobhash[i+d] = new BOBHash(i + d +  1000);
            
            int_counter[i] = new int[w];
            memset(int_counter[i], 0, sizeof(int) * w);
        }
    }
    //insert function for int counters
    void Insert(const char *str, int c) {
        for (int i = 0; i < d; i++) {
            index[i] = (bobhash[i]->run(str, strlen(str))) % w;
            if (int_counter[i][index[i]] + c * VI_Counter[(bobhash[i+d]->run(str, strlen(str)))%L]<= MAX_CNT) {
                int_counter[i][index[i]] += c * VI_Counter[(bobhash[i+d]->run(str, strlen(str)))%L];
            }
        }
    }
    //query function for int counters
    int Query(const char *str) {
        int min_value = MAX_CNT;
        int temp;
        
        for (int i = 0; i < d; i++)
        {
            index[i] = (bobhash[i]->run(str, strlen(str))) % w;
            temp = int_counter[i][index[i]];
            bool condition1 = (temp <= 2*L-1) && (temp != VI_Counter[(bobhash[i+d] -> run(str,strlen(str)))%L]);
            bool condition2 = (temp <=3*L-1) && (temp >= 2*L && temp-VI_Counter[(bobhash[i+d] -> run(str,strlen(str)))%L] < L);
            if (temp == 0 || condition1 || condition2 ) return 0;
           // min_value = temp < min_value ? temp : min_value;
        }
	    return 1;
      //  return min_value;
    }
    //  上面这个查询的地方有点？？可能看这个地方查询的ARE与AAE大不大�?
    //delete function for int counters
    void Delete(const char *str, int c) {
        for (int i = 0; i < d; i++) {
            index[i] = (bobhash[i]->run(str, strlen(str))) % w;
            int_counter[i][index[i]] -= c * VI_Counter[(bobhash[i+d]->run(str, strlen(str)))%L];
        }
    }
    //the insert function for dynamic sign bits SEAD, the function adds c to "d mapped counters"
    void dynamic_sead_insert(const char *str,int c,LL *gamma){
        for(int i = 0; i < d; i++){
            index[i] = (bobhash[i]->run(str, strlen(str))) % w;
            adding(counter[i][index[i]],c* VI_Counter[(bobhash[i+d]->run(str, strlen(str)))%L],gamma);
        }
    }
    //the query function for dynamic sign bits SEAD
    // 在用sead的insert的时候，adding里的long_maxi要改成long_maxi_VI
    int dynamic_sead_query(const char *str, LL *gamma)
    {
        sead_c min_value = MAX_CNT_CO;
        sead_c temp;
        
        for (int i = 0; i < d; i++)
        {
            index[i] = (bobhash[i]->run(str, strlen(str))) % w;
            temp = predict(counter[i][index[i]],gamma);
	        bool condition1 = (temp <= 2*L-1) && (temp != VI_Counter[(bobhash[i+d] -> run(str,strlen(str)))%L]);
            bool condition2 = (temp <=3*L-1) && (temp >= 2*L && temp-VI_Counter[(bobhash[i+d] -> run(str,strlen(str)))%L] < L);
            if (temp == 0 || condition1 || condition2 ) return 0;
            //min_value = min(min_value, temp);
        }
        // return predict(min_value, gamma);
        return 1;
        
    }
    //the insert function for fixed sign bits SEAD, the function adds c to "d mapped counters"
    void static_sead_insert(const char *str,int l_sign, LL *gamma) {
        for (int i = 0; i < d; i++) {
            index[i] = (bobhash[i]->run(str, strlen(str))) % w;
            int m = VI_Counter[(bobhash[i+d]->run(str, strlen(str))) %L];
            for(int j=0;j<m;j++)
                add_one(counter[i][index[i]],l_sign, gamma);
        }
    }
    //the query function for fixed sign bits SEAD
    int static_sead_query(const char *str,int l_sign, LL *gamma)
    {
        sead_c min_value = MAX_CNT_CO;
        sead_c temp;
        
        for (int i = 0; i < d; i++)
        {
            index[i] = (bobhash[i]->run(str, strlen(str))) % w;
            temp = predict(counter[i][index[i]],l_sign,gamma);
            bool condition1 = (temp <= 2*L-1) && (temp != VI_Counter[(bobhash[i+d] -> run(str,strlen(str)))%L]);
            bool condition2 = (temp <=3*L-1) && (temp >= 2*L && temp-VI_Counter[(bobhash[i+d] -> run(str,strlen(str)))%L] < L);
            if (temp == 0 || condition1 || condition2 ) return 0;
            //min_value = min(min_value, temp);
        }
        //return predict(min_value,l_sign, gamma);
            return 1;
        
    }
    //the debug function used to print values in "d mapped counters" correspond with str
    void check(const char *str, LL *gamma) {
        int temp;
        int result[22];
        cout << "[[";
        //check str's correspond hash table content
        for (int i = 0; i<d; ++i) {
            int pos = (bobhash[i]->run(str, strlen(str))) % w;
            result[i] = abs(predict(counter[i][pos], gamma));
            
        }
        //sort(result, result + d);
        for (auto x : result)cout << x << " ";
        cout << "]]" << endl;
    }
};

#endif /* CBF_SEAD_h */
