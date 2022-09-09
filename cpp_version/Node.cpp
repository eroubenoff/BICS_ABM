// #include <stdio.h> 
#include <string>
#include <iostream>
#include "defs.h"

using namespace std;

string Node::id() const {
    return _id;
}
int Node::age() const {
    return _age;
}
bool Node::gender() const {
    return _gender;
}
int Node::num_cc() const {
    return _num_cc;
}
int Node::num_days_sick() const {
    return _num_days_sick;
}
string Node::disease_status() const {
    return _disease_status;
}
string Node::hhid() const {
    return _hhid;
}
void Node::set_hhid(string hhid) {
    _hhid = hhid;
    return;
}
void Node::set_hhid() {
    _hhid = randstring();
    return;
}
void Node::set_sick(int n) {
    _disease_status = "I";
    _num_days_sick = n;
    return;
}
void Node::decrement() {
    _num_days_sick -= 1;

    if (_num_days_sick == 0) { 
        recover();
    }
    return;
}
void Node::recover() {
    _disease_status = "R";
    return;
}

void Node::add_edge(string id) {
    _edges.push_back(id);
    return;
}

void Node::remove_edge(string id) {

    // Find by id; swap with last value, then pop back
    int pos = 0;
    while (pos < _edges.size() ) {
        if (_edges[pos] == id) {
            
            _edges[pos] = _edges[_edges.size()];
            _edges.pop_back();

            return; 
        }
    }

    throw "id " + id + " not found in nodelist";
    return;
}

void Node::clear_edges() {
    while (_edges.size() > 0) {
        _edges.pop_back();
    }
}


// void Node::add_edge(int n) {
//     _edges.push_back(n);
//     return;
// }
// void Node::remove_edge(int n) {
//     _edges.pop(n);
//     return;
// }
// void Node::clear_edges() {
//     _edges.clear();
//     return;
// }        
Node::Node() {};

Node::Node(int age, bool gender, int num_cc) {
    _id = randstring();
    _age = age;
    _gender = gender;
    _num_cc = num_cc;
    _num_days_sick = 0;
    _disease_status = "S";
    _hhid = "";
};

Node::Node(string id, int age, bool gender, int num_cc) {
    _id = id;
    _age = age;
    _gender = gender;
    _num_cc = num_cc;
    _num_days_sick = 0;
    _disease_status = "S";
    _hhid = "";
};

Node::~Node() {};

ostream & operator << (ostream &out, const Node &n)
{
    out << "Node id: " << n.id() << " in memory location " << &n << " | "; 
    out << "Household id: " << n.hhid() << " | ";
    out << "Age: " << n.age() << " | ";
    out << "Gender: " << (n.gender() ? "Female" : "Male") << " | " ;
    out << "Disease status: " << n.disease_status(); 
    out << endl;

    return out;
}

