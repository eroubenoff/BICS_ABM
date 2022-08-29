#include <stdio.h> 
#include <string>
#include <iostream>
#include <vector>
#include <stdlib.h>  

using namespace std;

class Node {
	private:
		int _id;
		vector<int> _edges;
		int _gender;
		int _age;

	public:
		int id() {
			return _id;
		}
		vector<int> edges() {
			return _edges;
		}
		void add_edge(int n) {
			_edges.push_back(n);
			return;
		}
		void remove_edge(int n) {
			_edges.pop(n);
			return;
		}
		void clear_edges() {
			_edges.clear();
			return;
		}

		// Fully parameterized constructor
		Node(int id, vector<int> edges, int gender, int age){
			_id = id;
			_edges = edges;
			_gender = gender;
			_age = age;
		}


}

class Household{
	private:
		int _hhid;
		int _hhsize;
		int _head;
		vector<int> _memberlist;

	public:
		int hhid() {
			return _hhid;
		}
		int hhsize() {
			return _hhsize;
		}
		int head() {
			return _head;
		}
		vector<int> memberlist() {
			return memberlist;
		}
		void add_member(r) {
			_memberlist.push_back(r);
		}

		Household(){};

		// Fully parameterized constructor
		Household(int hhid, int hhsize, int head, vector<int> memberlist){
			_hhid = hhid;
			_hhsize = hhsize;
			_head = head;
			_memberlist = memberlist; 
		};



}