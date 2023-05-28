#include <string>
#include <vector>
#include <igraph.h>
#include <map>
#include <tuple>
#include <iostream>
using namespace std;
/* 
 * Class of differences between graphs
 * 
 */
class Diff {
    private: 
        string _type, _attribute;
        int _v0, _v1, _idx; 
    public:
        string type() const {return _type;};
        string attribute() const {return _attribute;};
        int idx() const {return _idx;};
        int v0() const {return _v0;};
        int v1() const {return _v1;};
        Diff(string type, string attribute, int v0, int v1,  int idx = -1) {
            _type = type;
            _attribute = attribute;
            _v0 = v0;
            _v1 = v1;
            _idx = idx;
        }
        Diff(){};
        string print() const {
            string ret = "";
            ret = ret + "Attribute diff in: " + _type + ", attribute name: " + _attribute+  ", value 0: " + to_string(_v0) +  ", value 1: " + to_string(_v1);
            if (_idx != -1) {
                ret = ret + ", index: " + to_string(_idx);
            }

            return ret;
        }
    
};

class DiffList {
    private:
        vector<Diff> _difflist;
        int n_diffs;

        map<string, vector<Diff>> type_hash;
        map<tuple<string, int>, vector<Diff>> id_hash;
        map<tuple<string, string>, vector<Diff>> attribute_hash;
        map<tuple<string, string, int>, Diff> all_hash; 

        void hash_diffs() {
            string type_key;
            tuple<string, int> id_key;
            tuple<string, string> attribute_key;
            tuple<string, string, int> all_key;

            for (auto &i: _difflist) {
                // Hash on type
                type_key = i.type();
                type_hash[type_key].push_back(i);

                // Hash on type and id
                id_key = make_tuple(i.type(), i.idx());
                id_hash[id_key].push_back(i);

                // Hash on type and attribute
                attribute_key = make_tuple(i.type(), i.attribute());
                attribute_hash[attribute_key].push_back(i);

                // Hash on all 
                all_key = make_tuple(i.type(), i.attribute(), i.idx());
                all_hash[all_key] = i;
            }
        }
    public:
        // Constructor from vector
        DiffList(vector<Diff> difflist) {
            _difflist = difflist;
            n_diffs = _difflist.size();
            hash_diffs();
        }

        // Default constructor
        DiffList(){};

        // Add a single diff
        void add_diff(Diff diff) {
            _difflist.push_back(diff);
            n_diffs = _difflist.size();
            hash_diffs();
        }

        // Get all
        vector<Diff> get() const {
            return _difflist;
        }

        // Get by type
        vector<Diff> get(string type) const {
            try {
                return type_hash.at(type);
            } catch (out_of_range &e) {
                throw out_of_range("No Diff of type " + type + " found ");
            } 
        };
        
        // Get by type and id
        vector<Diff> get(string type, int idx) const {
            try {
                return id_hash.at(make_tuple(type, idx));
            } catch (out_of_range &e) {
                throw out_of_range("No Diff of type " + type + " for idx  " + to_string(idx) + " found ");
            } 
        }

        // Get by type and attribute
        vector<Diff> get(string type, string attribute) const {
            try{
                return attribute_hash.at(make_tuple(type, attribute));
            } catch (out_of_range &e) {
                throw out_of_range("No Diff of type " + type + " for attribute " + attribute + " found ");
            } 
        }
        // Get by type attribute and id
        Diff get(string type, string attribute, int idx) const {
            try{
                return all_hash.at(make_tuple(type, attribute, idx));
            } catch (out_of_range &e) {
                throw out_of_range("No Diff of type " + type + " for attribute " + attribute + " for idx  " + to_string(idx) +  " found ");
            } 
        }


};
DiffList graph_diff(igraph_t *g1, igraph_t *g2); 


/* Various test graphs */ 
void create_test_graphs(igraph_t *g) ;
