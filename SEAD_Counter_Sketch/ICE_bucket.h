#pragma once

#include<iostream>
#include<cmath>
#include<cfloat>
#include<cstdlib>
#include<algorithm>
#include<cstring>
#include<climits>
#include"params.h"

using namespace std;

const double EPSILON=0.00001;
const int MAX_ITERS = 1<<18;

class Bucket{
public:
    int cedar_index;
    short symbol[1<<10];
    Bucket(){
        this->cedar_index=0;
        memset(this->symbol,0,sizeof(short)*(1<<10));
    }
};


// estimator function 
class OptimalEstimator{
public:
    LL L,M;
    double m,q;
    double epsilon;
    OptimalEstimator(){}
    OptimalEstimator(LL L,LL M=-1,double epsilon=-DBL_MAX){
        this->L=L;
        this->epsilon=epsilon;
        if(epsilon!=-DBL_MAX){
            this->update_parameters(epsilon);
        }
        else if(M!=-1){
            this->init_parameters(L,M);
        }
        else 
            printf("Value Error!\n");
    }
    void update_parameters(double epsilon){
        this->epsilon = epsilon;
        if(epsilon>0&&epsilon<1){
            this->m=(1.0+pow(this->epsilon,2.0))/(2*pow(this->epsilon,2.0));
            
            this->q=(1.0+2*pow(this->epsilon,2.0));
            //printf("%f   %f   \n ",this->m,this->q);
        }
        else{
            this->m=1.0;
            this->q=2.0;
        }
        return;
    }
    void init_parameters(LL L,LL M){
        double min_key=0,max_key=1-1e-4,required_value=M;
        double tolerance=1e-4;
        double f_min_key=this->get_max_value(min_key);
        double f_max_key=this->get_max_value(max_key);
        if(f_min_key==DBL_MAX||f_max_key==DBL_MAX){
            printf("Overflow!\n");
            return;
        }
        if(f_max_key<required_value||f_min_key>required_value){
            printf("%f ValueError! %f  %f  \t   ",required_value,f_min_key,f_max_key);
            return;
        }
        double current_key=(min_key+max_key)/2;
        double f_current_key=this->get_max_value(current_key);
        if(f_current_key==DBL_MAX){
            printf("Overflow!\n");
            return;
        }
        int c=0;
        while(abs(f_current_key-required_value)>tolerance&& c<MAX_ITERS){
            if(f_current_key-required_value<-tolerance)
                min_key=current_key;
            else if(f_current_key-required_value>tolerance){
                max_key=current_key;
            }
            current_key=(min_key+max_key)/2;
            f_current_key=this->get_max_value(current_key);
            if(f_current_key==DBL_MAX){
                printf("Overflow!\n");
                return;
            }
            c++;
            //printf("%d\n",c);
        }
        return;
    }
    double get_max_value(double epsilon=-DBL_MAX){
        if(epsilon==-DBL_MAX)
            epsilon=this->epsilon;
        else if(epsilon!=this->epsilon)
            this->update_parameters(epsilon);
        return this->estimate(this->L-1);
    }
    double estimate(LL symbol){
        if(symbol<0||symbol>=this->L){
            //printf("symbol: %lld\n",symbol);
            //printf("symbol overflow!\n");
            return -DBL_MAX;
        }
        if(this->epsilon<EPSILON)
            return symbol;
        if(this->epsilon>=1)
            return 0;
        double result=(LL)this->m*(pow(this->q,symbol)-1);
        //printf("%f\t  %f   \t   %f \t %lld\n",result,this->m,this->q,symbol);
        return result;
    }
    LL inc(LL symbol){
    
        double sym_val1=this->estimate(symbol+1);
        if(sym_val1==-DBL_MAX)
            return LLONG_MIN;
        double diff=sym_val1-this->estimate(symbol);
        //printf("%f\n",diff);
        if(diff<=0)
            return symbol;
        if(rand()/(double)RAND_MAX<1.0/diff)
        {
            return symbol+1;
         }   
        else return symbol;
    }
    LL upscale(const short& symbol,OptimalEstimator& new_OpEst){
        double val=this->estimate(symbol);
        LL symbol_new=log(1+val/new_OpEst.m)/log(new_OpEst.q);
        double diff1=this->estimate(symbol)-new_OpEst.estimate(symbol_new);
        double diff2=new_OpEst.estimate(symbol_new+1)-new_OpEst.estimate(symbol_new);
        if(rand()<diff1/(double)diff2)
            return symbol_new+1;
        else return symbol_new;
    }
};




class ICEBuckets{
public:
    int log_steps=INT_MIN,M_step,nupscales,memory_overhead;
    LL E,S,bucket_size,L,M,total_packets_expected,N,B;
    double epsilon_step;
    OptimalEstimator CEDARS[1<<15];   // E should be less than 1<<15 or inc E here
    Bucket * buckets;
    
    bool should_do_global_upscale=true;

    ICEBuckets(LL N,LL L,LL M,int memory_overhead,int log_steps=INT_MIN)
    {
    	
        //this->should_do_global_upscale=should_do_global_upscale;
        if(log_steps==INT_MIN)
            this->log_steps=log_steps=this->find_best_e(N,L,M,memory_overhead);
        else
            this->log_steps=log_steps;
        //printf("e = %d\n",this->log_steps);
        this->N=N;
        this->M=M;
        this->memory_overhead=memory_overhead;
        this->E=1<<log_steps;
        this->B=memory_overhead/log_steps;
        this->L=L;
        this->S=this->bucket_size=this->N/this->B;
        this->B+=1;
        
        
        //printf("Bound = %f \n",this->bound(this->N,this->L,this->M,this->B,this->E));
        //printf("N= %lld\t M= %lld\t  E= %lld\t L=%lld\t B=%lld\t S=%lld\t\n",this->N,this->M,this->E,this->L,this->B,this->S);
        this->nupscales=0;
        OptimalEstimator a=OptimalEstimator(this->L,this->M);
        if(should_do_global_upscale)
            this->epsilon_step=a.epsilon/((this->E-1)*16.0); // how 16
        else 
            this->epsilon_step=a.epsilon/(this->E-1);
        
        //this->M=this->L-1+this->M_step*(this->E-1);
        this->total_packets_expected=M;
        
        for(int i=0;i<=this->E;++i)
            this->CEDARS[i]=OptimalEstimator(this->L,-1,this->epsilon_step*i);
        /*
        this->buckets=(Bucket **)malloc(sizeof(Bucket*)*this->B);
        if(o==NULL) printf("bad malloc!\n");
        */
        
        this->buckets=new Bucket[this->B]();
        
        
    }
    LL find_best_e(LL N,LL L,LL M,int memory_overhead){
        double best=DBL_MAX;
        LL best_e=0;
        
        for(int i=1;i<log(M)/log(2.0);++i){
            //printf("\nin best_e  %lld\n\n",i);
            LL E=pow(2,i);
            LL B=memory_overhead/i;
            LL S=N/B;
            double bound = this->bound(N,L,M,B,E);
            //printf("bound=: %f\n",bound);
            if(bound<best)
                best_e=i;
                best=bound;
        }
        return best_e;
    }
    double bound(LL N,LL L,LL M,LL B,LL E){
    	//printf("%lld %lld %lld in bound!\n",M,B,L);
        return OptimalEstimator(L,M/(double)B+L-1).epsilon+OptimalEstimator(L,M).epsilon/(E-1);
    }
    int get_memory(){
        int l=log(this->L)/log(2.0);  // log?
        return this->B*(this->S+log_steps);

    }
    LL estimate(int buc_num,int buc_sym){
        int cedar_index_buc=this->buckets[buc_num].cedar_index;
        int sym = this->buckets[buc_num].symbol[buc_sym];
        //printf("symbol: %d \n",sym);
        return this->CEDARS[cedar_index_buc].estimate(sym);
    }
    void upscale(Bucket& Buc){
        for(int i=0;i<this->bucket_size;++i)
            Buc.symbol[i]=this->CEDARS[Buc.cedar_index].upscale(
                Buc.symbol[i],this->CEDARS[Buc.cedar_index+1]
            );
        Buc.cedar_index+=1;
        this->nupscales+=1;
        return;
    }
    void total_upscale(){
        this->CEDARS[this->E]=OptimalEstimator(this->L,-1,this->epsilon_step*this->E);
        for(int i=0;i<this->B;++i){
            if(this->buckets[i].cedar_index%2==1)
                this->upscale(this->buckets[i]);
            this->buckets[i].cedar_index/=2;
        }
        //printf("total_upscale happened!\n");
        this->epsilon_step=this->epsilon_step*2;
        for(int i=0;i<=this->E;++i)
            this->CEDARS[i]=OptimalEstimator(this->L,-1,this->epsilon_step*i);
        return;
    }
    LL inc(int buc_num,int buc_sym){
        Bucket *Buc=&(this->buckets[buc_num]);
        LL ret_buc_val=this->CEDARS[Buc->cedar_index].inc(Buc->symbol[buc_sym]);
        
        //printf("ret_buc:%lld\n",ret_buc_val);
        if(ret_buc_val==LLONG_MIN)
        {
            //printf("overflow happend!");
            if(Buc->cedar_index+1>=this->E)
                {
                    double max_epsilon=this->CEDARS[this->E-1].epsilon;
                    //printf("max_epsilon: %f\n",max_epsilon);
                    if(max_epsilon<1.0 && this->should_do_global_upscale)
                        this->total_upscale();
                }
            else{
            	 //printf("upscale happened!\n");
                if(this->CEDARS[Buc->cedar_index].epsilon<1.0){
                    //printf("epsilon: %f\n",this->CEDARS[Buc->cedar_index].epsilon);
                    this->upscale(*Buc);
                }
            }
            ret_buc_val=this->CEDARS[Buc->cedar_index].inc(Buc->symbol[buc_sym]);
            
            //if last upscale failed, try again.
            if(ret_buc_val==LLONG_MIN){
            	printf("another upscale try!\n");
                double max_epsilon=this->CEDARS[this->E-1].epsilon;
                if(max_epsilon<1.0 && this->should_do_global_upscale)
                    this->total_upscale();
                    this->inc(buc_num,buc_sym);
            }
        }
        else{
            Buc->symbol[buc_sym]=ret_buc_val;
        }
        return ret_buc_val;
    }
    ~ICEBuckets(){
        delete [] this->buckets;
    }
};
