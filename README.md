# SEAD Counter: Self-Adaptive Counters with Different Counting Ranges

## Introduction

The Sketch is a compact data structure useful for network measurements. However, to cope with the high speeds of the current data plane, it needs to be held in the small on-chip memory (SRAM). Therefore, the product of the counter size and the number of counters must be below a certain limit. With small counters, some will overflow. With large counters, the total number of counters will be small, but each counter will be shared by more flows, leading to poor accuracy. To address this issue, we propose a generic technique: *self-adaptive counters (SEAD Counter)*. When the value of the counter is small, it works as a standard counter. When the value of the counter is large however, we increment it using a predefined probability, so as to represent this large value. Moreover, in the SEAD Counter, the probability decreases when the value increases. We show that this technique can significantly improve the accuracy of counters. This technique can be adapted to different circumstances. We theoretically analyze the improvements achieved by the SEAD Counter. We further show that our SEAD Counter can be extended to three typical sketches and Bloom filters. We conduct extensive experiments on three real datasets and one synthetic dataset. The experimental results show that, compared with the state-of-the-art, sketches using the SEAD Counter improve the accuracy by up to 13.6 times, while the Bloom filters using SEAD Counter can reduce the false positive rate by more than one order of magnitude.


## About the source code, dataset
In the subfolder SEAD_Counter_CBF, the source code contains the C++ implementation of CBF, VI-CBF and SEAD CBF, SEAD-VI-CBF. In the subfoler SEAD_Counter_Sketch, the source code contains the C++ implementation of the CM, CU, C sketch and SEAD CM, SEAD CU, SEAD C sketch. Besides our algorithms, we also uploaded some other compared algorithms, including SAC, ICEBuckets and Counter-Tree. The codes have been compiled successfully using g++ 9.3.0 on Ubuntu 20.04. 

There are four kinds of datasets: IP trace datasets, datacenter dataset, synthetic dataset and kosarac dataset in our experiments. For the IP trace dataset, it is collected from CAIDA[1]. A five-tuple is used as the ID of a flow, which includes source IP address, destination IP address, source port, destination port, and protocol, 13 bytes in total.  The full dataset used in our experiments has been updated to Onedrive and can be downloaded from  <https://1drv.ms/u/s!AsNxYjNVnyK8g-EwlxyVv7Au8aj9ig?e=3gaGc8>. For the small file CAIDA.dat, after 13-bytes five-tuple, there are 2 bytes indicating the length of the read pakcet and one should modify the code relating to reading it. For the datacenter dataset, it is one datacenter trace offered by [2] and also has the format of 13-bytes five-tuple. The files in new_zipf, synthetic datasets, are following the Zipf distribution ($p(x)=\frac{x^{-a}}{\zeta(a)}$) with different total flow sizes $F$ (1M to 10M). Their filename indicates their different $a$. They also has the format of 13-bytes five-tuple  For the kosarc dataset, it can be downloaded from <http://fimi.ua.ac.be/data/> and its elements are integers separated by space.[3]

In our experiments, the packets read in our experiments are determined by their dataset size. For large dataset like CAIDA_large.dat and Datacenter.dat, we read 30M packets(they contains more packets than 30M) in our experiments; For small dataset like CAIDA.dat and synthetic dataset, we choose the maximum possible number of packets to read. We have compared our algorithms with other estimators. We also have compared our algorithms applied on sketches with the original sketches, Counter Tree, Pyramid Sketch, ICEBuckets and SAC on sketches. The parameter settings can be seen in our paper.


## How to build
In the two subfolders: SEAD_Counter_CBF and SEAD_Counter_Sketch, there is one Makefile separately, use ``` make``` and execute the output files.


[1] “The caida anonymized 2016 internet traces.” <http://www.caida.org/data/overview/>.

[2] T. Benson, A. Akella, and D. A. Maltz, “Network traffic characteristics of data centers in the wild,” in Proceedings of the 10th ACM SIGCOMM conference on Internet measurement, 2010, pp. 267–280.

[3]  "Real-life transactional dataset." <http://fimi.ua.ac.be/data/>.
