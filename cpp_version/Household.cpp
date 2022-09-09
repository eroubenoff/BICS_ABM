#include <string>
#include <iostream>
#include <vector>
#include "defs.h"
using namespace std;


string Household::hhid() const {
	return _hhid;
}
int Household::hhsize() const {
	return _hhsize;
}
string Household::head() const {
	return _head;
}
vector<Node*> Household::memberlist() const {
	return _memberlist;
}
void Household::add_member(Node *r) {
	// Make sure that node either has no hhid or one that matches

	if (r -> hhid() == "") {
		r -> set_hhid(_hhid);
	}
	if (r -> hhid() != _hhid) {
		throw runtime_error("Node hhid doesn't match household hhid");
	}


	if (_hhsize == 0) {
		_head = r -> id();
	}

	_memberlist.push_back(r);
	_hhsize = _memberlist.size();


}

Household::Household(){
	_hhid = randstring();
	_hhsize = 0;
	_head = "";
	vector<Node*> _memberlist;
};

Household::Household(string hhid){
	_hhid = hhid;
	_hhsize = 0;
	_head = "";
	vector<Node*> _memberlist;
};

Household::Household(vector<Node*> memberlist) {
	/* Passing a vector of nodes automatically assume that
	household head is the first node */

	// Check to see if all nodes have the same hhid
	string tmp_hhid = memberlist[0] -> hhid();
	for (int i = 0; i < memberlist.size(); i++) {
		if (tmp_hhid != memberlist[i] -> hhid()) {
			throw runtime_error("Error in HH constructor: all nodes must have same hhid");
		}
	}

	if (tmp_hhid == "") {
		tmp_hhid = randstring();

		for (int i = 0; i < memberlist.size(); i++) {
			memberlist[i] -> set_hhid(tmp_hhid); 
		}
	}

	_hhid = tmp_hhid;
	_memberlist = memberlist;
	_head = memberlist[0] -> id();
	_hhsize = memberlist.size();
}

Household::~Household(){};

ostream & operator << (ostream &out, const Household &h)
{
    out << "Household id: " << h.hhid() << " | "; 
    out << "Size : " << h.hhsize() << endl;

    // Loop through each member
    for (int i = 0; i < h.hhsize(); i++){

    	Node* member = h.memberlist()[i];

    	if (member -> id() == h.head()) {
    		out << "Member " << i << " (head of household): ";
    		out << *member;
    	}
    	else {
    		out << "Member " << i << ": ";
    		out << *member;
    	}
    }

    out << endl;

    return out;
}

