CXXFLAGS = -O3 -march=native -g -Wall -Wsign-compare
#CXXFLAGS = -g -Wall -Wsign-compare

lifealgo: *.cpp *.h
	g++ -I lifelib $(CXXFLAGS) -std=c++11 -o lifealgo *.cpp -lpthread
