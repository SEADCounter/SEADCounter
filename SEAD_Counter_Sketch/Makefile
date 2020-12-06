GCC = g++
CFLAG = -O3 -std=c++11 

FILE =  a.out estimator.out estimator_on_sketch.out


all: $(FILE)
.PHONY: all

a.out: main.cpp 
	$(GCC) $(CFLAG) -D COUNTER_SIZE=16 -D per_estimator_int=16 -D per_estimator_bit=16.5 -o a.out main.cpp

estimator.out: estimator_main.cpp
	$(GCC) $(CFLAG) -D COUNTER_SIZE=12 -D per_estimator_int=12 -D per_estimator_bit=12.5 -o estimator.out estimator_main.cpp

estimator_on_sketch.out: estimator_on_sketch.cpp
	$(GCC) $(CFLAG) -D COUNTER_SIZE=12 -D per_estimator_int=12 -D per_estimator_bit=12.5 -o estimator_on_sketch.out estimator_on_sketch.cpp

clean:
	rm $(all) -f *~ *.o *.out
