#include <string>
#include <iostream>
#include <random>
#include "defs.h"


using namespace std;

default_random_engine generator;

string randstring(int length) {
	
	string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

	uniform_int_distribution<int> distribution(0,str.length());

	string randstring;

	for (int i = 0; i < length; i++) {
		randstring.push_back(str[distribution(generator)]);
	}

	return randstring;

	}

string randstring() {
	return randstring(16);
}

