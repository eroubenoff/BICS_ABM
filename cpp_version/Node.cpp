// #include <stdio.h> 
#include <string>
#include <iostream>
// #include <vector>
// #include <stdlib.h>  
#include "Node.h"

using namespace std;

int Node::id() const {
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
Node::Node(int id, int age, bool gender, int num_cc) {
    _id = id;
    _age = age;
    _gender = gender;
    _num_cc = num_cc;
    _num_days_sick = 0;
    _disease_status = "S";
};
Node::~Node() {};



// Main contains unit tests
int main() {
    Node n(49, 26, false, 10);

    cout << "Expecting 49: " << n.id() << endl;
    if (n.id() != 49) throw "Incorrect value for id";

    cout << "Expecting 26: " << n.age() << endl;
    if (n.age() != 26) throw "Incorrect value for age";

    cout << "Expecting 0: " << n.gender() << endl;
    if (n.gender() != 0) throw "Incorrect value for gender";

    cout << "Expecting 10: " << n.num_cc() << endl;
    if (n.num_cc() != 10) throw "Incorrect value for num_cc";

    cout << "Expecting S: " << n.disease_status() << endl;
    if (n.disease_status() != "S") throw "Incorrect value for disease_status";
    
    n.set_sick(10);
    cout << "Setting sick... " << endl;
    cout << "Expecting I: " << n.disease_status() << endl;
    if (n.disease_status() != "I") throw "Incorrect value for disease_status"; 

    cout << "Expecting 10: " << n.num_days_sick() << endl;
    if (n.num_days_sick() != 10) throw "Incorrect value num_days_sick"; 

    n.decrement();
    cout << "Decrementing... " << endl;
    cout << "Expecting 9: " << n.num_days_sick() << endl;
    if (n.num_days_sick() != 9) throw "Incorrect value num_days_sick"; 

    cout << "Decrementing to 0" << endl;

    while (n.disease_status() == "I") {
        n.decrement();
        cout << "Num days remaining sick: " << n.num_days_sick() << endl;;  
    }
    cout << "Expecting R: " << n.disease_status(); 
    if (n.disease_status() != "R") throw "Incorrect value for disease_status"; 

    return 1;
}
