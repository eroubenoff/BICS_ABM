#include <string>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <sstream>
#include "defs.h"
using namespace std;


Node* Population::node(string id) {
	/* Searches for the node by id. Returns pointer to the node
	*  if it exists. If it doesn't exist, returns nullptr.
	*/

	auto loc = _nodelist.find(id);

	if (loc == _nodelist.end()) {
		return nullptr;
	}

	else {
		return &_nodelist.at(id);
	}
}

vector<string> Population::nodelist() const {
	/* Returns a list of the keys to the nodelist */
	vector<string> keys;

	for(auto kv : _nodelist) {
	    keys.push_back(kv.first);
	} 

	return keys;
}

Household* Population::household(string hhid) {
	auto loc = _hhlist.find(hhid);

	if (loc == _hhlist.end()) {
		return nullptr;
	}

	else {
		return &_hhlist.at(hhid);
	}
}

vector<string> Population::hhlist() const {
	/* Returns a list of the keys to the nodelist */
	vector<string> keys;

	for(auto kv : _hhlist) {
	    keys.push_back(kv.first);
	} 

	return keys;
}

void Population::add_node(Node n) {
	/* throws error if node already present
	Also adds hh
	*/

	string id = n.id();
	string hhid = n.hhid();

	if (node(n.id()) == nullptr) {
		_nodelist[n.id()] = n;
	}

	else {
		throw "Node already present in population";
		return;
	}

	// If node doesn't have a household, return without adding to household
	if (hhid == "") {
		return;
	}

	// If household doesn't exist, create it with the given hhid
	if (household(hhid) == nullptr) {
		_hhlist[hhid] = Household(vector<Node*> {node(id)});
		return;
	} else {
		// Add to household
		_hhlist[hhid].add_member(&_nodelist[n.id()]);

	}



	return;


}

Population::Population(vector<Node> nodelist) {
	/* Primary constructor. Pass a vector of nodes. 
	Constructor puts nodes into unordered map _nodelist
	hashed on node id.

	Also creates hashmap _hhlist from node ids.

	*/

	Node n;

	for (int i = 0; i < nodelist.size(); i++) {

		n = nodelist[i];

		// Check to make sure node isn't already in Population
		if (!node(n.id())) {
			add_node(n);
		}
	}
}

Population::Population() {

}


ostream & operator << (ostream &out, Population &p)
{
	out << "Population size: " << p.nodelist().size() << endl;

	vector<string> hhlist = p.hhlist();

	for (int i = 0; i < hhlist.size(); i++) {
		out << *p.household(hhlist[i]);
	}

    out << endl;

    return out;
}


// Function to create a population by reading a csv

int age_remap(string age) {
	if (age == "[0,18)") return 1;
    else if (age == "[18,25)") return 2;
    else if (age == "[25,35)") return 3;
    else if (age == "[35,45)") return 4;
    else if (age == "[45,55)") return 5;
    else if (age == "[55,65)") return 6;
    else if (age == "[65,75)") return 7;
    else if (age == "[75,85)") return 8;
    else if (age == ">85") return 9;
    else return 0;
}

bool gender_remap(string gender) {
	if (gender == "Female") {
		return true;
	} else return false;
}

vector<Node> pop_from_csv(string fname) {
	/*
	Creates a population by adding a node from each line of csv

	*/



	vector<Node> nodelist;

	ifstream in_file;
	in_file.open(fname); 
	// if (in_file.fail()) { return 0; } // Check for failure after opening


	// Constructs for parsing lines
	string line;
	stringstream linestream;
	string str;

	// Parse the first line of column headers
	vector<string> header;
	getline(in_file, line);
	linestream = stringstream(line);

	while(linestream.good()) {
		getline(linestream, str, ',');
		header.push_back(str);
	}

	// Now walk through each row creating an unordered map of column headers
	// called linemap

	while (in_file.good()) {
		unordered_map<string, string> linemap;

		// Get the line
		getline(in_file, line);
		linestream = stringstream(line);

		// Add each to the map
		int counter = 0;
		while (linestream.good()) {
			getline(linestream, str, ',');
			linemap[header[counter]] = str;
			counter++;
		}

		string id = linemap["id"];
		int age = age_remap(linemap["age"]);
		bool gender = gender_remap(linemap["gender"]);
		// string num_cc_s = linemap["num_cc"];
		// stringstream num_cc_ss(num_cc_s);
		int num_cc = 5;
		// num_cc_ss >> num_cc;
		string hhid = linemap["hhid"];

		Node n = Node(
			id,
			age,
			gender,
			num_cc,
			hhid
		);

		nodelist.push_back(n);
	}



	return nodelist;




}
