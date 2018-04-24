CXXFLAGS = -O3 -march=native -g -Wall -Wsign-compare
#CXXFLAGS = -g -Wall -Wsign-compare -march=native
#CXXFLAGS = -O3 -g -Wall -Wsign-compare

lifealgo: *.cpp *.h
	g++ -I lifelib $(CXXFLAGS) -std=c++11 -o lifealgo *.cpp -lpthread
