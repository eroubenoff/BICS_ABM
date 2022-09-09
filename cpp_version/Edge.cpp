#include <string>
#include <vector>
#include <unordered_map>
#include "defs.h"
using namespace std;

/*
Constructor for edges. Edges live in an edgelist within population. 
On creation of an edge, nodes are updated to include linked nodes. 
*/



string edgeid_generator(string id1, string id2) {
	/* Rules:
		General heuristic: priority is 123ABC
		If id1[0] is a number and id2[0] is a character, then order is id1_id2
		If id1[0] is a character and id2[0] is a number, then order is id2_id1
		If both are characters, then alphabetically first
		If both are numbers, then lowest first

		If first number is a tie then recurse, dropping the first letter
		If self-edge, then return id1_id2 
	*/

	string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");

	if (id1 == id2) throw exception("Invalid self-edge");
	if (id1 == "" & id2 == "") throw exception("Invalid node ids");

	int pos1 = 0;
	int pos2 = 0;

	for (pos1; id1[0] != str[pos1]; pos1++) continue;
	for (pos2; id2[0] != str[pos2]; pos2++) continue;

	if (pos1 < pos2) 
		return id1 + "_" + id2;
	else if (pos2 < pos1) 
		return id2 + "_" + id1;
	else if (pos1 == pos2) 
		pos1[0] = pos2[0] = "."
		return edgeid_generator(id1, id2);


}

Edge::Edge(Node* n1, Node* n2) {
	_n1 = n1;
	_n2 = n2; 
	_id = edgeid_generator(n1 -> id(), n2 -> id());

	n1 -> add_edge(n2 -> id());
	n2 -> add_edge(n1 -> id());
}

Edge::id() {
	return _id;
}

Edge::n1