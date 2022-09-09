#include <string>
#include <iostream>
#include "defs.h"
using namespace std;

// Main contains unit tests
int main() {
    Node n("121", 26, false, 10);

    cout << "Expecting 49: " << n.id() << endl;
    if (n.id() != "121") throw "Incorrect value for id";

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



    cout << "\n\n\n Testing random node id generation" << endl;
    

    n = Node(26, false, 10); 
    cout << n.id() << endl;

    n = Node(26, false, 10); 
    cout << n.id() << endl;

    n = Node(26, false, 10); 
    cout << n.id() << endl;


    // Testing overloaded <<
    cout << n; 


    return 1;
}