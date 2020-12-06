#pragma once


#define LL long long int

#define LOOP_NUM 10
#define KIND_OF_COUNTERS 3
 
typedef int sead_c;  

//set the size of counter according to the chosen type


#define MAX_INT_CNT 2147483647
#define MIN_INT_CNT -1<<31
const int MAX_CNT_CO = (1 << (COUNTER_SIZE - 1)) - 1;
const int MIN_CNT_CO = -(1 << (COUNTER_SIZE - 1))+1;
const int MAX_CNT_CO_P = (1 << COUNTER_SIZE) - 1;
const int MAX_CNT = (COUNTER_SIZE)*(1 << (COUNTER_SIZE - 2));
const int MIN_CNT = -(COUNTER_SIZE)*(1 << (COUNTER_SIZE - 2));
const sead_c high_bit = 1<<(COUNTER_SIZE-1);  
const sead_c second_high_bit = 1<<(COUNTER_SIZE-2);
const sead_c high_bit_ones=~MAX_CNT_CO_P;
const sead_c second_high_bit_ones=~MAX_CNT_CO;



const int prealloc_flow_num = 10000000;


// average 12.5 bits for all flows. For ICEbucket, 12 bit for
// estimator, 0.5 bit for E. For SEAD, 12 bit for all the estimators
// defined in Makefile
//#define per_estimator_bit  12.5
//#define per_estimator_int  12

// if the type of gamma value is double, should change here.
//LL gamma_2[20] = {1, 4, 16, 128, 1024, 4096, 16384, 16384, 65536, 262144,1048576, 4194304, 16777216, 67108864, 268435456};
LL gamma_2[20] = {1, 4, 16, 64, 256, 1024,4096, 16384, 65536, 262144,1048576, 4194304, 16777216, 67108864, 268435456};
const int per_estimator_mode = 4;

const int read_packet_per_times = 30000000;
// should be adjusted according to the dataset size
// no more than 9000000 for datacenter.dat

const int read_packet_print = 3000000;
// should be adjusted according to the dataset size
// 10000000 for CAIDA_large.dat




