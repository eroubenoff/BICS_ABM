igraph: igraph_test.cpp CMakeLists.txt build/
	clear
	cmake . -B build/
	cmake --build build/
	./build/igraph_test


Population_tests: Population_tests.cpp Population.cpp Household.cpp Node.cpp randstring.cpp 
	clear; g++ -std=c++11 Population_tests.cpp Population.cpp Household.cpp Node.cpp randstring.cpp -o Population_tests ; ./Population_tests

Household_tests: Household_tests.cpp Household.cpp Node.cpp randstring.cpp 
	clear; g++ -std=c++11 Household_tests.cpp Household.cpp Node.cpp randstring.cpp -o Household_tests ; ./Household_tests

Node_tests: Node_tests.cpp Node.cpp randstring.cpp 
	clear; g++ -std=c++11 Node_tests.cpp Node.cpp randstring.cpp -o Node_tests ; ./Node_tests

clean:
	rm *.o 
	cmake --build build/ --target clean