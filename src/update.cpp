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
    if (_eid_or_vpair == 1) {
        igraph_integer_t eid;
        igraph_get_eid(g, &eid, _v1, _v2, false, false);
        _eid = eid;
    }
    if ((_eid == -1) & (_break_on_failure) ) {
        print_attributes(g);
        throw invalid_argument("Error called from DeleteEdge::retrieve_eid: No edge between " + to_string(_v1) + ", " + to_string(_v2) + " present in graph");
    }
}
void EdgeUpdate::retrieve_endpoints(igraph_t* g) {
    if (_eid_or_vpair == 0) {
        if (_eid > igraph_ecount(g)) {
            if (_break_on_failure) {
                print_attributes(g);
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
}


// AttributeUpdate functions
string AttributeUpdate::get_attr() {
    return _attr;
} 
int AttributeUpdate::get_value() {
    return _value;
}


// Derived classes
//
// GraphUpdate functions
UpdateGraphAttribute::UpdateGraphAttribute(string attr, int value) {
    _attr = attr;
    _value = value;
}

// Edge functions
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

UpdateEdgeAttribute::UpdateEdgeAttribute(int eid, string attr, int value) {
    _v1 = -1;
    _v2 = -1;
    _eid = eid;
    _eid_or_vpair = 0;
    _break_on_failure = 1;
    _attr = attr;
    _value = _value;
}
UpdateEdgeAttribute::UpdateEdgeAttribute(int v1, int v2, string attr, int value) {
    if (v2 < v1) {swap(v1, v2);}
    _v1 = v1;
    _v2 = v2;
    _eid = -1;
    _eid_or_vpair = 1;
    _break_on_failure = 1;
    _attr = attr;
    _value = _value;
}

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
UpdateVertexAttribute::UpdateVertexAttribute(int vid, string attr, int value) {
    _vid = vid;
    _attr = attr;
    _value = value;
}


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
}
void UpdateList::add_updates_to_graph(igraph_t *g) {

    igraph_vector_t eattr_v;
    igraph_vector_init(&eattr_v, 0);
    igraph_vector_t vattr_v;
    igraph_vector_init(&vattr_v, 0);
    igraph_vector_int_t new_edges;
    igraph_vector_int_init(&new_edges, 0);
    igraph_vector_int_t edges_v;
    igraph_vector_int_init(&edges_v, 0);
    igraph_es_t edges_es;

    // Begin with graph attributes
    // Don't bother with pulling the vectors of attributes
    // here. There usually aren't that many graph updates
    // so it makes sense to just do them individually.
    if (_update_graph_attribute_v.size() > 0) {
        for (auto &u: _update_graph_attribute_v) {
            SETGAN(g, u.get_attr().c_str(), u.get_value());
        }
    }

    // Create any needed edges
    if (_create_edge_v.size() > 0) {

        for (auto &u: _create_edge_v) {
            igraph_vector_int_push_back(&new_edges, u.get_v1());
            igraph_vector_int_push_back(&new_edges, u.get_v2());
        }
        
        igraph_add_edges(g, &new_edges, NULL);

    }

    // Delete any edges (do this by EID instead of end points)
    // First, for all edge updates, make sure we are working with 
    // edge endpoints rather than edge ids. This makes it much
    // more safe-- what if an edge gets deleted,etc. 

    for (int i = 0; i < _delete_edge_v.size(); i++) {
        _delete_edge_v[i].retrieve_endpoints(g);
        _delete_edge_v[i].retrieve_eid(g);
    }
    if (_delete_edge_v.size() > 0 ) {

        for (auto &u: _delete_edge_v) {
            // cout << "vid1: " << to_string(u.get_v1()) << " vid2: " << to_string(u.get_v2()) << " eid: " << to_string(u.get_eid()) << endl;
            if ((u.get_v1() == -1) | (u.get_v2() == -1) | (u.get_eid() == -1)) {
                if (u.break_on_failure()) {
                    print_attributes(g);
                    throw invalid_argument("Error called from UpdateList::add_updates_to_graph: No edge between " + 
                            to_string(u.get_v1() ) + ", " + to_string(u.get_v2()) + " present in graph");
                }
            } else {

                igraph_vector_int_push_back(&edges_v, u.get_eid());
            }
        }

        igraph_es_vector(&edges_es, &edges_v);
        igraph_delete_edges(g, edges_es);

    }

    // Update edge attributes
    // First retrieve all eids/end points
    for (int i = 0; i < _update_edge_attribute_v.size(); i++) {
        _update_edge_attribute_v[i].retrieve_endpoints(g);
        _update_edge_attribute_v[i].retrieve_eid(g);
    }

    if (_update_edge_attribute_v.size() > 0) {
        // To do this we first need to collect 
        // all of the attribute updates into a map
        // Can do this step by eid

        unordered_set<string> eattrs;
        for (auto &i: _update_edge_attribute_v) {
            eattrs.insert(i.get_attr());
        }

        for (auto &a: eattrs) {
            // Check to see if attribute exists; if it does,
            // pull it, else create an empty vector
            if (igraph_cattribute_has_attr(g, IGRAPH_ATTRIBUTE_EDGE, a.c_str())) {
                // Pull the vector
                EANV(g, a.c_str(), &eattr_v);
            } else {
                igraph_vector_resize(&eattr_v, igraph_ecount(g));
                igraph_vector_null(&eattr_v);
                // throw invalid_argument("Edge attribute " + a+ " is not present in graph and must be added before update handlers");  
            }
            // Make the changes
            for (auto &i: _update_edge_attribute_v) {
                if (i.get_attr() == a) {
                    VECTOR(eattr_v)[i.get_eid()] = i.get_value();
                }
            }
            // Push back to graph
            SETEANV(g, a.c_str(), &eattr_v);
        }

    }


    // Update vertex attributes
    if (_update_vertex_attribute_v.size() > 0) {
        // To do this we first need to collect 
        // all of the attribute updates into a map

        // First step: iterate over all vattrs
        // and create a set of attribute names 
        // with positions corresponding to _update_vertex_attributes_v

        /*
        unordered_set<string> vattrs;
        for (auto &i: _update_vertex_attribute_v) {
            vattrs.insert(i.get_attr());
        }
        */

        map<string, vector<UpdateVertexAttribute*>> vattr_map;
        for (auto &i: _update_vertex_attribute_v) {
            vattr_map[i.get_attr()].push_back(&i);
        }

        for (auto &a: vattr_map) {

            // Pull the vector
            // Check to see if attribute exists; if it does,
            // pull it, else create an empty vector
            if (igraph_cattribute_has_attr(g, IGRAPH_ATTRIBUTE_VERTEX, a.first.c_str())) {
                // Pull the vector
                VANV(g, a.first.c_str(), &vattr_v);
            } else {
                igraph_vector_resize(&vattr_v, igraph_vcount(g));
                // throw invalid_argument("Vertex attribute " + a + " is not present in graph and must be added before update handlers");  
                // igraph_vector_resize(&vattr_v, igraph_vcount(g));
                igraph_vector_null(&vattr_v);
            }
            // Make the changes
            for (auto &i: a.second) {
                VECTOR(vattr_v)[i ->get_vid()] = i ->get_value();
            }
            // Push back to graph
            SETVANV(g, a.first.c_str(), &vattr_v);
        }
    }



        igraph_vector_destroy(&eattr_v);
        igraph_vector_destroy(&vattr_v);
        igraph_vector_int_destroy(&new_edges);
        igraph_vector_int_destroy(&edges_v);
        igraph_es_destroy(&edges_es);

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
