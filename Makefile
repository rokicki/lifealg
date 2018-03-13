CXXFLAGS = -O3 -march=native

lifealgo: *.cpp *.h
	g++ $(CXXFLAGS) -std=c++11 -o lifealgo *.cpp -lpthread
