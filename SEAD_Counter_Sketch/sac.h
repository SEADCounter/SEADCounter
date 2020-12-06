#pragma once

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/timeb.h>
#include <iostream>
#include "params.h"
using namespace std;
//#define  COUNTER_SIZE  16


// maybe only use some bit of it, but to ensure it can be used to experiment on more bits per counter,
// we define it as 32-bit int instead of 16-bit short.
   

sead_c edge[COUNTER_SIZE+1];

LL max_predict_dyn,max_predict_sta;

LL predict(sead_c, int, LL*,bool);

LL predict(sead_c, LL*,bool);

//should be called when using sead
// if gamma changes, edge changes. edge is only used and necessary in adding c, but not 1.
void greeting(bool sp_negative=true,int l_sign=3) {
	if(sp_negative)
		for(int i=1;i<COUNTER_SIZE;++i)
			edge[i]=(1<<(COUNTER_SIZE-1))-(1<<(COUNTER_SIZE-i-1));
	else
		for(int i=1;i<=COUNTER_SIZE;++i)
			edge[i]=(1<<COUNTER_SIZE)-(1<<(COUNTER_SIZE-i));
	sead_c MAX_CNT_C = sp_negative?MAX_CNT_CO:MAX_CNT_CO_P;
	max_predict_dyn=predict(MAX_CNT_C,gamma_2,sp_negative);
	max_predict_sta=predict(MAX_CNT_C,l_sign,gamma_2,sp_negative);
	printf("***************\n");
	printf("**This is SEAD**\n");
	printf("***************\n\n");
	printf("Welcome!\n");
}

void bprint(sead_c x) { //print binary value of x
	for (int i =COUNTER_SIZE-1; i >= 0; --i) {
		printf("%hhd", (x & (1<<i)) >> i);
	}
	printf("\n");
}


int get_val(sead_c x, int base = 0) {//return the actual unsigned value stored in one SAC
	//base is the starting scale. 0 meaning multiply by 2^0
	unsigned short pos = 0; //0 means the seperating 0-bit starts at the front
	sead_c delta = high_bit;
	while ((x & delta) && delta >= 1) {  // flip and bit op should be faster 
		++pos;
		delta = delta >> 1;
	}
	sead_c ones = (1<<(COUNTER_SIZE-1-pos))-1;
	return ((ones & x) << (pos + base)) + pos*(1<<(base+COUNTER_SIZE-1));
}

int get_signed_val(sead_c x, int base = 0) { //return the actual signed value stored in one SAC
	if (x >= MAX_CNT_CO)return MAX_CNT;
	if (x <= MIN_CNT_CO)return MIN_CNT;
	unsigned short pos = 0;
	sead_c delta = second_high_bit;
	if (x < 0) {
		sead_c y = ~x + 1;
		while ((y & delta) && delta >= 1) {
			++pos;
			delta = delta >> 1;
		}
		sead_c ones = (1<<(COUNTER_SIZE-1-pos))-1;
		return -((ones & y) << (pos + base))-pos*(1<<(base+COUNTER_SIZE-1));
	}
	else {
		while ((x & delta) && delta >= 1) {
			++pos;
			delta = delta >> 1;
		}
		sead_c ones = (1<<(COUNTER_SIZE-1-pos))-1;
		return ((ones & x) << (pos + base)) + pos*(1<<(base+COUNTER_SIZE-1));
	}
}
	
//this function returns the predict value of an 16-bit static version SAC
LL predict(sead_c x, int l_sign, LL* gamma,bool sp_negative=true) {  //l_sign is the length of sign section
	sead_c delta = sp_negative?second_high_bit:high_bit; //This variable is used to get the value of certain bit in SAC
	int total_length=sp_negative?COUNTER_SIZE-1:COUNTER_SIZE;
	int sign_bits = 0;
	double ret = 0;
	sead_c y=x<0?(~x+1):x; //if x is nagative, take the 2's complement of it
	//read the sign bits
	for (int i = 0; i < l_sign; ++i) {
		if (y & delta) {
			sign_bits += 1<<(l_sign - i - 1);
		}
		delta = delta >> 1;
	}
	sead_c ones = (1<<(total_length-l_sign))-1;
	//add up the value for all previous stages
	for (int i = 0; i < sign_bits; ++i) {
		ret += gamma[i] * ((1<<(total_length - l_sign)) - 1);
	}
	return (gamma[sign_bits] * (ones & y)+ret)*(x<0?-1:1);
}

//this function returns the predict value of an 16-bit dynamic version SAC
LL predict(sead_c x, LL* gamma,bool sp_negative=true) {
	unsigned short pos = 0;
	//sead_c delta = sp_negative?second_high_bit:high_bit;
	int total_length=sp_negative?(COUNTER_SIZE-1):COUNTER_SIZE;
	//if(!sp_negative) printf("%d   %d     ",total_length,x);
	LL answer = 0;
	sead_c y =x<0?(~x + 1):x; //if x is nagative, take the 2's complement of it
	
	/*while ((y & delta) && delta >= 1) {
		++pos;
		delta = delta >> 1;
	}*/
	sead_c y_new=sp_negative?(y|second_high_bit_ones):(y|high_bit_ones);
	pos=sp_negative?(__builtin_clz(~y_new)-(32-COUNTER_SIZE+1)):(__builtin_clz(~y_new)-(32-COUNTER_SIZE));
	sead_c ones = (1<<(total_length-pos))-1;
	answer += (ones & y) * gamma[pos];
	for (int i = 0; i < pos; ++i) {
		answer += gamma[i] * (1<<(total_length-1 - i));
	}
	return x<0?-answer:answer;
}



//this is the add one function for fixed version of 16-bit SAC
void add_one(sead_c& x, int l_sign, LL* gamma,bool sp_negative=true) {
	sead_c MAX_CNT_C = sp_negative?MAX_CNT_CO:MAX_CNT_CO_P;
	if (predict(x, l_sign,gamma,sp_negative) + 1 >= max_predict_sta) {
//		cout << "overflow!" << endl;
		x = MAX_CNT_C;
		return;
	}
	sead_c delta = sp_negative?second_high_bit:high_bit;
	sead_c y=x<0?(~x+1):x;
	int sign_bits = 0;
	
	for (int i = 0; i < l_sign; ++i) {
		if (y & delta) {
			sign_bits += 1<<(l_sign - i - 1);
		}
		delta = delta >> 1;
	}
	double add = 1.0 / gamma[sign_bits];
	double r = (rand()%RAND_MAX) / (double)(RAND_MAX);
	if (r < add)++x;
	return;
}

//this is the minus one function for fixed version of 16-bit SAC
void minus_one(sead_c& x, int l_sign, LL* gamma,bool sp_negative=true) {
	sead_c MIN_CNT_C = sp_negative?MIN_CNT_CO:0;
	if (predict(x, l_sign,gamma,sp_negative) - 1 <= predict(MIN_CNT_C,l_sign,gamma,sp_negative)) {
		printf("underflow!\n");
		x = MIN_CNT_C;
		return;
	}
	sead_c delta = sp_negative?second_high_bit:high_bit;
	int sign_bits = 0;
	sead_c y=x<0?(~x+1):x;
	
	for (int i = 0; i < l_sign; ++i) {
		if (x & delta) {
			sign_bits += 1<<(l_sign - i - 1);
		}
		delta = delta >> 1;
	}
	double add = 1.0 / gamma[sign_bits];
	double r = (rand()%RAND_MAX) / (double)(RAND_MAX);
	if (r < add)--x;
	return;
}


void adding(sead_c& x, int c, LL* gamma,bool sp_negative=true) {
	//bprint(x);
	if (c < 0) { printf("illeagal adding\n"); return; }
	sead_c MAX_CNT_C = sp_negative?MAX_CNT_CO:MAX_CNT_CO_P;
	/*if (predict(x, gamma,sp_negative) + c >= max_predict_dyn) {
//		printf("overflow!\n");
		x = MAX_CNT_C;
		return;
	}*/
	unsigned short pos = 0;
	//sead_c delta = sp_negative?second_high_bit:high_bit;
	sead_c y=(x < 0)?(~x+1):x;
	sead_c y_new=sp_negative?(y|second_high_bit_ones):(y|high_bit_ones);
	pos=sp_negative?(__builtin_clz(~y_new)-(32-COUNTER_SIZE+1)):(__builtin_clz(~y_new)-(32-COUNTER_SIZE));
	/*while ((y & delta) && delta >= 1) {
		++pos;
		delta = delta >> 1;
	}*/
	
	//now it's clear that the "gain" at this stage is gamma[pos]
	//and we first add c/gamma[pos] to the counter
	double add = (double)c / gamma[pos];
	//printf("add: %f\n",add);
	if (add <= 1) {
		if (drand48() < add){++x;}//printf("x: %d ++\n",x);}
		return;
	}
	else {
		sead_c distance=x<0?(y-edge[pos]):(edge[pos+1]-x);
		if (add <= distance) {
			x += (int)add;
			if (drand48() < add - (int)add)++x;
			//printf("%d   ",x);
			return;
		}
		else {
			//cout<<"add went from "<<add<<" to ";
			//cout<<add<<"       here x= "<<x<<" y= "<<y<<"pos= "<<pos<<endl;
			x = (x<0)?(-edge[pos]+1):edge[pos+1];
			adding(x, c-distance * gamma[pos], gamma);
			return;

		}
	}
}

//this function substract c from the 16-bit Dynamic version of SAC
void subtracting(sead_c& x, int c, LL* gamma,bool sp_negative=true) {
	if (c < 0) { printf("illeagal subtracting\n"); return; }
	sead_c MIN_CNT_C = sp_negative?MIN_CNT_CO:0;
	//printf("x: %d   %lld\n",x,predict(x,gamma,sp_negative)-c);
	if (((predict(x, gamma,sp_negative)) - c) <= predict(MIN_CNT_C, gamma,sp_negative)) {
		printf("underflow! \n");
		x = MIN_CNT_C;
		return;
	}
	unsigned short pos = 0;
	sead_c delta = sp_negative?second_high_bit:high_bit;
	sead_c y=(x <= 0)?(~x+1):x;
	while ((y & delta) && delta >= 1) {
		++pos;
		delta = delta >> 1;
	}
	double minus = (double)c / gamma[pos];
	if (minus < 1) {
		if ((rand() % RAND_MAX) / (double)RAND_MAX < minus)--x;
		return;
	}
	else {
		sead_c distance=x<0?(edge[pos+1]-y):(x-edge[pos]);
		//printf("distance: %d\n",distance);
		if ((int)minus <= distance) {
			x -= (int)minus;
			if ((rand()%RAND_MAX)  / (double)RAND_MAX < minus - (int)minus)--x;
			return;
		}
		else {
			x = (x<0)?(-edge[pos + 1]):(edge[pos]-1);
			//printf("second:%lld\n",c-distance*gamma[pos]);
			subtracting(x, c-distance * gamma[pos], gamma);
			return;
		}
	}
}
