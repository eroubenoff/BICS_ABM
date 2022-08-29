#include <stdio.h> 
#include <string>
#include <iostream>
#include <vector>
using namespace std;
using std::string;


class Population {
    private: 
        vector<Node> _nodearray;
        int _nodecount;
        int _S_count;
        int _I_count;
        int _R_count;

    public:
        void add_node(Node n) {
            _nodearray.push_back(n);
            _nodecount = _nodearray.size();
            return;
        };

        Node get_node(int i) {
            if (i > _nodecount) {
                throw "Node index out of bounds";
            }
            return _nodearray[i];
        };

        void update_counts() {
            _N_count = 0;
            _S_count = 0;
            _I_count = 0;
            _R_count = 0;
            string status;

            for (int i = 0; i < _nodearray.size(); i++) {
                status = _nodearray[i].get_status(); 
                if (status == "S") {
                    _S_count += 1;
                } 
                else if (status == "I") {
                    _I_count += 1;
                }
                else if (status == "R") {
                    _R_count +=1;
                }
                else continue;
            };

            if (_N_count != _S_count + _I_count + _R_count) {
                throw "SIR does not sum to N";
            };
        };

        Population(){};
        Population(vector<Node> nodearray) {
            _nodearray = nodearray;
        };

        ~Population(){};

};

int main() {

    // Create a single node
    Node n = Node(5, 10);
    cout << &n << endl;
    cout << n.id() << endl;
    cout << n.num_cc() << endl;
    cout << n.num_days_sick() << endl;

    // Create a population

    Population p1;
    p1.add_node(n);
    p1.get_node(0);


    return(0);
}
