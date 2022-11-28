run: CMakeLists.txt build/
	clear
	cmake . -B build/ 
	cmake --build build/ 
	./build/BICS_ABM

python: CMakeLists.txt build/
	clear
	cmake . -B build/ 
	cmake --build build/ 
	python3 py_extension.py

xcode: CMakeLists.txt build/
	clear
	cmake . -B build/ 
	cmake --build build/ 
	cmake ./ -GXcode

test: CMakeLists.txt build/
	clear
	cmake . -B build/ 
	cmake --build build/ 
	./build/BICS_ABM_tests
	


clean:
	rm *.o 
	cmake --build build/ --target clean
