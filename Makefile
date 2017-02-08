all:
	g++ main.cpp -std=c++11 -lpthread -o md5crack

clean:
	rm -rf *.o md5crack

