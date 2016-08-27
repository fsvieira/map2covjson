all:
	g++ -o map2covjson map2covjson.cpp -lgdal -g
	
test:
	g++ -o projection_tests projection_tests.cpp -lgdal -g

