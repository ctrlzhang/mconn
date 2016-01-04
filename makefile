mconn:mconn.o common.o
	g++ -Wall -g -o mconn mconn.o common.o

mconn.o: mconn.cpp
	g++ -c mconn.cpp

common.o: common.cpp common.h message.h mqueue.h
	g++ -c common.cpp

.PHONY:clean
