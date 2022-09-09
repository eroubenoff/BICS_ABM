#include <string>
#include <vector>
#include <unordered_map>
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
