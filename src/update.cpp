#include <igraph.h>
#include "BICS_ABM.h"
#include <vector>
#include <string>
#include <iostream>
#include <unordered_set>

using namespace std;

/*
 * Update: includes a change to the graph. An update should normally
 * concern a single change to a vertex or edge, or a change to the graph
 * 
 * Valid types of updates:
 * - Change vertex attribute
 * - Change edge attribute
 * - Create/destroy new edge
 * - Change graph attribute
 */




// Base Class member functions


// VertexUpdate functions
VertexUpdate::VertexUpdate(int vid) {
    _vid = vid;
}
int VertexUpdate::get_vid() {
    return _vid;
} 

// EdgeUpdate functions
int EdgeUpdate::get_v1() {
    return _v1;
}
int EdgeUpdate::get_v2() {
    return _v2;
}
int EdgeUpdate::get_eid() { 
    return _eid;
}
bool EdgeUpdate::eid_or_vpair() {
    return _eid_or_vpair;
}
bool EdgeUpdate::break_on_failure() {
    return _break_on_failure;
}
void EdgeUpdate::retrieve_eid(igraph_t* g) {

    if ((_v1 == -1 ) || (_v2 == -1)) return;
    if (_eid > -1) return;

    igraph_integer_t eid;
    igraph_get_eid(g, &eid, _v1, _v2, false, false);
    _eid = eid;

    if ((_eid == -1) & (_break_on_failure) ) {
        // print_attributes(g);
        throw invalid_argument("Error called from DeleteEdge::retrieve_eid: No edge between " + to_string(_v1) + ", " + to_string(_v2) + " present in graph");
    }
}
void EdgeUpdate::retrieve_endpoints(igraph_t* g) {
    if (_eid == -1) return; 
    if (_v1 > -1  && _v2 > -1) return;
    if (_eid > igraph_ecount(g)) {
        if (_break_on_failure) {
            // print_attributes(g);
            throw invalid_argument("Error called from DeleteEdge::retrieve_endoints: Edge id " + to_string(_eid) + " not present in graph");
        }
        else {
            _v1 = -1;
            _v2 = -1;
        }
    } else {
        _v1 = IGRAPH_FROM(g, _eid);
        _v2 = IGRAPH_TO(g, _eid);
        if (_v2 < _v1) {swap(_v1, _v2);}
    }
}


// AttributeUpdate functions
AttributeUpdate::AttributeUpdate(string attr, double value) {
    _attr = attr;
    _value = value;
}
string AttributeUpdate::get_attr() {
    return _attr;
} 
double AttributeUpdate::get_value() {
    return _value;
}


// Derived classes
//
// GraphUpdate functions
/*
UpdateGraphAttribute::UpdateGraphAttribute(string attr, double value) {
    _attr = attr;
    _value = value;
}

*/
// Edge functions
EdgeUpdate::EdgeUpdate(int v1, int v2, bool break_on_failure){
    if (v2 < v1) {swap(v1, v2);}
    _v1 = v1;
    _v2 = v2;
    _eid = -1;
    _eid_or_vpair = 1;
    _break_on_failure = break_on_failure;
}
EdgeUpdate::EdgeUpdate(int eid, bool break_on_failure){
    _v1 = -1;
    _v2 = -1;
    _eid = eid;
    _eid_or_vpair = 0;
    _break_on_failure = break_on_failure;
}
/*
CreateEdge::CreateEdge(int v1, int v2) {
    if (v2 < v1) {swap(v1, v2);}
    _v1 = v1;
    _v2 = v2;
    _eid = -1;
    _eid_or_vpair = 1;
    _break_on_failure = 1;
}

DeleteEdge::DeleteEdge(int v1, int v2, bool break_on_failure) {
    if (v2 < v1) {swap(v1, v2);}
    _v1 = v1;
    _v2 = v2;
    _eid = -1;
    _eid_or_vpair = 1;
    _break_on_failure = break_on_failure;
}
DeleteEdge::DeleteEdge(int eid, bool break_on_failure) {
    _v1 = -1;
    _v2 = -1;
    _eid = eid;
    _eid_or_vpair = 0;
    _break_on_failure = break_on_failure;
}
*/

/*
UpdateEdgeAttribute::UpdateEdgeAttribute(int eid, string attr, double value) {
    _v1 = -1;
    _v2 = -1;
    _eid = eid;
    _eid_or_vpair = 0;
    _break_on_failure = 1;
    _attr = attr;
    _value = _value;
}
UpdateEdgeAttribute::UpdateEdgeAttribute(int v1, int v2, string attr, double value): EdgeUpdate(v1, v2), AttributeUpdate(attr, value){
    if (v2 < v1) {swap(v1, v2);}
    _v1 = v1;
    _v2 = v2;
    _eid = -1;
    _eid_or_vpair = 1;
    _break_on_failure = 1;
    // _attr = attr;
    // _value = _value;
    cout << "Edge between: " << v1 << " and " << v2 << ". Attr: " << _attr << " value: " << _value << endl;
}
*/

/*
void UpdateEdgeAttribute::retrieve_eid(igraph_t* g) {
    if (_eid_or_vpair == 1) {
        igraph_integer_t eid;
        igraph_get_eid(g, &eid, _v1, _v2, false, false);
        _eid = eid;

        if ((_eid == -1 ) & (_break_on_failure) ) {
            print_attributes(g);
            throw invalid_argument("Error called from UpdateEdgeAttribute::retrieve_eid: No edge between " + to_string(_v1) + ", " + to_string(_v2) + " present in graph");
        }
    }
}
void UpdateEdgeAttribute::retrieve_endpoints(igraph_t* g) {
    if (_eid_or_vpair == 0) {
        // Check if eid is in g
        if (_eid > igraph_ecount(g)) {
            if (_break_on_failure) {
                print_attributes(g);
                throw invalid_argument("Error called from UpdateEdgeAttribute::retrieve_endoints: Edge id " + to_string(_eid) + " not present in graph");
            }
            else {
                _v1 = -1;
                _v2 = -1;
            }
        } else {
            _v1 = IGRAPH_FROM(g, _eid);
            _v2 = IGRAPH_TO(g, _eid);
            if (_v2 < _v1) {swap(_v1, _v2);}
        }
    }
}
*/

// Vertex attribute
// 
/*
UpdateVertexAttribute::UpdateVertexAttribute(int vid, string attr, double value) {
    _vid = vid;
    _attr = attr;
    _value = value;
}
*/

/*
 * List of updates
 *
 * Needs to be grown one at a time using the
 * overloaded add_update() function
 *
 */
UpdateList::UpdateList() {
        _update_graph_attribute_v.reserve(100);
        _create_edge_v.reserve(10000);
        _delete_edge_v.reserve(10000);
        _update_edge_attribute_v.reserve(10000);
        _update_vertex_attribute_v.reserve(10000);
}
void UpdateList::add_update(UpdateGraphAttribute update) {
    _update_graph_attribute_v.emplace_back(update);
}
void UpdateList::add_update(CreateEdge update) {
    _create_edge_v.emplace_back(update);
}
void UpdateList::add_update(DeleteEdge update) {
    _delete_edge_v.emplace_back(update);
}
void UpdateList::add_update(UpdateEdgeAttribute update) {
    _update_edge_attribute_v.emplace_back(update);
}
void UpdateList::add_update(UpdateVertexAttribute update) {
    _update_vertex_attribute_v.emplace_back(update);
}
void UpdateList::clear_updates() {
    _update_graph_attribute_v.clear();
    _create_edge_v.clear();
    _delete_edge_v.clear();
    _update_edge_attribute_v.clear();
    _update_vertex_attribute_v.clear();

    _update_graph_attribute_v.reserve(100);
    _create_edge_v.reserve(10000);
    _delete_edge_v.reserve(10000);
    _update_edge_attribute_v.reserve(10000);
    _update_vertex_attribute_v.reserve(10000);
}
// Helper functions to add_update_to_graph: lookup fns
// These are sort of cheating because instead of hashing
// the index of each attribute name they are just hard
// coded. But, it is faster, and we need speed now.
//
inline int vattr_lookup_fn(string name){
    char first_char = name[0];
    if ((first_char == 'd' ) && (name == "disease_status") ) return 8;
    else if ((first_char == 'r') && (name== "remaining_days_exposed") ) return 9;
    else if ((first_char == 'r') && (name== "remaining_days_sick") ) return 10;
    else if ((first_char == 't') && (name== "time_until_v2") )  return 11;
    else if ((first_char == 't') && (name== "time_until_vw") ) return 12;
    else if ((first_char == 't') && (name== "time_until_vboost") )  return 13;
    else if ((first_char == 't') && (name== "time_until_susceptible") )  return 14;
    else if ((first_char == 's') && (name== "symptomatic") ) return 15;
    else if ((first_char == 'v') && (name== "vaccine_status") ) return 16;
    else if ((first_char == 'm') && (name== "mu") ) return 17;
    else if ((first_char == 'h') && (name== "home_status") ) return 18;

    else if (name == "hhid") {
        return 0;
    }
    else if (name == "age") {
        return 1;
    }
    else if (name == "gender") {
        return 2;
    }
    else if (name == "num_cc_nonhh") {
        return 3;
    }
    else if (name == "num_cc_school") {
        return 4;
    }
    else if (name == "lefthome_num") {
        return 5;
    }
    else if (name == "vaccine_priority") {
        return 6;
    } 
    else if (name == "NPI"){
        return 7;
    }
    else {
        throw runtime_error("In Update: invalid vertex attribute lookup name: " + name);
    }

}
inline int eattr_lookup_fn(string name) {
    char first_char = name[0];
    if ((first_char == 't' ) && (name== "type") ) return 0;
    else if ((first_char == 'd' ) && (name== "duration"))  return 1;
    else {
        throw runtime_error("In Update: invalid edge attribute lookup name: " + name);
    }
}
void UpdateList::add_updates_to_graph(igraph_t *g) {
    // cout << _updates() << endl;
    igraph_vector_int_t gtypes, vtypes, etypes;
    igraph_strvector_t gnames, vnames, enames;
    igraph_vector_int_init(&gtypes, 0);
    igraph_vector_int_init(&vtypes, 0);
    igraph_vector_int_init(&etypes, 0);
    igraph_strvector_init(&gnames, 0);
    igraph_strvector_init(&vnames, 0);
    igraph_strvector_init(&enames, 0);

    igraph_cattribute_list(g,
            &gnames, &gtypes,
            &vnames, &vtypes,
            &enames, &etypes);



    // Begin with graph attributes
    // Don't bother with pulling the vectors of attributes
    // here. There usually aren't that many graph updates
    // so it makes sense to just do them individually.
    if (_update_graph_attribute_v.size() > 0) {
        for (auto &u: _update_graph_attribute_v) {
            SETGAN(g, u.get_attr().c_str(), u.get_value());
        }
    }

    // cout << "Getting endpoints for delete edge vector" << endl;
    for (int i = 0; i < _delete_edge_v.size(); i++) {
        _delete_edge_v[i].retrieve_endpoints(g);
    }
    // cout << "Getting endpoints for update edge attribute vector" << endl;
    for (int i = 0; i < _update_edge_attribute_v.size(); i++) {
        _update_edge_attribute_v[i].retrieve_endpoints(g);
    }

    igraph_vector_int_t edges_to_delete_v;
    igraph_vector_int_init(&edges_to_delete_v, 0);
    igraph_vector_int_reserve(&edges_to_delete_v, 2*_delete_edge_v.size() + 100);
    // Delete any edges (do this by end points)
    if (_delete_edge_v.size() > 0 ) {
        igraph_es_t edges_es;

        for (auto u: _delete_edge_v) {
            // cout << "vid1: " << to_string(u.get_v1()) << " vid2: " << to_string(u.get_v2()) << " eid: " << to_string(u.get_eid()) << endl;
            if ((u.get_v1() == -1) | (u.get_v2() == -1) | (u.get_eid() == -1)) {
                if (u.break_on_failure()) {
                    throw invalid_argument("Error called from UpdateList::add_updates_to_graph: No edge between " + 
                            to_string(u.get_v1() ) + ", " + to_string(u.get_v2()) + " present in graph");
                }

            } else {
                if (u.get_v1() == u.get_v2()) continue;

                // cout << u.get_v1() << "  " << u.get_v2() << endl;

                igraph_vector_int_push_back(&edges_to_delete_v, u.get_v1());

                // cout << "Done..." << endl;
                igraph_vector_int_push_back(&edges_to_delete_v, u.get_v2());

                // cout << "Done!" << endl;
           }
        }

        // for (int i = 0; i < igraph_vector_int_size(&edges_to_delete_v); i++) cout << VECTOR(edges_to_delete_v)[i] << endl;

        igraph_es_pairs(&edges_es, &edges_to_delete_v, false);
        igraph_delete_edges(g, edges_es);
        igraph_es_destroy(&edges_es);

    }
    igraph_vector_int_destroy(&edges_to_delete_v);


    igraph_vector_int_t new_edges;
    igraph_vector_int_init(&new_edges, 0);
    igraph_vector_int_reserve(&new_edges, 2*_create_edge_v.size()+100);

    // Create any needed edges
    if (_create_edge_v.size() > 0) {

        for (auto &u: _create_edge_v) {
            // cout << u.get_v1() << "  " << u.get_v2() << endl;
            igraph_vector_int_push_back(&new_edges, u.get_v1());
            igraph_vector_int_push_back(&new_edges, u.get_v2());
        }
        
        igraph_add_edges(g, &new_edges, NULL);
    }
    igraph_vector_int_destroy(&new_edges);

    // Update edge attributes
    // First make sure we have all edge ids. This step
    // has to be done by edge id, where the delete step can 
    // be done by end points.
    bool connected;
    for (int i = _update_edge_attribute_v.size(); i--;) {
        // Need to check that each edge we're updating is still
        // in the graph and wasn't deleted above

        igraph_are_connected(g, _update_edge_attribute_v[i].get_v1(), _update_edge_attribute_v[i].get_v2(), &connected); 
        if (!connected) {
            _update_edge_attribute_v.erase(_update_edge_attribute_v.begin() + i);
        }
        _update_edge_attribute_v[i].retrieve_eid(g);
    }


    igraph_vector_t eattr_v;
    igraph_vector_init(&eattr_v, 0);
    igraph_vector_reserve(&eattr_v, _update_edge_attribute_v.size()+100);

    if (_update_edge_attribute_v.size() > 0) {
        // To do this we first need to collect 
        // all of the attribute updates into a map
        // Can do this step by eid

        /*
        vector<string> eupdate_types;
        eupdate_types.reserve(_update_vertex_attribute_v.size());

        for (auto &i: _update_edge_attribute_v) {
            // eattrs.insert(i.get_attr());
            eupdate_types.push_back(i.get_attr());
        }

        vector<string> enames_v(eupdate_types);
        sort(enames_v.begin(), enames_v.end());
        vector<string>::iterator it;
        it = unique(enames_v.begin(), enames_v.end());
        enames_v.resize(distance(enames_v.begin(),it));

        unordered_map<string,int> enames_map;
        for (int i = 0; i < enames_v.size(); i++) {
            enames_map.insert({enames_v[i], i}); // {STR(enames, i), i});
        }
        */

        vector<string> enames_v = {"type", "duration"};
        vector<vector<int>> eattr_lookup(enames_v.size(), vector<int>(0));
        for (int i = 0; i < eattr_lookup.size(); i++) {
            eattr_lookup[i].reserve(1000);
        }
        string attrname;
        int eid;
        int attridx;
        for (int j = 0; j < _update_edge_attribute_v.size(); j++) {
            attrname = _update_edge_attribute_v[j].get_attr();
            eid = _update_edge_attribute_v[j].get_eid();
            attridx = eattr_lookup_fn(attrname);


            eattr_lookup[attridx].push_back(j);
        }

        string ename;
        vector<int> tmpvec; 
        for (int i=0; i < enames_v.size(); i++) {
            ename = enames_v[i]; // STR(enames,i);

            // Check if there are any updates for that attribute name
            if (eattr_lookup[i].size() == 0) {
                continue;
            }

            // Pull the vector
            if (igraph_cattribute_has_attr(g, IGRAPH_ATTRIBUTE_EDGE, ename.c_str())) {
                EANV(g, ename.c_str(), &eattr_v);
            } else {
                igraph_vector_resize(&eattr_v, igraph_ecount(g));
                igraph_vector_fill(&eattr_v, 0);
            }

            // Make the changes
            tmpvec = eattr_lookup[i];
            for (auto &j: tmpvec) {
                VECTOR(eattr_v)[_update_edge_attribute_v[j].get_eid()] = _update_edge_attribute_v[j].get_value();
            }
            // Push back to graph
            SETEANV(g, ename.c_str(), &eattr_v);
        }

        /*
        for (auto &a: eattrs) {
            // Check to see if attribute exists; if it does,
            // pull it, else create an empty vector
            if (igraph_cattribute_has_attr(g, IGRAPH_ATTRIBUTE_EDGE, a.c_str())) {
                // Pull the vector
                EANV(g, a.c_str(), &eattr_v);
            } else {
                igraph_vector_resize(&eattr_v, igraph_ecount(g));
                igraph_vector_fill(&eattr_v, 0);
                // throw invalid_argument("Edge attribute " + a+ " is not present in graph and must be added before update handlers");  
            }
            // Make the changes
            for (auto &i: _update_edge_attribute_v) {
                if (i.get_attr() == a) {
                    // igraph_vector_set(&eattr_v, i.get_eid(), (igraph_real_t) i.get_value());
                    VECTOR(eattr_v)[i.get_eid()] = (igraph_real_t) i.get_value();
                }
            }

            // Push back to graph
            SETEANV(g, a.c_str(), &eattr_v);
        }
        */

    }
    igraph_vector_destroy(&eattr_v);


    // cout << "Updating Vertex Attributes" << endl;
    // Update vertex attributes
    igraph_vector_t vattr_v;
    igraph_vector_init(&vattr_v, 0);
    igraph_vector_reserve(&vattr_v, _update_vertex_attribute_v.size()+100);

    if (_update_vertex_attribute_v.size() > 0) {
        // To do this we first need to collect 
        // all of the attribute updates into a map
        //
        // First create a vector containing the attribute name of each update,
        // such that the i-th entry in this vector represents the 
        // attribute name of i-th entry in the update vector.
        /*
        vector<string> vupdate_types; 
        vupdate_types.reserve(_update_vertex_attribute_v.size());
        for (auto &i: _update_vertex_attribute_v) {
            vupdate_types.push_back(i.get_attr());
        }
        */

        // Create hash of vnames and their locations in the update vector,
        // so that we can easily access the locations of each attribute to update.
        // In this map, the key is the name of the attribute and the value
        // is the index in the vnames vector.
        /*
        unordered_map<string,int> vnames_map;
        for (int i = 0; i < igraph_strvector_size(&vnames); i++) {
            vnames_map.insert({STR(vnames, i), i});
            // vnames_map[STR(vnames, i)] = i;
        }
        */ 

        // Then hash the location of each update of each type. 
        // TYhis vector is equal in length to all of the possible graph attributes.
        // If the 6-th attribute in the graph is disease_status, the 6-th entry in this vector
        // is a vector of ints corresponding to all of the indices of updates about disease_status.
        vector<vector<int>> vattr_lookup(igraph_strvector_size(&vnames), vector<int>(0));
        for (int i = 0; i < vattr_lookup.size(); i++) {
            vattr_lookup[i].reserve(1000);
        }
        string attrname;
        int vid;
        int attridx;
        for (int j = 0; j < _update_vertex_attribute_v.size(); j++) {
            attrname = _update_vertex_attribute_v[j].get_attr();
            vid = _update_vertex_attribute_v[j].get_vid();
            attridx = vattr_lookup_fn(attrname);


            vattr_lookup[attridx].push_back(j);

            // vattr_lookup[vnames_map.at(vupdate_types[j])].push_back(j);
        }


        // for (auto &a: vattr_map) {
        string vname;
        vector<int> tmpvec; 
        for (int i=0; i < igraph_strvector_size(&vnames); i++) {
            vname = STR(vnames,i);

            // Check if there are any updates for that attribute name
            if (vattr_lookup[i].size() == 0) {
                continue;
            }

            // Pull the vector
            VANV(g, vname.c_str(), &vattr_v);

            // Make the changes
            tmpvec = vattr_lookup[i];
            for (auto &j: tmpvec) {
                VECTOR(vattr_v)[_update_vertex_attribute_v[j].get_vid()] = _update_vertex_attribute_v[j].get_value();
            }
            // Push back to graph
            SETVANV(g, vname.c_str(), &vattr_v);
        }
        // cout << "Here 10.3" << endl;
    }
    igraph_vector_destroy(&vattr_v);



    igraph_strvector_destroy(&enames);
    igraph_strvector_destroy(&vnames);
    igraph_strvector_destroy(&gnames);
    igraph_vector_int_destroy(&etypes);
    igraph_vector_int_destroy(&vtypes);
    igraph_vector_int_destroy(&gtypes);


}

string UpdateList::print_updates() {
    /*
        vector<UpdateGraphAttribute> _update_graph_attribute_v;
        vector<CreateEdge> _create_edge_v;
        vector<DeleteEdge> _delete_edge_v;
        vector<UpdateEdgeAttribute> _update_edge_attribute_v;
        vector<UpdateVertexAttribute> _update_vertex_attribute_v;
        */ 

    string s = "~~~~~Update list~~~~~";

    s = s + "\nGraph updates:";
    if (_update_graph_attribute_v.size() > 0 ) {
        for (auto &u: _update_graph_attribute_v) {
            s = s + "\nAttribute: " + u.get_attr() + ", value: " + to_string(u.get_value());
        }

    } else {
        s = s + "\nNone";
    }

    s = s + "\n\nCreate Edges Between:";
    if (_create_edge_v.size() > 0 ) {
        for (auto &u: _create_edge_v) {
            s = s + "\nVertex 1: " + to_string(u.get_v1() ) + " and Vertex 2: " + to_string(u.get_v2());
        }

    } else {
        s = s + "\nNone";
    }

    s = s + "\n\nDelete Edges Between:";
    if (_delete_edge_v.size() > 0 ) {
        for (auto &u: _delete_edge_v) {
            s = s + "\nEdge id: " + to_string(u.get_eid()) + " between Vertex " + to_string(u.get_v1() ) + " and Vertex " + to_string(u.get_v2());
        }

    } else {
        s = s + "\nNone";
    }

    s = s + "\n\nUpdate Edge Attributes:";
    if (_update_edge_attribute_v.size() > 0 ) {
        for (auto &u: _update_edge_attribute_v) {
            s = s +"\nEdge id: " + to_string(u.get_eid() ) + " between Vertex " + to_string(u.get_v1() ) + " and Vertex " + to_string(u.get_v2()) + ", Attribute: " + u.get_attr() + ", value: " + to_string(u.get_value());
        }

    } else {
        s = s + "\nNone";
    }

    s = s + "\n\nUpdate Vertex Attributes:";
    if (_update_vertex_attribute_v.size() > 0 ) {
        for (auto &u: _update_vertex_attribute_v) {
                s = s + "\nVertex : " + to_string(u.get_vid() ) + ", Attribute: " + u.get_attr() + ", value: " + to_string(u.get_value());
        }

    } else {
        s = s + "\nNone";
    }
    
    s = s + "\n\nTotal updates:\n" + to_string(
        _update_graph_attribute_v.size() + 
        _create_edge_v.size() + 
        _delete_edge_v.size() + 
        _update_edge_attribute_v.size() + 
        _update_vertex_attribute_v.size()
        );

    return s;
}
