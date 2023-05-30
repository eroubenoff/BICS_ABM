#include <igraph.h>
#include "BICS_ABM.h"
#include <vector>
#include <string>
#include <iostream>

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



// Derived classes

UpdateGraphAttribute::UpdateGraphAttribute(string attr, int value) {
    _attr = attr;
    _value = value;
}

string UpdateGraphAttribute::get_attr() {
    return _attr;
}
int UpdateGraphAttribute::get_value() { 
    return _value;
}

CreateEdge::CreateEdge(int v1, int v2) {
    if (v2 < v1) {swap(v1, v2);}
    _v1 = v1;
    _v2 = v2;
}
int CreateEdge::get_v1() {
    return _v1;
}
int CreateEdge::get_v2() {
    return _v2;
}

DeleteEdge::DeleteEdge(int v1, int v2) {
    if (v2 < v1) {swap(v1, v2);}
    _v1 = v1;
    _v2 = v2;
    _eid_or_vpair = 1;
}
DeleteEdge::DeleteEdge(int eid) {
    _eid = eid;
    _eid_or_vpair = 0;
}
void DeleteEdge::retrieve_eid(igraph_t* g) {
    if (_eid_or_vpair == 1) {
        igraph_integer_t eid;
        igraph_get_eid(g, &eid, _v1, _v2, false, false);
        _eid = eid;
    }
}
void DeleteEdge::retrieve_endpoints(igraph_t* g) {
    if (_eid_or_vpair == 0) {
        igraph_integer_t v1, v2;
        igraph_edge(g, _eid, &v1, &v2);
        _v1 = v1;
        _v2 = v2;
        if (_v2 < _v1) {swap(_v1, _v2);}
    }
}
int DeleteEdge::get_v1() { 
    return _v1;
}
int DeleteEdge::get_v2() { 
    return _v2;
}
int DeleteEdge::get_eid() { 
    return _eid;
}

/* 
 * Update an edge attribute: be defined for either a pair of
 * end points, or an edge id.
 * 
 * Can either include a single attribute to update or 
 * dict of attributes, in string-int pairs.
 */
UpdateEdgeAttribute::UpdateEdgeAttribute(int eid, unordered_map <string, int> attr) {
    _eid = eid;
    _attr = attr;
    _eid_or_vpair = 0;
}
UpdateEdgeAttribute::UpdateEdgeAttribute(int v1, int v2, unordered_map <string, int> attr) {
    if (v2 < v1) {swap(v1, v2);}
    _v1 = v1;
    _v2 = v2;
    _attr = attr;
    _eid_or_vpair = 1;
}
UpdateEdgeAttribute::UpdateEdgeAttribute(int eid, string attr, int value) {
    _eid = eid;
    _attr[attr] = value;
    _eid_or_vpair = 0;
}
UpdateEdgeAttribute::UpdateEdgeAttribute(int v1, int v2, string attr, int value) {
    if (v2 < v1) {swap(v1, v2);}
    _v1 = v1;
    _v2 = v2;
    _attr[attr] = value;
    _eid_or_vpair = 1;
}
void UpdateEdgeAttribute::retrieve_eid(igraph_t* g) {
    if (_eid_or_vpair == 1) {
        igraph_integer_t eid;
        igraph_get_eid(g, &eid, _v1, _v2, false, false);
        _eid = eid;
    }
}
void UpdateEdgeAttribute::retrieve_endpoints(igraph_t* g) {
    if (_eid_or_vpair == 0) {
        _v1 = IGRAPH_FROM(g, _eid);
        _v2 = IGRAPH_TO(g, _eid);
        if (_v2 < _v1) {swap(_v1, _v2);}
    }
}
int UpdateEdgeAttribute::get_v1() {
    return _v1;
}
int UpdateEdgeAttribute::get_v2() {
    return _v2;
}
int UpdateEdgeAttribute::get_eid() {
    return _eid;
}
unordered_map<string, int> UpdateEdgeAttribute::get_attrs() {
    return _attr;
}



/* 
 * Update a vertex attribute:
 * 
 * Can either include a single attribute to update or 
 * dict of attributes, in string-int pairs.
 */
UpdateVertexAttribute::UpdateVertexAttribute(int vid, string attr, int value) {
    _vid = vid;
    _attr = attr;
    _value = value;
}
int& UpdateVertexAttribute::get_vid() {
    return _vid;
} 
string& UpdateVertexAttribute::get_attr() {
    return _attr;
} 
int& UpdateVertexAttribute::get_value() {
    return _value;
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

    // First, for all edge updates, make sure we are working with 
    // edge endpoints rather than edge ids. This makes it much
    // more safe-- what if an edge gets deleted,etc. 

    for (int i = 0; i < _delete_edge_v.size(); i++) {
        _delete_edge_v[i].retrieve_endpoints(g);
        _delete_edge_v[i].retrieve_eid(g);
    }
    for (int i = 0; i < _update_edge_attribute_v.size(); i++) {
        _update_edge_attribute_v[i].retrieve_endpoints(g);
        _update_edge_attribute_v[i].retrieve_eid(g);
    }

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

    // Delete any edges (do this by end points rather than EIDs)
    if (_delete_edge_v.size() > 0 ) {

        for (auto &u: _delete_edge_v) {
            igraph_vector_int_push_back(&edges_v, u.get_v1());
            igraph_vector_int_push_back(&edges_v, u.get_v2());
        }

        igraph_es_pairs(&edges_es, &edges_v, false);
        igraph_delete_edges(g, edges_es);



    }

    // Update edge attributes
    if (_update_edge_attribute_v.size() > 0) {
        // To do this we first need to collect 
        // all of the attribute updates into a map
        // Can do this step by eid

        
        // Keys are attribute and eid
        unordered_map<string, unordered_map<int, int>> eattrs;

        for (auto &u: _update_edge_attribute_v) {
            for (auto a: u.get_attrs()) {
                // Then collect the attributes by name 
                eattrs[a.first][u.get_eid()] = a.second;
            }

        }

        for (auto &a: eattrs) {
            // Check to see if attribute exists; if it does,
            // pull it, else create an empty vector
            if (igraph_cattribute_has_attr(g, IGRAPH_ATTRIBUTE_EDGE, a.first.c_str())) {
                // Pull the vector
                EANV(g, a.first.c_str(), &eattr_v);
            } else {
                throw invalid_argument("Vertex attribute " + a.first + " is not present in graph and must be added before update handlers");  
                // igraph_vector_resize(&eattr_v, igraph_ecount(g));
                // igraph_vector_null(&eattr_v);
            }
            // Make the changes
            for (auto &i: a.second) {
                VECTOR(eattr_v)[i.first] = i.second;
            }
            // Push back to graph
            SETEANV(g, a.first.c_str(), &eattr_v);
        }

    }

    // Update vertex attributes
    if (_update_vertex_attribute_v.size() > 0) {
        // To do this we first need to collect 
        // all of the attribute updates into a map
        // Can do this step by eid

        
        // Keys are attribute and eid
        unordered_map<string, vector<tuple<int, int>>> vattrs;

        for (auto &u: _update_vertex_attribute_v) {
            vattrs[u.get_attr()].push_back(make_tuple(u.get_vid(),  u.get_value()));
        }

        for (auto &a: vattrs) {
            //cout << "a.first" <<  a.first << endl;
            // Pull the vector
            // Check to see if attribute exists; if it does,
            // pull it, else create an empty vector
            if (igraph_cattribute_has_attr(g, IGRAPH_ATTRIBUTE_VERTEX, a.first.c_str())) {
                // Pull the vector
                VANV(g, a.first.c_str(), &vattr_v);
            } else {
                throw invalid_argument("Vertex attribute " + a.first + " is not present in graph and must be added before update handlers");  
                // igraph_vector_resize(&vattr_v, igraph_vcount(g));
                // igraph_vector_null(&vattr_v);
            }
            // Make the changes
            for (auto &i: a.second) {
                // cout << "i.first " << i.first << " i.second " << i.second << endl;
                VECTOR(vattr_v)[get<0>(i)] = get<1>(i);
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
