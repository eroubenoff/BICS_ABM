#include <string>
#include <iostream>
#include <vector>
#include <unordered_map>
using namespace std;

class Node {
    private:
        string _id;
        int _num_cc;
        int _num_days_sick;
        int _age;
        bool _gender; 
        string _hhid;
        vector<string> _edges;
        string _disease_status;

    public:
        string id() const; 
        int age() const;
        bool gender() const;
        int num_cc() const;
        int num_days_sick() const;
        string disease_status() const;
        string hhid() const;
        void set_hhid(string hhid);
        void set_hhid();
        void set_sick(int n);
        void decrement();
        void recover();
        void add_edge(string id);
        void remove_edge(string id);
        void clear_edges();
        Node();
        Node(int age, bool gender, int num_cc);
        Node(string id, int age, bool gender, int num_cc);
        ~Node();

        friend ostream & operator << (ostream &out, const Node &n);
}; 


class Edge{
private:
	string _id;
	Node *_n1;
	Node *_n2;

public:
	string id() const;
	string n1() const;
	string n2() const;

	Edge(Node* n1, Node* n2);

};

class Household{
private:
	string _hhid;
	int _hhsize;
	string _head;
	vector<Node*> _memberlist;

public:
	string hhid() const;
	int hhsize() const;
	string head() const;
	vector<Node*> memberlist() const;
	void add_member(Node *r); 
	Household();
    Household(string hhid);
	~Household();

	// Fully parameterized constructor
	Household(vector<Node*> memberlist);

    friend ostream & operator << (ostream &out, const Household &h);

};




string randstring(int length);
string randstring();



class Population{
private:
    // All nodes are stored in the master nodelist, hashed on Node.id
    unordered_map<string, Node> _nodelist;
    // Households contain pointers to nodes in nodelist
    unordered_map<string, Household> _hhlist;
    // Edges contain pointers to nodes in nodelist
    // unordered_map<string, Edge> _edgelist;

    int _num_S;
    int _num_I;
    int _num_R;

public:
    // Accessors
    vector<string> nodelist() const; // Gets full list of node ids
    Node* node(string id) ; // Gets a single node by id

    vector<string> hhlist() const; // Gets the full list of household keys
    Household* household(string hhid) ; // Gets a single household by id

    // vector<Edge> edgelist(); // Gets the full list of edges
    // Edge edge(string id); // Gets a single edge by id

    // Mutators
    void add_node(Node n);
    // void add_Node(vector<Node> n);
    // void add_Household(Household h);
    // void add_Household(vector<Node> h);
    // void add_edge(Edge e);
    // void add_edge(Node n1, Node n2);

    // void remove_Node(string n);
    // void remove_Household(string h);
    // void remove_edge(string  e);
    // void remove_edge(Node n1, Node n2);

    // Population();
    Population(vector<Node> nodelist);


    friend ostream & operator << (ostream &out, Population &p);
};