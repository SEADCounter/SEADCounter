#pragma once

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/timeb.h>
#include <iostream>
using namespace std;
const int COUNTER_SIZE= 16; //set the size of counter according to the chosen type
const int MAX_CNT=(1 << (COUNTER_SIZE - 1)) - 1;
const int MIN_CNT=-(1 << (COUNTER_SIZE - 1));

using type = uint8_t;      //different types of counters
using sign = int8_t;
using long_sign = int16_t;

const type powers[10]={1,2,4,8,16,32,64,128};

const uint16_t long_powers[20]={1,2,4,8,16,32,64,128,
                      256,512,1024,2048,4096,
                      8192,16384,32768}; //list out the power of 2 for later use

const sign edge[8]={0,64,96,112,120,124,126,127};
//edges are the value of counter for the starting of each stages for SAC
const long_sign long_edge[17]={0,16384,24576,28672,30720,31744,32256,32512,32640,32704,32752,32760,32764,32766,32767};



const type high_bit=128;
const type second_high_bit=64;

const long_sign long_high_bit=32768; //the value of 16-bit counter where only the leftmost bit is 1
const long_sign long_second_high_bit=16384;

const long_sign long_maxi=32767; //the maximum value for signed 16-bit counter
const long_sign long_mini=-32767;


void greeting(){
    cout<<"***************"<<endl;
    cout<<"**This is SAC**"<<endl;
    cout<<"***************"<<endl<<endl;
    cout<<"Welcome! To test SAC on different datasets,"<<endl;
    cout<<"Type 'CAIDA','webpage' or 'synthetic'"<<endl;
}

void bprint(type x){ //print binary value of x
    for(int i=7;i>=0;--i){
        printf("%hhd",(x&powers[i])>>i);
    }
    cout<<endl;
}

void bprint(long_sign x){ //print binary value of x
    for(int i=15;i>=0;--i){
        printf("%d",(x&long_powers[i])>>i);
    }
    cout<<endl;
}


int get_val(type x,int base=0){//return the actual unsigned value stored in one SAC
    //base is the starting scale. 0 meaning multiply by 2^0
    unsigned short pos=0; //0 means the seperating 0-bit starts at the front
    type delta=high_bit;
    type ones=0;
    while((x&delta)&&delta>=1){
        ++pos;
        delta=delta>>1;
    }
    for(int i=0;i<=6-pos;++i){
        ones += powers[i];
    }
    return ((ones&x)<<(pos+base))+(128*pos*pow(2,base));
}

int get_signed_val(sign x,int base=0){ //return the actual signed value stored in one SAC
    if(x>=125)return MAX_CNT;
    if(x<=-125)return MIN_CNT;
    unsigned short pos=0;
    type delta=second_high_bit;
    type ones=0;
    if(x<0){
        type y = ~x+1;
        while((y&delta)&&delta>=1){
            ++pos;
            delta=delta>>1;
        }
        for(int i=0;i<=6-pos;++i){
            ones += powers[i];
        }

        return -((ones&y)<<(pos+base))-(64*pos*pow(2,base));
    }
    else{
        while((x&delta)&&delta>=1){
            ++pos;
            delta=delta>>1;
        }
        for(int i=0;i<=6-pos;++i){
            ones += powers[i];
        }
        return ((ones&x)<<(pos+base))+(64*pos*pow(2,base));
    }
}

//this function retures the value of signed 8-bit SAC
double predict(sign x,double *gamma){ 
    if(x>=125)return MAX_CNT;
    if(x<=-125)return MIN_CNT;
    unsigned short pos=0;
    type delta=second_high_bit;
    type ones=0;
    double answer=0;
    if(x<0){
        type y = ~x+1;
        while((y&delta)&&delta>=1){
            ++pos;
            delta=delta>>1;
        }
        for(int i=0;i<=6-pos;++i){
            ones += powers[i];
        }
        answer-=(ones&y)*gamma[pos];
        for(int i=0;i<pos;++i){
            answer -= gamma[i]*pow(2,6-i);
        }
        return answer;
    }
    else{
        while((x&delta)&&delta>=1){
            ++pos;
            delta=delta>>1;
        }
        for(int i=0;i<=6-pos;++i){
            ones += powers[i];
        }
        answer += (ones&x)*gamma[pos];
        for(int i=0;i<pos;++i){
            answer += gamma[i]*pow(2,6-i);
        }
        return answer;
    }
}

//this function returns the predict value of an 16-bit static version SAC
double predict(long_sign x, int l_sign,double *gamma){  //l_sign is the length of sign section
    long_sign delta=long_second_high_bit; //This variable is used to get the value of certain bit in SAC
    int sign_bits=0;
    int ones=0;
    double ret=0;
    if(x<0){
        long_sign y= ~x+1; //if x is nagative, take the 2's complement of it
        //read the sign bits
        for(int i=0;i<l_sign;++i){
            if(y&delta){
                sign_bits += pow(2,l_sign-i-1);
            }
            delta=delta>>1;
        }
        for(int i=0;i<15-l_sign;++i){
            ones+=long_powers[i];
        }
        //add up the value for all previous stages
        for(int i=0;i<sign_bits;++i){
            ret+=gamma[i]*(long_powers[15-l_sign]-1);
        }
        return -gamma[sign_bits]*(ones&y)-ret;

    }
    else{   //if x is positive
        for(int i=0;i<l_sign;++i){
            if(x&delta){
                sign_bits += pow(2,l_sign-i-1);
            }
            delta=delta>>1;

        }
        for(int i=0;i<15-l_sign;++i){
            ones+=long_powers[i];
        }
        //add up the value for all previous stages
        for(int i=0;i<sign_bits;++i){
            ret+=gamma[i]*(long_powers[15-l_sign]-1);
        }
        return gamma[sign_bits]*(ones&x)+ret;
    }
}

//this function returns the predict value of an 16-bit dynamic version SAC
double predict(long_sign x,double *gamma){
    unsigned short pos=0;
    long_sign delta=long_second_high_bit;
    long_sign ones=0;
    double answer=0;
    if(x<0){
        long_sign y = ~x+1; //if x is nagative, take the 2's complement of it
        while((y&delta)&&delta>=1){
            ++pos;
            delta=delta>>1;
        }
        for(int i=0;i<=14-pos;++i){
            ones += long_powers[i];
        }
        answer-=(ones&y)*gamma[pos];
        for(int i=0;i<pos;++i){
            answer -= gamma[i]*pow(2,14-i);
        }
        return answer;
    }
    else{
        while((x&delta)&&delta>=1){
            ++pos;
            delta=delta>>1;
        }
        for(int i=0;i<=14-pos;++i){
            ones += long_powers[i];
        }
        answer += (ones&x)*gamma[pos];
        for(int i=0;i<pos;++i){
            //[](int (*f)(int)){return [=](int i){return f(i);};}([](int x){return x*x;})(3);
            answer += gamma[i]*pow(2,14-i);

        }
        return answer;
    }
}

void add_one(type &x){
    if(x>=253){cout<<"overflow!"<<endl;return;}
    if((x&high_bit)==0){++x;return;}
    int pos=0;
    type delta=high_bit;
    while((x&delta)&&delta>=1){
        ++pos;
        delta=delta>>1;
    }
    if(rand()%(1<<pos)==1)
        ++x;
    return;
}

//this a the procedure to add one on a signed 8-bit SAC
void add_one(sign &x, int base){
    if(x>=125){cout<<"overflow!"<<endl;return;}
    unsigned short pos=0;
    type delta=second_high_bit;
    if(x<0){
        type y = ~x+1;
        while((y&delta)&&delta>=1){
            ++pos;
            delta=delta>>1;
        }
        if(rand()%(1<<(pos+base))==0)
            ++x;
        return;
    }
    while((x&delta)&&delta>=1){
        ++pos;
        delta=delta>>1;
    }
    if(rand()%(1<<(pos+base))==0)
        ++x;
    return;

}

//this is the add one function for fixed version of 16-bit SAC
void add_one(long_sign &x, int l_sign,double* gamma){
    if(predict(x,gamma)+1 >= predict(long_maxi,gamma)){
        cout<<"overflow!"<<endl;
        x=long_maxi;
        return;
    }
    long_sign delta=long_second_high_bit;
    int sign_bits=0;
    if(x<0){
        long_sign y= ~x+1;
        for(int i=0;i<l_sign;++i){
            if(y&delta){
                sign_bits += pow(2,l_sign-i-1);
            }
            delta=delta>>1;
        }
        double add=1/gamma[sign_bits];
        double r=(rand()%RAND_MAX)/(double)(RAND_MAX);
            if(r < add)++x;
            return;
    }
    for(int i=0;i<l_sign;++i){
        if(x&delta){
            sign_bits += pow(2,l_sign-i-1);
        }
        delta=delta>>1;
    }
    double add=1.0/gamma[sign_bits];
    double r=(rand()%RAND_MAX)/(double)(RAND_MAX);
        if(r < add)++x;
        return;

}

//this is the minus one function for fixed version of 16-bit SAC
void minus_one(long_sign &x, int l_sign,double* gamma){
    if(predict(x,gamma)-1 <= predict(long_mini,gamma)){
        cout<<"underflow!"<<endl;
        x=long_mini;
        return;
    }
    long_sign delta=long_second_high_bit;
    int sign_bits=0;
    if(x<=0){
        long_sign y= ~x+1;
        for(int i=0;i<l_sign;++i){
            if(y&delta){
                sign_bits += pow(2,l_sign-i-1);
            }
            delta=delta>>1;
        }
        double add=1/gamma[sign_bits];
        double r=(rand()%RAND_MAX)/(double)(RAND_MAX);
            if(r < add)--x;
            return;
    }
    for(int i=0;i<l_sign;++i){
        if(x&delta){
            sign_bits += pow(2,l_sign-i-1);
        }
        delta=delta>>1;
    }
    double add=1.0/gamma[sign_bits];
    double r=(rand()%RAND_MAX)/(double)(RAND_MAX);
        if(r < add)--x;
        return;

}

//this function adds c to a signed 8-bit SAC
void adding(sign &x,int c,double *gamma){
    if(c<0){cout<<"illeagal adding"<<endl;return;}
    if(x>=125){cout<<"overflow!"<<endl;return;}
    unsigned short pos=0;
    type delta=second_high_bit;
    if(x<0){
        type y = ~x+1;
        while((y&delta)&&delta>=1){
            ++pos;
            delta=delta>>1;
        }
        //now it's clear that the "gain" at this stage is gamma[pos]
        //and we first add c/gamma[pos] to the counter
        double add=c/gamma[pos];
        if(add<1){
            if((rand()%RAND_MAX)/(double)RAND_MAX < add)++x;
            return;
        }
        else{
            if(add<=y-edge[pos]){
                x+=(int)add;
                if((rand()%RAND_MAX)/(double)RAND_MAX < add-(int)add)++x;
                return;
            }
            else{
                add-=y-edge[pos];
                x=-edge[pos];
                adding(x,add*gamma[pos],gamma);
                return;
            }
        }
    }
    else{
        while((x&delta)&&delta>=1){
            ++pos;
            delta=delta>>1;
        }
        double add=c/gamma[pos];
        if(add<1){
            if((rand()%100)/101.0 < add)++x;
            return;
        }
        else{
            if(add < (edge[pos+1]-x)){
                x+=(int)add;
                if((rand()%100)/101.0 < add-(int)add)++x;
                return;
            }
            else{
                add-=edge[pos+1]-x;
                x=edge[pos+1];
                adding(x,add*gamma[pos],gamma);
                return;
            }
        }
    }
}

void adding(long_sign &x,int c,double *gamma){
    if(c<0){cout<<"illeagal adding "<<c<<endl;return;}
    if(predict(x,gamma)+c >= predict(long_maxi,gamma)){
        cout<<"overflow!"<<endl;
        x=long_maxi;
        return;
    }
    unsigned short pos=0;
    uint16_t delta=long_second_high_bit;
    if(x<0){
        uint16_t y = ~x+1;
        while((y&delta)&&delta>=1){
            ++pos;
            delta=delta>>1;
        }
        //now it's clear that the "gain" at this stage is gamma[pos]
        //and we first add c/gamma[pos] to the counter
        double add=c/gamma[pos];
        if(add<1){
            if((rand()%RAND_MAX)/(double)RAND_MAX < add)++x;
            return;
        }
        else{
            if(add<=y-long_edge[pos]){
                x+=(int)add;
                if((rand()%RAND_MAX)/(double)RAND_MAX < add-(int)add)++x;
                return;
            }
            else{
                //cout<<"add went from "<<add<<" to ";
                add-=(y-long_edge[pos]);
                //cout<<add<<"       here x= "<<x<<" y= "<<y<<"pos= "<<pos<<endl;
                x=-long_edge[pos]+1;
                return adding(x,add*gamma[pos],gamma);

            }
        }
    }
    else{
        while((x&delta)&&delta>=1){
            ++pos;
            delta=delta>>1;
        }
        double add=c/gamma[pos];
        if(add<1){
            if((rand()%RAND_MAX)/(double)RAND_MAX < add)++x;
            return;
        }
        else{
            if(add < (long_edge[pos+1]-x)){
                x+=(int)add;
                if((rand()%RAND_MAX)/(double)RAND_MAX < add-(int)add)++x;
                return;
            }
            else{
                add-=long_edge[pos+1]-x;
                x=long_edge[pos+1];
                return adding(x,add*gamma[pos],gamma);

            }
        }
    }
}

//this function substract c from the 16-bit Dynamic version of SAC
void subtracting(long_sign &x, int c,double *gamma){
    if(c<0){cout<<"illeagal subtracting"<<endl;return;}
    if( ((predict(x,gamma))-c) <= predict(long_mini,gamma) ){
        cout<<"underflow! "<<(predict(x,gamma))<<" "<<c<<" "<<predict(long_mini,gamma)<<endl;
        x=long_mini;
        return;
    }
    unsigned short pos=0;
    uint16_t delta=long_second_high_bit;
    if(x<=0){//It's important to deal with 0 here in this case
        uint16_t y = ~x+1;
        while((y&delta)&&delta>=1){
            ++pos;
            delta=delta>>1;
        }
        double minus=c/gamma[pos];
        if(minus<1){
            if((rand()%RAND_MAX)/(double)RAND_MAX < minus)--x;
            return;
        }
        else{
            if(minus<=long_edge[pos+1]-y){
                x-=(int)minus;
                if((rand()%RAND_MAX)/(double)RAND_MAX < minus-(int)minus)--x;
                return;
            }
            else{
                minus-=(long_edge[pos+1]-y);
                x=-long_edge[pos+1];
                return subtracting(x,minus*gamma[pos],gamma);
            }
        }
    }
    else{
        while((x&delta)&&delta>=1){
            ++pos;
            delta=delta>>1;
        }
        double minus=c/gamma[pos];
        if(minus<1){
            if((rand()%RAND_MAX)/(double)RAND_MAX < minus)--x;
            return;
        }
        else{
            if(minus < (x-long_edge[pos])){
                x-=(int)minus;
                if((rand()%RAND_MAX)/(double)RAND_MAX < minus-(int)minus)--x;
                return;
            }
            else{
                //cout<<"this x="<<x<<endl;
                minus-=(x-long_edge[pos]);
                //cout<<minus<<" "<<long_edge[pos]<<endl;
                x=long_edge[pos]-1  ;
                //cout<<"new minus="<<minus<<endl;
                return subtracting(x,minus*gamma[pos],gamma);
            }
        }
    }
}

//this function substract c from the 8-bit SAC
void subtracting(sign &x,int c,double *gamma){

    if(x<=-125){cout<<"underflow!"<<endl;return;}
    unsigned short pos=0;
    type delta=second_high_bit;
    if(x<=0){//It's important to deal with 0 here in this case
        type y = ~x+1;
        while((y&delta)&&delta>=1){
            ++pos;
            delta=delta>>1;
        }
        //now it's clear that the "gain" at this stage is gamma[pos]
        //and we first minus c/gamma[pos] to the counter
        double minus=c/gamma[pos];
        if(minus<1){
            if((rand()%100)/101.0 < minus)--x;
            return;
        }
        else{
            if(minus<=edge[pos+1]-y){
                x-=(int)minus;
                if((rand()%100)/101.0 < minus-(int)minus)--x;
                return;
            }
            else{
                minus-=(edge[pos+1]-y);
                x=-edge[pos+1];
                subtracting(x,minus*gamma[pos],gamma);
                return;
            }
        }
    }
    else{
        while((x&delta)&&delta>=1){
            ++pos;
            delta=delta>>1;
        }
        double minus=c/gamma[pos];
        if(minus<1){
            if((rand()%100)/101.0 < minus)--x;
            return;
        }
        else{
            if(minus < (x-edge[pos])){
                x-=(int)minus;
                if((rand()%100)/101.0 < minus-(int)minus)--x;
                return;
            }
            else{
                minus-=(x-edge[pos]);
                x=edge[pos];
                subtracting(x,minus*gamma[pos],gamma);
                return;
            }
        }
    }
}


 // ADDER_H

