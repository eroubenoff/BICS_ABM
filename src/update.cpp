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
        _create_edge_v.reserve(100);
        _delete_edge_v.reserve(100);
        _update_edge_attribute_v.reserve(100);
        _update_vertex_attribute_v.reserve(100);
}
void UpdateList::add_update(UpdateGraphAttribute update) {
    _update_graph_attribute_v.push_back(update);
}
void UpdateList::add_update(CreateEdge update) {
    _create_edge_v.push_back(update);
}
void UpdateList::add_update(DeleteEdge update) {
    _delete_edge_v.push_back(update);
}
void UpdateList::add_update(UpdateEdgeAttribute update) {
    _update_edge_attribute_v.push_back(update);
}
void UpdateList::add_update(UpdateVertexAttribute update) {
    _update_vertex_attribute_v.push_back(update);
}
void UpdateList::clear_updates() {
    _update_graph_attribute_v.clear();
    _create_edge_v.clear();
    _delete_edge_v.clear();
    _update_edge_attribute_v.clear();
    _update_vertex_attribute_v.clear();

    _update_graph_attribute_v.reserve(100);
    _create_edge_v.reserve(500);
    _delete_edge_v.reserve(100);
    _update_edge_attribute_v.reserve(100);
    _update_vertex_attribute_v.reserve(500);
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



    // cout << "Adding graph updates" << endl;

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
        // _delete_edge_v[i].retrieve_eid(g);
    }
    // cout << "Getting endpoints for update edge attribute vector" << endl;
    for (int i = 0; i < _update_edge_attribute_v.size(); i++) {
        _update_edge_attribute_v[i].retrieve_endpoints(g);
        // _update_edge_attribute_v[i].retrieve_eid(g);
    }

    //  << "Deleting edges" << endl;
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

    // cout << "Creating edges" << endl;
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
        
        // for (int i = 0; i < igraph_vector_int_size(&new_edges); i++) cout << VECTOR(new_edges)[i] << "  ";
        // cout << endl;
        
        // cout << "Adding edges" << endl;
        igraph_add_edges(g, &new_edges, NULL);
        // cout << "Done" << endl;

    }
    igraph_vector_int_destroy(&new_edges);

    // Update edge attributes
    // First make sure we have all edge ids. This step
    // has to be done by edge id, where the delete step can 
    // be done by end points.
    // cout << "Updating edge attributes: Getting eids" << endl;
    for (int i = 0; i < _update_edge_attribute_v.size(); i++) {
        // _update_edge_attribute_v[i].retrieve_endpoints(g);
        _update_edge_attribute_v[i].retrieve_eid(g);
    }
    // cout << "Update edge attributes" << endl;

    igraph_vector_t eattr_v;
    igraph_vector_init(&eattr_v, 0);
    igraph_vector_reserve(&eattr_v, _update_edge_attribute_v.size()+100);

    if (_update_edge_attribute_v.size() > 0) {
        // To do this we first need to collect 
        // all of the attribute updates into a map
        // Can do this step by eid

        unordered_set<string> eattrs;
        for (auto &i: _update_edge_attribute_v) {
            eattrs.insert(i.get_attr());

            // cout << "eid: " <<  i.get_eid() << " attr: " << i.get_attr() << " val:  " << i.get_value() << endl;
        }

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
                    VECTOR(eattr_v)[i.get_eid()] = (int) i.get_value();
                }
            }

            // for (int i = 0; i < igraph_vector_size(&eattr_v); i++) cout << VECTOR(eattr_v)[i]; cout << endl;
            // Push back to graph
            SETEANV(g, a.c_str(), &eattr_v);
        }

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

        // New method: create a vector of the edge attribute names

        // cout << "Here 10.11" << endl;
        vector<string> vupdate_types;
        vupdate_types.reserve(_update_vertex_attribute_v.size());
        for (auto &i: _update_vertex_attribute_v) {
            vupdate_types.push_back(i.get_attr());
        }
        // cout << "Here 10.12" << endl;

        // Create hash of vnames
        map<string,int> vnames_map;
        for (int i = 0; i < igraph_strvector_size(&vnames); i++) {
            vnames_map[STR(vnames, i)] = i;
        }

        // cout << "Here 10.13" << endl;
        // Then hash the location of each type. This is done in parallel
        // with the vnames vector pulled from the graph
        vector<vector<int>> vattr_lookup(igraph_strvector_size(&vnames), vector<int>(0));


        // cout << "Here 10.1" << endl;
        for (int j = 0; j < vupdate_types.size(); j++) {
            vattr_lookup[vnames_map[vupdate_types[j]]].push_back(j);
        }


        // cout << "Here 10.2" << endl;
        // for (auto &a: vattr_map) {
        string vname;
        for (int i=0; i < igraph_strvector_size(&vnames); i++) {
            vname = STR(vnames,i);

            // Check if there are any updates for that attribute name

            if (vattr_lookup[i].size() == 0) {
                continue;
            }

            // Pull the vector
            VANV(g, vname.c_str(), &vattr_v);

            // Make the changes
            for (auto &j: vattr_lookup[i]) {
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

    // cout << "Update completed" << endl;

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
