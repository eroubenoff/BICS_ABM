igraph: CMakeLists.txt build/
	clear
	cmake . -B build/ 
	cmake --build build/ 
	# cmake ./ -GXcode
	./build/BICS_ABM

debug : igraph_test.cpp CMakeLists.txt build/
	clear
	cmake . -B build/ -DCMAKE_BUILD_TYPE=Debug # -DCMAKE_CXX_COMPILER=g++
	cmake --build build/ -DCMAKE_BUILD_TYPE=Debug# -DCMAKE_CXX_COMPILER=g++
	lldb ./build/BICS_ABM

igraph_nocmake: BICS_ABM.cpp BICS_ABM.h get_csv_row.cpp print_attributes.cpp \
	disease_status.cpp decrement.cpp set_sick.cpp \
	transmit.cpp create_pop.cpp randstring.cpp
	g++ BICS_ABM.cpp BICS_ABM.h get_csv_row.cpp print_attributes.cpp \
		disease_status.cpp decrement.cpp set_sick.cpp \
		transmit.cpp create_pop.cpp randstring.cpp \
		-I/usr/local/include/igraph -L/usr/local/lib -ligraph \
		-std=c++17 
	


clean:
	rm *.o 
	cmake --build build/ --target clean
