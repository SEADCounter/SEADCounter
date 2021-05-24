#pragma once

#include "sead.h"
//base class to make experimet function and main function simple and clear

class class_sketches {
public:
	virtual void Insert(const char * str, int c) {}
	virtual int Query(const char *str) = 0;
	virtual void dynamic_sead_insert(const char *str, int c, LL *gamma) {}
	virtual int dynamic_sead_query(const char *str, LL *gamma) = 0;
	virtual void static_sead_insert(const char *str, int l_sign, LL *gamma) {}
	virtual int static_sead_query(const char *str, int l_sign, LL *gamma) = 0;
};
