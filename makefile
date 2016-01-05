mconn:mconn.o common.o timeout_handler.o share_mem.o
	g++ -Wall -g -o mconn share_mem.o mconn.o common.o timeout_handler.o

mconn.o:mconn.cpp mconn.h
	g++ -c mconn.cpp

common.o:common.cpp common.h message.h mqueue.h
	g++ -c common.cpp

timeout_handler.o:timeout_handler.cpp timeout_handler.h
	g++ -c timeout_handler.cpp

share_mem.o:share_mem.cpp share_mem.h
	g++ -c share_mem.cpp

clean:
	rm *.o

.PHONY:clean
