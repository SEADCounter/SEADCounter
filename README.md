# SEAD Counter: Self-Adaptive Counters with Different Counting Ranges

## Introduction

The Sketch is a compact data structure useful for network measurements. However, to cope with the high speeds of the current data plane, it needs to be held in the small on-chip memory (SRAM). Therefore, the product of the counter size and the number of counters must be below a certain limit. With small counters, some will overflow. With large counters, the total number of counters will be small, but each counter will be shared by more flows, leading to poor accuracy. To address this issue, we propose a generic technique: *self-adaptive counters (SEAD Counter)*. When the value of the counter is small, it works as a standard counter. When the value of the counter is large however, we increment it using a predefined probability, so as to represent this large value. Moreover, in the SEAD Counter, the probability decreases when the value increases. We show that this technique can significantly improve the accuracy of counters. This technique can be adapted to different circumstances. We theoretically analyze the improvements achieved by the SEAD Counter. We further show that our SEAD Counter can be extended to three typical sketches and Bloom filters. We conduct extensive experiments on three real datasets and one synthetic dataset. The experimental results show that, compared with the state-of-the-art, sketches using the SEAD Counter improve the accuracy by up to 13.6 times, while the Bloom filters using SEAD Counter can reduce the false positive rate by more than one order of magnitude.


## About the source code, dataset
In the subfolder SEAD_Counter_CBF, the source code contains the C++ implementation of CBF, VI-CBF and SEAD CBF, SEAD-VI-CBF. In the subfoler SEAD_Counter_Sketch, the source code contains the C++ implementation of the CM, CU, C sketch and SEAD CM, SEAD CU, SEAD C sketch. Besides our algorithms, we also uploaded some other compared algorithms, including SAC, ICEBuckets and Counter-Tree. The codes have been compiled successfully using g++ 9.3.0 on Ubuntu 20.04. 

The file CAIDA.dat is one IP trace collected from CAIDA[]. This small dataset ...  The full dataset can be downloaded from  <https://1drv.ms/u/s!AsNxYjNVnyK8g-EwlxyVv7Au8aj9ig?e=3gaGc8>.


We set the memory allocated to each sketch 1MB. 


## How to build
In the two subfolders: SEAD_Counter_CBF and SEAD_Counter_Sketch, there is one Makefile separately, using ...


