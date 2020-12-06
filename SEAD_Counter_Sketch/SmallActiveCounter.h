#pragma once
#include<iostream>
#include<math.h>
#include<algorithm>
#include<stdio.h>
#include<stdlib.h>
#include "params.h"

using namespace std;


class SmallActiveCounter{
public:
    static char q,mode,A;
    static int r;
    int q_val,mode_val,A_val;
    SmallActiveCounter(){
	this->A_val=0;
	this->mode_val=0;
    }
};


void RenormalizeCounters(SmallActiveCounter* SAC,int size){
    for(int i=0;i<size;++i){
    	int old_mode = SAC[i].mode_val;
    	SAC[i].mode_val = ceil(SAC[i].mode_val*(double)SmallActiveCounter::r/(SmallActiveCounter::r+1));
    	int r1=SAC[i].mode_val*(SmallActiveCounter::r+1)-old_mode*SmallActiveCounter::r;
    	double add=SAC[i].A_val/(double)(1<<r1);
    	SAC[i].A_val=(int)add;
    	add-=(int)add;
    	double rand_value=rand()/(double)RAND_MAX;
    	if(rand_value<add)SAC[i].A_val++;
    }
    return;
}

void UpdateCounter(SmallActiveCounter* SAC,int i,int inc,int size){
    SAC[i].A_val+=inc/(1<<(SmallActiveCounter::r*SAC[i].mode_val));
    double add = (double)inc/(1<<(SmallActiveCounter::r*SAC[i].mode_val));
    add -= (int)add;
    double rand_value= rand()/(double)RAND_MAX;
    if(rand_value<add)SAC[i].A_val+=1;
    if(SAC[i].A_val>(1<<SmallActiveCounter::A)){
    	SAC[i].mode_val+=1;
    	add=(double)SAC[i].A_val/(1<<(SmallActiveCounter::r));
    	SAC[i].A_val=(int)add;
    	add-=SAC[i].A_val;
    	rand_value=rand()/(double)RAND_MAX;
    	if(rand_value<add)SAC[i].A_val+=1;
    }
    if(SAC[i].mode_val==(1<<SmallActiveCounter::mode)){
    	RenormalizeCounters(SAC,size);
    	SmallActiveCounter::r++;
    }
    return;
}



LL QueryCounter(SmallActiveCounter* SAC,int counter_num){
    return SAC[counter_num].A_val*(1<<(SAC[counter_num].mode_val*SmallActiveCounter::r));
}
