#! /bin/bash
g++ -DUNIX -DKNET_UNIX -std=c++11 -c client.cpp -I/home/sasl/encad/pecheux/kNet-stable/include
g++ -o client client.o -L/home/sasl/encad/pecheux/kNet-stable/lib -lkNet -lpthread
