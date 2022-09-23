#include <string>
#include <iostream>
#include "defs.h"
using namespace std;

// Main contains unit tests
int main() {

	// Create a set of nodes to put into households
	Node n1 = Node(20, true, 1);
	Node n2 = Node(30, false, 10);
	Node n3 = Node(40, false, 13);
	Node n4 = Node(50, true, 5);
	Node n5 = Node(60, true, 2);

	// Assemble one household by passing a vector of nodes
	Household h1 = Household(vector<Node*>{&n1, &n2, &n3});

	// Assemble the other by growing one by one
	Household h2 = Household();
	h2.add_member(&n4); 
	h2.add_member(&n5); 


	// Check 
	cout << h1.hhid() << " | " << h2.hhid() << &endl;
	cout << h1.hhsize() << " | " << h2.hhsize() << &endl;
	cout << h1.head() << " | " << h2.head() << &endl;
	cout << h1.memberlist()[0] << " | " << h2.memberlist()[0] << &endl;

	cout << h1 << h2 << endl;


	return 0;
}
