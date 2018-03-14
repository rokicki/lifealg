CXXFLAGS = -g -O3

lifealgo: *.cpp *.h
	g++ -I lifelib $(CXXFLAGS) -std=c++11 -o lifealgo *.cpp -lpthread
