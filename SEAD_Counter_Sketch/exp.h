#pragma once

#include <iostream>
#include <math.h>
#include "sketch.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unordered_map>
#include <fstream>
#include "sac.h"
#include "cmsketch.h"
#include "cusketch.h"
#include "csketch.h"
#include "counter_tree.h"
#include "bobhash.h"
#include <limits>
#include <time.h>
#include <chrono>
using namespace std;






// three experiment functions: CAIDA_experiment, webpage_experiment and synthetic_experiment.
void CAIDA_experiment(int number, double& ret_error, int version, double n, int m, long double& sigma, int sketch, double& ret_aae, long double& aae_sig) {
	//n is the memory size(in KB) and m is the number of arrays used in the sketch
	//the total number(number*LOOP_NUM here) should not exceed the size of dataset.
	cout << "your sketch number is: " << sketch << " (0~5 for 'CM', 'CU', 'C', 'SACCM', 'SACCU' or 'SACC' sketches)" << endl;
	class_sketches* s[20];
	if (sketch == 0) {

		for (int i = 0; i < LOOP_NUM; ++i) {
			s[i] = new CMSketch(n * 1024 / 4 / m, m);
		}
		cout << "CMSketch with " << n << "KB hash memory generated!" << endl;

	}
	if (sketch == 1) {
		for (int i = 0; i < LOOP_NUM; ++i) {
			s[i] = new CUSketch(n * 1024 / 4 / m, m);
		}
		cout << "CUSketch with " << n << "KB hash memory generated!" << endl;
	}
	if (sketch == 2) {
		for (int i = 0; i < LOOP_NUM; ++i) {
			s[i] = new CSketch(n * 1024 / 4 / m, m);
		}
		cout << "CSketch with " << n << "KB hash memory generated!" << endl;
	}
	if (sketch == 3) {
		for (int i = 0; i < LOOP_NUM; ++i) {
			s[i] = new CMSketch(n * 1024 / 2 / m, m);
		}
		cout << "SAC CMSketch with " << n << "KB hash memory generated!" << endl;
	}
	if (sketch == 4) {
		for (int i = 0; i < LOOP_NUM; ++i) {
			s[i] = new CUSketch(n * 1024 / 2 / m, m);
		}
		cout << "SAC CUSketch with " << n << "KB hash memory generated!" << endl;
	}
	if (sketch == 5) {
		for (int i = 0; i < LOOP_NUM; ++i) {
			s[i] = new CSketch(n * 1024 / 2 / m, m);
		}
		cout << "SAC CSketch with " << n << "KB hash memory generated!" << endl;
	}


	unordered_map<string, int> unmp[20];

	
	int package_num = 0;
	int total_size;
	double error[20];
	double error_aae[20];
	char filename[35];
	memset(filename, 0, sizeof(char) * 15);

	sprintf(filename, "../dataset/CAIDA_large.dat");
	//only CAIDA.dat allowed, as read format is different from caida files in 60s.
	//  counter tree experiment doesn't support this experiment, as this dataset tells
	//  and counts the package length when each element is inserted.
	FILE* file_stream = fopen(filename, "rb");
	double time_count=0;
	for (int m = 0; m < LOOP_NUM; ++m) {	
		package_num = 0;
		total_size = 0;
		
		char buf[15];
		
		while (fread(buf, 1, 13, file_stream) && total_size < number) {
			string s(buf,13);
			unmp[m][s] ++;
			total_size ++;
		}

		cout<<"total size="<<total_size<<endl;
	
		int max_freq = 0;
		unordered_map<string, int>::iterator it = unmp[m].begin();

		for (int i = 0; i < unmp[m].size(); i++, it++)
		{
			

			int temp2 = it->second;

			max_freq = max_freq > temp2 ? max_freq : temp2;
		}

		//cout<<"loading complete and max_freq="<<max_freq<<endl;
		//cout<<"total size="<<total_size<<endl;
		//cout<<"package_number="<<package_num<<endl;
		//printf("unmp[%d].size: %d\n",m,unmp[m].size());
		unordered_map<string, int>::iterator it1;
		auto start=chrono::system_clock::now();
		for (it1 = unmp[m].begin(); it1 != unmp[m].end(); ++it1) {

			if (sketch <= 2){
				for(int i=0;i<it1->second;++i)
					s[m]->Insert(it1->first.c_str(), 1);
			}
			else if (!version&&sketch!=5) {
				for(int i=0;i<it1->second;++i)
					s[m]->dynamic_sac_insert(it1->first.c_str(), 1, gamma_2,false);
			}
			else if(!version&&sketch==5){
				for(int i=0;i<it1->second;++i)
					s[m]->dynamic_sac_insert(it1->first.c_str(), 1, gamma_2);
			}
		}
		
		auto end=chrono::system_clock::now();                                                                                  
		auto duration=chrono::duration_cast<chrono::microseconds>(end-start);
		time_count+=double(duration.count())*chrono::microseconds::period::num/chrono::microseconds::period::den;
		
	

		int flow_num = unmp[m].size();
		unordered_map<string, int>::iterator it2;

		error[m] = 0;
		error_aae[m] = 0;
		
		for (it2 = unmp[m].begin(); it2 != unmp[m].end(); ++it2) {
			if (sketch <= 2) {
				error[m] += abs(s[m]->Query(it2->first.c_str()) - it2->second) / (double)it2->second;
				error_aae[m] += abs((double)s[m]->Query(it2->first.c_str()) - it2->second);
			}
			else if (!version&&sketch!=5)
			{
				//printf("%d %d\n",s[m]->dynamic_sac_query(it2->first.c_str(), gamma_2),it2->second);
				error[m] += abs(s[m]->dynamic_sac_query(it2->first.c_str(), gamma_2,false) - it2->second) / (double)it2->second;
				error_aae[m] += abs((double)s[m]->dynamic_sac_query(it2->first.c_str(), gamma_2,false) - it2->second);
			}
			else if(!version&&sketch==5){
				error[m] += abs(s[m]->dynamic_sac_query(it2->first.c_str(), gamma_2) - it2->second) / (double)it2->second;
				error_aae[m] += abs((double)s[m]->dynamic_sac_query(it2->first.c_str(), gamma_2) - it2->second);
			}
		}
		error[m] /= flow_num;
		error_aae[m] /= flow_num;
	}
	fclose(file_stream);
	
	ret_error = 0;
	sigma = 0;
	ret_aae = 0;
	aae_sig = 0;
	for (int m = 0; m < LOOP_NUM; ++m) {
		ret_error += error[m];
		ret_aae += error_aae[m];
	}
	printf("average_time_count_for_sketch_number_%d:%f\n",sketch,time_count/LOOP_NUM);
	
	ret_error /= LOOP_NUM;
	ret_aae /= LOOP_NUM;
	for (int m = 0; m < LOOP_NUM; ++m) {
		sigma += pow(error[m] - ret_error, 2);
		aae_sig += pow(error_aae[m] - ret_aae, 2);
	}
	sigma = sqrt(sigma / (LOOP_NUM-1));
	aae_sig = sqrt(aae_sig / (LOOP_NUM-1));
}

void webpage_experiment(int number, double& ret_error, int version, double n, int m, long double& sigma, int sketch, double& ret_aae, long double& aae_sig) {
	//n is the memory size(in KB) and m is the number of arrays used in the sketch
	//the total number(number*LOOP_NUM here) should not exceed the size of dataset.
	cout << "your sketch number is: " << sketch;
	cout << " (0~6 for 'CM', 'CU', 'C','Counter Tree', 'SACCM', 'SACCU' or 'SACC' sketches)" << endl;
	class_sketches* s[20];
	CT* ct[20];
	if (sketch == 0) {

		for (int i = 0; i < LOOP_NUM; ++i) {
			s[i] = new CMSketch(n * 1024 / 4 / m, m);
		}
		cout << "CMSketch with " << n << "KB hash memory generated!" << endl;

	}
	if (sketch == 1) {
		for (int i = 0; i < LOOP_NUM; ++i) {
			s[i] = new CUSketch(n * 1024 / 4 / m, m);
		}
		cout << "CUSketch with " << n << "KB hash memory generated!" << endl;
	}
	if (sketch == 2) {
		for (int i = 0; i < LOOP_NUM; ++i) {
			s[i] = new CSketch(n * 1024 / 4 / m, m);
		}
		cout << "CSketch with " << n << "KB hash memory generated!" << endl;
	}
	if (sketch == 3) {
		double left = 0, right = 2 * n * 1024, mid = (left + right) / 2;
		while ((right - left) > 1e-5) {
			mid = (left + right) / 2;
			ct[0] = new CT(mid);
			if (ct[0]->get_memory() <= n * 1024)
				left = mid;
			else
				right = mid;
			delete ct[0];
		}
		for (int i = 0; i < LOOP_NUM; ++i) {
			ct[i] = new CT(mid);
		}
		cout << "Counter tree with " << n << "KB hash memory generated!" << endl;
	}
	if (sketch == 4) {
		for (int i = 0; i < LOOP_NUM; ++i) {
			s[i] = new CMSketch(n * 1024 / 2 / m, m);
		}
		cout << "SAC CMSketch with " << n << "KB hash memory generated!" << endl;
	}
	if (sketch == 5) {
		for (int i = 0; i < LOOP_NUM; ++i) {
			s[i] = new CUSketch(n * 1024 / 2 / m, m);
		}
		cout << "SAC CUSketch with " << n << "KB hash memory generated!" << endl;
	}
	if (sketch == 6) {
		for (int i = 0; i < LOOP_NUM; ++i) {
			s[i] = new CSketch(n * 1024 / 2 / m, m);
		}
		cout << "SAC CSketch with " << n << "KB hash memory generated!" << endl;
	}


	unordered_map<string, int> unmp[20];

	int package_num = 0;
	int total_size;
	double error[20];
	double error_aae[20];
	char filename[25];
	memset(filename, 0, sizeof(char) * 15);

	sprintf(filename, "../dataset/webpage.txt");

	
	ifstream file_read(filename, ios::in);
	char tmp_read[20];
	for (int m = 0; m < LOOP_NUM; ++m) {
		total_size = 0;
		while (file_read >> tmp_read && total_size < number) {
			unmp[m][(string)tmp_read] += 1;
			if (sketch == 3)
				ct[m]->Insert(tmp_read);
			if (sketch >= 4 && version)
				s[m]->static_sac_insert(tmp_read, 3, gamma_2);
			++total_size;
		}
		//cout << "total size=" << total_size << endl;
	}
	double time_count=0;
	for (int m = 0; m < LOOP_NUM; ++m) {
		int max_freq = 0;
		unordered_map<string, int>::iterator it = unmp[m].begin();

		for (int i = 0; i < unmp[m].size(); i++, it++)
		{
			int temp2 = it->second;
			max_freq = max_freq > temp2 ? max_freq : temp2;
		}
		//cout << "loading complete and max_freq=" << max_freq << endl;
		//cout<<"total size="<<total_size<<endl;
		unordered_map<string, int>::iterator it1;
		auto start=chrono::system_clock::now();
		for (it1 = unmp[m].begin(); it1 != unmp[m].end(); ++it1) {
			if (sketch <= 2)
				s[m]->Insert(it1->first.c_str(), it1->second);
			else if (sketch>=4&&!version)
				s[m]->dynamic_sac_insert(it1->first.c_str(), it1->second, gamma_2);
		}
		auto end=chrono::system_clock::now();                                                                                  
		auto duration=chrono::duration_cast<chrono::microseconds>(end-start);
		time_count+=double(duration.count())*chrono::microseconds::period::num/chrono::microseconds::period::den;
		
		int flow_num = unmp[m].size();
		unordered_map<string, int>::iterator it2;

		error[m] = 0;
		error_aae[m] = 0;
		for (it2 = unmp[m].begin(); it2 != unmp[m].end(); ++it2) {
			if (sketch <= 2) {
				//cout << s[m]->Query(it2->first.c_str()) <<(double)it2->second << endl;
				error[m] += abs(s[m]->Query(it2->first.c_str()) - it2->second) / (double)it2->second;
				error_aae[m] += abs((double)s[m]->Query(it2->first.c_str()) - it2->second);
			}
			else if (sketch == 3) {
				error[m] += abs(ct[m]->Query(it2->first.c_str(), number) - it2->second) / (double)it2->second;
				error_aae[m] += abs((double)ct[m]->Query(it2->first.c_str(), number) - it2->second);
			}
			else if (!version)
			{
				error[m] += abs(s[m]->dynamic_sac_query(it2->first.c_str(), gamma_2) - it2->second) / (double)it2->second;
				error_aae[m] += abs((double)s[m]->dynamic_sac_query(it2->first.c_str(), gamma_2) - it2->second);
			}
			else {
				error[m] += abs(s[m]->static_sac_query(it2->first.c_str(), 3, gamma_2) - it2->second) / (double)it2->second;
				error_aae[m] += abs((double)s[m]->static_sac_query(it2->first.c_str(), 3, gamma_2) - it2->second);
			}
		}
		error[m] /= flow_num;
		error_aae[m] /= flow_num;
	}
	printf("time_count_for_sketch_number_%d:%f\n",sketch,time_count);
	ret_error = 0;
	sigma = 0;
	ret_aae = 0;
	aae_sig = 0;
	for (int m = 0; m < LOOP_NUM; ++m) {
		ret_error += error[m];
		ret_aae += error_aae[m];
	}

	ret_error /= LOOP_NUM;
	ret_aae /= LOOP_NUM;
	for (int m = 0; m < LOOP_NUM; ++m) {
		sigma += pow(error[m] - ret_error, 2);
		aae_sig += pow(error_aae[m] - ret_aae, 2);
	}
	sigma = sqrt(sigma / (LOOP_NUM-1));
	aae_sig = sqrt(aae_sig / (LOOP_NUM-1));
	if (sketch == 3) {
		for (int i = 0; i < LOOP_NUM; ++i)
			delete ct[i];
	}
	else if (sketch >= 4) {
		for (int i = 0; i < LOOP_NUM; ++i)
			delete s[i];
	}
}


void synthetic_experiment(int number, double& ret_error, int version, int a_zipf, double n, int m, long double& sigma, int sketch, double& ret_aae, long double& aae_sig) {
	//a is the skewness parameter of Zipf distribution, n is the memory size(in KB) and m is the number of arrays used in the sketch
	//the total number(number*LOOP_NUM here) should not exceed the size of dataset.
	cout << "your sketch number is: " << sketch;
	cout << " (0~6 for 'CM', 'CU', 'C','Counter Tree', 'SACCM', 'SACCU' or 'SACC' sketches)" << endl;
	class_sketches* s[20];
	CT* ct[20];
	if (sketch == 0) {

		for (int i = 0; i < LOOP_NUM; ++i) {
			s[i] = new CMSketch(n * 1024 / 4 / m, m);
		}
		cout << "CMSketch with " << n << "KB hash memory generated!" << endl;

	}
	if (sketch == 1) {
		for (int i = 0; i < LOOP_NUM; ++i) {
			s[i] = new CUSketch(n * 1024 / 4 / m, m);
		}
		cout << "CUSketch with " << n << "KB hash memory generated!" << endl;
	}
	if (sketch == 2) {
		for (int i = 0; i < LOOP_NUM; ++i) {
			s[i] = new CSketch(n * 1024 / 4 / m, m);
		}
		cout << "CSketch with " << n << "KB hash memory generated!" << endl;
	}
	if (sketch == 3) {
		double left = 0, right = 2 * n * 1024, mid = (left + right) / 2;
		while ((right - left) > 1e-5) {
			mid = (left + right) / 2;
			ct[0] = new CT(mid);
			if (ct[0]->get_memory() <= n * 1024)
				left = mid;
			else
				right = mid;
			delete ct[0];
		}
		for (int i = 0; i < LOOP_NUM; ++i) {
			ct[i] = new CT(mid);
		}
		cout << "Counter tree with " << n << "KB hash memory generated!" << endl;
	}
	if (sketch == 4) {
		for (int i = 0; i < LOOP_NUM; ++i) {
			s[i] = new CMSketch(n * 1024 / 2 / m, m);
		}
		cout << "SAC CMSketch with " << n << "KB hash memory generated!" << endl;
	}
	if (sketch == 5) {
		for (int i = 0; i < LOOP_NUM; ++i) {
			s[i] = new CUSketch(n * 1024 / 2 / m, m);
		}
		cout << "SAC CUSketch with " << n << "KB hash memory generated!" << endl;
	}
	if (sketch == 6) {
		for (int i = 0; i < LOOP_NUM; ++i) {
			s[i] = new CSketch(n * 1024 / 2 / m, m);
		}
		cout << "SAC CSketch with " << n << "KB hash memory generated!" << endl;
	}


	unordered_map<string, int> unmp[20];

	int package_num = 0;
	int total_size = 0;
	double error[20];
	double error_aae[20];
	char filename[25];
	memset(filename, 0, sizeof(char) * 15);

	sprintf(filename, "../dataset/%d.txt", a_zipf);

	ifstream file_read(filename, ios::in);
	char tmp_read[20];
	for (int m = 0; m < LOOP_NUM; ++m) {
		total_size = 0;
		while (file_read >> tmp_read && total_size < number) {
			unmp[m][(string)tmp_read] += 1;
			if (sketch == 3)
				ct[m]->Insert(tmp_read);
			if (sketch>=4&&version)
				s[m]->static_sac_insert(tmp_read, 3, gamma_2);
			++total_size;
		}
		//cout<<"total size="<<total_size<<endl;
	}
	double time_count=0;
	for (int m = 0; m < LOOP_NUM; ++m) {
		int max_freq = 0;
		unordered_map<string, int>::iterator it = unmp[m].begin();

		for (int i = 0; i < unmp[m].size(); i++, it++)
		{
			int temp2 = it->second;
			max_freq = max_freq > temp2 ? max_freq : temp2;
		}
		//cout << "loading complete and max_freq=" << max_freq << endl;
		//cout<<"total size="<<total_size<<endl;
		unordered_map<string, int>::iterator it1;
		auto start=chrono::system_clock::now();
		for (it1 = unmp[m].begin(); it1 != unmp[m].end(); ++it1) {
			if (sketch <= 2)
				s[m]->Insert(it1->first.c_str(), it1->second);
			if (sketch >= 4 && !version)
				s[m]->dynamic_sac_insert(it1->first.c_str(), it1->second, gamma_2);
		}
		auto end=chrono::system_clock::now();                                                                                  
		auto duration=chrono::duration_cast<chrono::microseconds>(end-start);
		time_count+=double(duration.count())*chrono::microseconds::period::num/chrono::microseconds::period::den;
		
		int flow_num = unmp[m].size();
		unordered_map<string, int>::iterator it2;

		error[m] = 0;
		error_aae[m] = 0;
		for (it2 = unmp[m].begin(); it2 != unmp[m].end(); ++it2) {
			if (sketch <= 2) {
				error[m] += abs(s[m]->Query(it2->first.c_str()) - it2->second) / (double)it2->second;
				error_aae[m] += abs((double)s[m]->Query(it2->first.c_str()) - it2->second);
			}
			else if (sketch == 3)
			{
				error[m] += abs(ct[m]->Query(it2->first.c_str(), number) - it2->second) / (double)it2->second;
				error_aae[m] += abs((double)ct[m]->Query(it2->first.c_str(), number) - it2->second);
			}
			else if (!version)
			{
				error[m] += abs(s[m]->dynamic_sac_query(it2->first.c_str(), gamma_2) - it2->second) / (double)it2->second;
				error_aae[m] += abs((double)s[m]->dynamic_sac_query(it2->first.c_str(), gamma_2) - it2->second);
			}
			else {
				error[m] += abs(s[m]->static_sac_query(it2->first.c_str(), 3, gamma_2) - it2->second) / (double)it2->second;
				error_aae[m] += abs((double)s[m]->static_sac_query(it2->first.c_str(), 3, gamma_2) - it2->second);
			}
		}
		error[m] /= flow_num;
		error_aae[m] /= flow_num;
	}
	printf("time_count_for_sketch_number_%d:%f\n",sketch,time_count);
	ret_error = 0;
	sigma = 0;
	ret_aae = 0;
	aae_sig = 0;
	for (int m = 0; m < LOOP_NUM; ++m) {
		ret_error += error[m];
		ret_aae += error_aae[m];
	}

	ret_error /= LOOP_NUM;
	ret_aae /= LOOP_NUM;
	for (int m = 0; m < LOOP_NUM; ++m) {
		sigma += pow(error[m] - ret_error, 2);
		aae_sig += pow(error_aae[m] - ret_aae, 2);
	}
	sigma = sqrt(sigma / (LOOP_NUM-1));
	aae_sig = sqrt(aae_sig / (LOOP_NUM-1));
	if (sketch == 3) {
		for (int i = 0; i < LOOP_NUM; ++i)
			delete ct[i];
	}
	else if (sketch >= 4) {
		for (int i = 0; i < LOOP_NUM; ++i)
			delete s[i];
	}

}
