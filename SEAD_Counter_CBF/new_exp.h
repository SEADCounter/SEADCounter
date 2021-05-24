#pragma once

#include <iostream>
#include <math.h>
#include "sketch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unordered_map>
#include <unordered_set>
#include <fstream>
#include <sstream>
#include "sead.h"
#include "bobhash.h"
#include "cbf-sead.h"
#include "vicbf-sead.h"
#include <limits>
#include <time.h>
#include <utility>
using namespace std;
char insert[9999998][20];
char five_tuple[9999998][15];
int package_length[9999998];
char query[9999998][20];
double memory = 1.0;
int w = memory * 100000;

double re[810];
int counter[810];

// four experiment functions: CAIDA_experiment, kosarak_experiment, webpage_experiment and synthetic_experiment.
void CAIDA_experiment(int sketch, int version, int arr, int l, double& bits_in_elements, double& err) {
	//n is the memory size(in KB) and m is the number of arrays used in the sketch
	// this experiment uses CAIDA files in the folder '60s'
	cout << "your sketch number is: " << sketch << " (0~3 for 'CBF', 'VI-CBF', 'SEADCBF', or 'VI-SEADCBF')" << endl;
	int m = 12;
	int n = l;
	class_sketches* s[20];
	if (sketch == 0) {

		for (int i = 0; i < 12; ++i) {
			s[i] = new CBF(n * 1024 / 4 / m, m);
		}
		cout << "CBF with " << n << "KB hash memory generated!" << endl;

	}
	if (sketch == 1) {
		for (int i = 0; i < 12; ++i) {
			s[i] = new VI_CBF(n * 1024 / 4 / m, m);
		}
		cout << "VI-CBF with " << n << "KB hash memory generated!" << endl;
	}
	if (sketch == 2) {
		for (int i = 0; i < 12; ++i) {
			s[i] = new CBF(n * 1024 / 2 / m, m);
		}
		cout << "SEAD CBF with " << n << "KB hash memory generated!" << endl;
	}
	if (sketch == 3) {
		for (int i = 0; i < 12; ++i) {
			s[i] = new VI_CBF(n * 1024 / 2 / m, m);
		}
		cout << "SEAD VI-CBF with " << n << "KB hash memory generated!" << endl;
	}

	
	double fpp[20];
	int number = 250000;// number of elements read from the dataset
	bits_in_elements = 8 * 1024 * (double)n / number;
	int total_count = 0;
	char filename[35];
	memset(filename, 0, sizeof(char) * 35);
	sprintf(filename, "../dataset/caida/0.dat");
	FILE* fin = fopen(filename, "rb");
	unordered_map<string, uint32_t> unmp[20];

	for (int k = 0; k < 10; k++) {
		unmp[k].clear();
		uint8_t key[13];
		total_count = 0;
		int t;
		while ((t = fread(key, 1, 13, fin)) == 13 && total_count < number) {
			string string_key((const char*)key, 13);
			unmp[k][string_key]++;
			total_count++;
		}
	}


	fclose(fin);
	//cout << "output kth query result after insertion as k_fpp;" << endl;
	for (int k = 0; k < 10; k++) {

		unordered_map<string, uint32_t>::iterator it1 = unmp[k].begin();
		for (it1 = unmp[k].begin(); it1 != unmp[k].end(); ++it1) {
			if (sketch <= 1)
				s[k]->Insert(it1->first.c_str(), it1->second);
			else {
				s[k]->dynamic_sead_insert(it1->first.c_str(), it1->second, gamma_2);
			}
		}
		// read random sequence

		fpp[k] = 0;
		FILE* fin2 = fopen("../dataset/rdm_str.txt", "rb");
		char str_test[20];
		int ch;
		while (fgets(str_test, 17, fin2) != NULL)
		{
			string string_key((const char*)str_test, 12);
			//cout<<string_key<<endl;
			if (sketch <= 1) {
				if (s[k]->Query(string_key.c_str()) != 0)
					fpp[k] += 1;
			}
			else{
				if (s[k]->dynamic_sead_query(string_key.c_str(), gamma_2) != 0)
					fpp[k] += 1;
			}
			fgetc(fin2);
		}
		fclose(fin2);
		fpp[k] /= 2000001;
		//printf("%d_%lf;",k,fpp[k]);
	}
	//cout << endl;
	err = 0;
	for (int k = 0; k < 10; ++k) {
		err += fpp[k];
	}
	err = err / 10;
}

void kosarak_experiment(int sketch, int version, int arr, int l, double& bits_in_elements, double& err) {
	//n is the memory size(in KB) and m is the number of arrays used in the sketch
	cout << "your sketch number is: " << sketch << " (0~3 for 'CBF', 'VI-CBF', 'SEADCBF', or 'VI-SEADCBF')" << endl;
	int m = 3;
	int n = l;
	class_sketches* s;
	if (sketch == 0) {
		s = new CBF(n * 1024 / 4 / m, m);
		cout << "CBF with " << n << "KB hash memory generated!" << endl;
	}
	if (sketch == 1) {
		s = new VI_CBF(n * 1024 / 4 / m, m);
		cout << "VI-CBF with " << n << "KB hash memory generated!" << endl;
	}
	if (sketch == 2) {
		s = new CBF(n * 1024 / 2 / m, m);
		cout << "SEAD CBF with " << n << "KB hash memory generated!" << endl;
	}
	if (sketch == 3) {
		s = new VI_CBF(n * 1024 / 2 / m, m);
		cout << "SEAD VI-CBF with " << n << "KB hash memory generated!" << endl;
	}

	double fpp;
	int number = 41270;
	bits_in_elements = 8 * 1024 * (double)n / number;
	int total_count = 0;
	unordered_map<string, uint32_t> unmp;
	unmp.clear();
	total_count = 0;

	std::ifstream file("../dataset/kosarak.dat");
    std::string line;
    while(std::getline(file, line))
    {
        std::stringstream lineStream(line);
        int value;
        while(lineStream >> value)
        {
            unmp[to_string(value)]++;
			total_count++;
        }
    }
	//cout << "output kth query result after insertion as k_fpp;" << endl;


	unordered_map<string, uint32_t>::iterator it1 = unmp.begin();
	for (it1 = unmp.begin(); it1 != unmp.end(); ++it1) {
		if (sketch <= 1)
			s->Insert(it1->first.c_str(), it1->second);
		else {
			s->dynamic_sead_insert(it1->first.c_str(), it1->second, gamma_2);
		}
	}
	// read random sequence
	fpp = 0;
	int rand_m = 2538939;
	int rand_e = rand_m + 2000000;
	for (int i = rand_m; i <= rand_e; ++i) {
		string tmp = to_string(i);
		if (sketch <= 1) {
			if (s->Query(tmp.c_str()) != 0)
				fpp += 1;
		}
		else  {
			if (s->dynamic_sead_query(tmp.c_str(), gamma_2) != 0)
				fpp += 1;
		}
	}
	fpp /= 2000001;
	err = fpp;
}

void webpage_experiment(int sketch, int version, int arr, int l, double& bits_in_elements, double& err) {
	//n is the memory size(in KB) and m is the number of arrays used in the sketch
	int n = l;
	int m = 12;
	cout << "your sketch number is: " << sketch << " (0~3 for 'CBF', 'VI-CBF', 'SEADCBF', or 'VI-SEADCBF')" << endl;
	class_sketches* s[20];
	if (sketch == 0) {

		for (int i = 0; i < 12; ++i) {
			s[i] = new CBF(n * 1024 / 2 / m, m);
		}
		cout << "CBF with " << n << "KB hash memory generated!" << endl;

	}
	if (sketch == 1) {
		for (int i = 0; i < 12; ++i) {
			s[i] = new VI_CBF(n * 1024 / 2 / m, m);
		}
		cout << "VI-CBF with " << n << "KB hash memory generated!" << endl;
	}
	if (sketch == 2) {
		for (int i = 0; i < 12; ++i) {
			s[i] = new CBF(n * 1024 / m, m);
		}
		cout << "SEAD CBF with " << n << "KB hash memory generated!" << endl;
	}
	if (sketch == 3) {
		for (int i = 0; i < 12; ++i) {
			s[i] = new VI_CBF(n * 1024 / m, m);
		}
		cout << "SEAD VI-CBF with " << n << "KB hash memory generated!" << endl;
	}

	
	double fpp[20];
	int number = 80000;// number of elements read from the dataset
	bits_in_elements = 8 * 1024 * (double)n / number;

	int total_size = 0;



	unordered_map<string, int> unmp[20];
	char filename[30];
	memset(filename, 0, sizeof(char) * 15);

	sprintf(filename, "../dataset/webpage.txt");

	ifstream file_read(filename, ios::in);

	string tmp_read;
	for (int k = 0; k < 10; k++) {
		for (int z = 1; z <= number; z++) {
			tmp_read = to_string(z);
			unmp[k][string(tmp_read)] += 1;
			++total_size;
		}
	}

	//cout << "output kth query result after insertion as k_fpp;" << endl;
	for (int k = 0; k < 10; k++) {
		unordered_map<string, int>::iterator it1 = unmp[k].begin();

		for (it1 = unmp[k].begin(); it1 != unmp[k].end(); ++it1) {
			if (sketch <= 1)
				s[k]->Insert(it1->first.c_str(), it1->second);
			else  {
				s[k]->dynamic_sead_insert(it1->first.c_str(), it1->second, gamma_2);
			}
		}
		// read random sequence

		fpp[k] = 0;
		int rand_m = 2538939;
		int rand_e = rand_m + 2000000;
		for (int i = rand_m; i <= rand_e; ++i) {
			string tmp = to_string(i);
			if (sketch <= 1) {
				if (s[k]->Query(tmp.c_str()) != 0)
					fpp[k] += 1;
			}
			else  {
				if (s[k]->dynamic_sead_query(tmp.c_str(), gamma_2) != 0)
					fpp[k] += 1;
			}
		}
		fpp[k] = fpp[k] / 2000001;
	}
	err = 0;
	for (int k = 0; k < 10; ++k) {
		err += fpp[k];
	}
	err = err / 10;
}

void synthetic_experiment(int sketch, int version, int arr, int i, int l, double& bits_in_elements, double& err) {
	//a is the skewness parameter of Zipf distribution, n is the memory size(in KB) and m is the number of arrays used in the sketch
	cout << "your sketch number is: " << sketch << " (0~3 for 'CBF', 'VI-CBF', 'SEADCBF', or 'VI-SEADCBF')" << endl;
	int m = 12;
	int n = l;
	int a_zipf = i;
	class_sketches* s[20];
	if (sketch == 0) {

		for (int i = 0; i < 12; ++i) {
			s[i] = new CBF(n * 1024 / 4 / m, m);
		}
		cout << "CBF with " << n << "KB hash memory generated!" << endl;

	}
	if (sketch == 1) {
		for (int i = 0; i < 12; ++i) {
			s[i] = new VI_CBF(n * 1024 / 4 / m, m);
		}
		cout << "VI-CBF with " << n << "KB hash memory generated!" << endl;
	}
	if (sketch == 2) {
		for (int i = 0; i < 12; ++i) {
			s[i] = new CBF(n * 1024 / 2 / m, m);
		}
		cout << "SEAD CBF with " << n << "KB hash memory generated!" << endl;
	}
	if (sketch == 3) {
		for (int i = 0; i < 12; ++i) {
			s[i] = new VI_CBF(n * 1024 / 2 / m, m);
		}
		cout << "SEAD VI-CBF with " << n << "KB hash memory generated!" << endl;
	}


	unordered_map<string, int> unmp[20];


	
	int package_num = 0;
	int total_size = 0;
	double fpp[20];

	char filename[150];
	memset(filename, 0, sizeof(char) * 150);
	sprintf(filename, "../dataset/%d.txt", a_zipf);
	int number = 100000;// number of elements read from the dataset
	bits_in_elements = 8 * 1024 * (double)n / number;

	ifstream file_read(filename, ios::in);


	for (int k = 0; k < 10; ++k) {

		char tmp_read[20];

		total_size = 0;
		while (file_read >> tmp_read && total_size < number) {
			unmp[k][string(tmp_read)] += 1;
			++total_size;
		}
	}

	for (int k = 0; k < 10; ++k) {

		fpp[k] = 0;

		unordered_map<string, int>::iterator it1 = unmp[k].begin();

		for (it1 = unmp[k].begin(); it1 != unmp[k].end(); ++it1) {
			if (sketch <= 1)
				s[k]->Insert(it1->first.c_str(), it1->second);
			else  {
				s[k]->dynamic_sead_insert(it1->first.c_str(), it1->second, gamma_2);
			}
		}

		int rand_m = 2538939;
		int rand_e = rand_m + 2000000;
		for (int i = rand_m; i <= rand_e; ++i) {
			string tmp = to_string(i);
			if (sketch <= 1) {
				if (s[k]->Query(tmp.c_str()) != 0)
					fpp[k] += 1;
			}
			else {
				if (s[k]->dynamic_sead_query(tmp.c_str(), gamma_2) != 0)
					fpp[k] += 1;
			}
		}
		fpp[k] = fpp[k] / 2000001;
	}
	err = 0;
	for (int k = 0; k < 10; ++k) {
		err += fpp[k];
	}
	err = err / 10;
}
