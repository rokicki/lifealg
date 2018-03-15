CXXFLAGS = -g -Wall -O3 -Wsign-compare

lifealgo: *.cpp *.h
	g++ -I lifelib $(CXXFLAGS) -std=c++11 -o lifealgo *.cpp -lpthread
