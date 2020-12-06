#pragma once

//base class to make experimet function and main function simple and clear

class class_sketches {
public:
	virtual void Insert(const char * str, int c) {}
	virtual int Query(const char *str) = 0;
	virtual void dynamic_sac_insert(const char *str, int c, long long int *gamma,bool sp_negative=true) {}
	virtual int dynamic_sac_query(const char *str, long long int *gamma,bool sp_negative=true) = 0;
	virtual void static_sac_insert(const char *str, int l_sign, long long int *gamma,bool sp_negative=true) {}
	virtual int static_sac_query(const char *str, int l_sign, long long int *gamma,bool sp_negative=true) = 0;
	virtual void check(const char* str,int l_sign,long long int *gamma) {}
	virtual void SmallActiveCounter_insert(const char* str){}
	virtual int SmallActiveCounter_query(const char* str){return 0;}
	virtual void icebuckets_insert(const char* str){}
	virtual int icebuckets_query(const char* str){return 0;}
};
