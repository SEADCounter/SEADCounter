#include <iostream>
#include <iomanip>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unordered_map>
#include <fstream>
#include "bobhash.h"
#include "new_exp.h"
#include "sketch.h"
#include "cbf-sead.h"
#include "vicbf-sead.h"
#include <limits>
#include <utility>
#include <time.h>
#include "sead.h"

using namespace std;



int main()
{
	greeting();
	string dataset;
	cout<<"To test SEAD on different datasets,"<<endl;
    cout<<"Type 'CAIDA','webpage', 'kosarak' or 'synthetic'"<<endl;
	cin >> dataset;
	double start, ends;
	int sketch, arr, version = 0;
	int name;
	double err, sigma;
	double bits_in_elements;
	// bits used per element are calculated in the result file, 
	//as different dataset have different number of elements.

	cout << "type in the range of memory usage(in KB)(ex: from 300 to 1500)" << endl;
	cout << "from:" << endl;
	cin >> start;
	cout << "to:" << endl;
	cin >> ends;
	cout << "type 0~3 for 'CBF','VI-CBF','SEAD-CBF', or 'SEAD-VI-CBF' sketches" << endl;
	cin >> sketch;

	if (dataset == "CAIDA") {
		arr = 12;
		ofstream file;
		char FILE_NAME_TEMPLATE[100] = "../output/output_cbf/caida_%d.txt";
		char file_name[100];
		sprintf(file_name, FILE_NAME_TEMPLATE, sketch);
		file.open(file_name);
		file << "Bits_in_elements(in Bit) FPP" << endl;
		for (int l = start; l < ends; l += (ends - start) / 12) {
			CAIDA_experiment(sketch, version, arr, l, bits_in_elements, err);
			file << setiosflags(ios::fixed) << setprecision(2);
			file << bits_in_elements << resetiosflags(ios::fixed);
			file << "  " << err << endl;
		}
	}
	else if (dataset == "webpage") {
		arr = 12;
		ofstream file;
		char FILE_NAME_TEMPLATE[100] = "../output/output_cbf/webpage_%d.txt";
		char file_name[100];
		sprintf(file_name, FILE_NAME_TEMPLATE, sketch);
		file.open(file_name);
		file << "Bits_in_elements(in Bit) FPP" << endl;
		for (int l = start; l < ends; l += (ends - start) / 12) {
			webpage_experiment(sketch, version, arr, l, bits_in_elements, err);
			file << setiosflags(ios::fixed) << setprecision(2);
			file << bits_in_elements << resetiosflags(ios::fixed);
			file << "  " << err << endl;
		}
	}
	else if (dataset == "synthetic") {
		double a;
		for (int i = 11; i <= 20; i++) {
			arr = 12;
			a = 1.0 * i / 10;
			ofstream file;
			char FILE_NAME_TEMPLATE[100] = "../output/output_cbf/synthetic_%d_%d.txt";
			char file_name[100];
			sprintf(file_name, FILE_NAME_TEMPLATE, sketch, i);
			file.open(file_name);
			file << "Bits_in_elements(in Bit) FPP" << endl;
			for (int l = start; l < ends; l += (ends - start) / 12) {
				synthetic_experiment(sketch, version, arr, i, l, bits_in_elements, err);
				file << setiosflags(ios::fixed) << setprecision(2);
				file << bits_in_elements << resetiosflags(ios::fixed);
				file << "  " << err << endl;
			}
		}
	}
	else if (dataset == "kosarak") {
		arr = 12;
		ofstream file;
		char FILE_NAME_TEMPLATE[100] = "../output/output_cbf/kosarak_%d.txt";
		char file_name[100];
		sprintf(file_name, FILE_NAME_TEMPLATE, sketch);
		file.open(file_name);
		file << "Bits_in_elements(in Bit) FPP" << endl;
		for (int l = start; l < ends; l += (ends - start) / 12) {
			kosarak_experiment(sketch, version, arr, l, bits_in_elements, err);
			file << setiosflags(ios::fixed) << setprecision(2);
			file << bits_in_elements << resetiosflags(ios::fixed);
			file << "  " << err << endl;
		}
	}
	else {
		cout << "No such dataset exists! Please type in datasets we generate above or modify our functions and use sketches we write here." << endl;
	}
	return 0;
}

/* sketch: which sketch to use
 * version: dynamic or static SEAD
 * arr: arrays in sketch
 * start -> ends: range of memory use
 * l : memory use
 * err: false positive proportion
 * caida, webpage
 *
 * synthetic: i: a of zipf
 *
 */
