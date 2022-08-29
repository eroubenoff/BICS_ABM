// #include <vector>
#include <string>
using namespace std;

class Node {
    private:
        int _id;
        int _num_cc;
        int _num_days_sick;
        int _age;
        bool _gender; 
        // vector<int> _edges;
        string _disease_status;

    public:
        int id() const; 
        int age() const;
        bool gender() const;
        int num_cc() const;
        int num_days_sick() const;
        string disease_status() const;
        void set_sick(int n);
        void decrement();
        void recover();
        // void add_edge(int n);
        // void remove_edge(int n);
        // void clear_edges();
        Node();
        Node(int id, int age, bool gender, int num_cc);
        ~Node();
}; 