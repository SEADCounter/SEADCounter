#pragma once
#include <algorithm>
#include <cstring>
#include <string.h>
#include "bobhash.h"
#include <iostream>
#include <math.h>

using namespace std;
// recommended parameter settings in paper of "counter tree"

#define b 4
#define d 2
#define h 3
#define r0 100

class CT
{
private:
    BOBHash *bobhash[r0 + 20];
    int index[r0];
    int8_t counter[20000000];
    int m;
    //uint64_t hash_value;
public:
    CT(int _m)
    {
        m = _m;
        memset(counter, 0, sizeof(counter));

        for (int i = 0; i < r0; ++i)
        {
            bobhash[i] = new BOBHash(i + 1000);
        }
    }
    //insert function for int counters
    void Insert(const char *str)
    {
        int k = rand() % r0;
        index[k]=(bobhash[k]->run(str, strlen(str))) % m;
        if (counter[index[k]] == ((1 << b) - 1))
        {
            int upper_index = m;
            int low_layer_index = index[k];
            int temp = index[k];
            int i = 0;
            for (; i < h; i++)
            {
                counter[temp]++;
                if (counter[temp] >= (1 << b))
                {
                    counter[temp] -= (1 << b);
                    temp = (low_layer_index) / ((int)pow(d, i + 1)) + upper_index;
                    upper_index += m / ((int)pow(d, i));
                    
                }
                else
                    break;
            }
            if (i == h)
            {
                cout << "overflow!!";
            }
        }
        else
        {
            counter[index[k]]++;
        }
    }
    //query function for int counters
    // f_or_n: the sum of all flow sizes.
    int Query(const char *str, int f_or_n)
    {
        int val = 0;
        int k = ((int)pow(d, h - 1));
        int my_upper_index[h][k];
        for (int i = 0; i < r0; i++)
        {
            index[i] = (bobhash[i]->run(str, strlen(str))) % m;
        }
        
        for (int i = 0; i < r0; i++)
        {
            memset(my_upper_index, 0, k*h*sizeof(int));

            for (int j = 0; j < h - 1; ++j)
            {
                my_upper_index[0][0] += m / ((int)pow(d, j));
            }
            my_upper_index[0][0] += (index[i] / ((int)pow(d, h - 1)));
            //	cout << my_upper_index[0][0]<<endl;
            //	cout << (int)counter[my_upper_index[0][0]]<<endl;
            val += counter[my_upper_index[0][0]] << (b * (h - 1));
            for (int j = h - 2; j >= 0; --j)
            {
                int index_lim = ((int)pow(d, h - 1 - j));
                for (int l = 0; l < index_lim; ++l)
                {

                    my_upper_index[h - 1 - j][l] = my_upper_index[h - 2 - j][0] - m / ((int)pow(d, j)) - (index[i] / ((int)pow(d, j + 1))) + index[i] / ((int)pow(d, j)) + l;
                    //	cout << my_upper_index[h - 1 - j][l]<<endl;
                    val += counter[my_upper_index[h - 1 - j][l]] <<(b * j);
                    //	cout << val << endl;
                }
            }
        }
        double temp_ = (double)f_or_n * k * r0 / (double)m;
        return (val - temp_);
    }
    double get_memory()
    {
        double memory = 0;
        for (int i = 0; i < h; ++i)
        {
            memory += 0.5 * ((double)m / ((int)pow(d, i)));
        }
        return memory;
    }
    ~CT()
    {
        for (int i = 0; i < r0; ++i)
        {
            delete bobhash[i];
        }
    }
};

// CT_H
