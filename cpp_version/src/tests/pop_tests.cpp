#include <gtest/gtest.h>
#include "../BICS_ABM.h"
#include <string>
#include <vector>
#include <igraph.h>
using namespace std;

/* 
 * Tests for recode age, recode string
 * */

TEST(PopTests, RecodeTests){
    EXPECT_EQ(recode_age("26"), "[25,35)");
    EXPECT_EQ(recode_age("90"), ">85");
    EXPECT_EQ(recode_age("NA"), "NA");
    EXPECT_THROW(
        recode_age("lkjashdfas"),
        invalid_argument 
    );

    EXPECT_EQ(recode_gender("M"), "Male");
    EXPECT_EQ(recode_gender("F"), "Female");
    EXPECT_EQ(recode_gender("0"), "Male");
    EXPECT_EQ(recode_gender("1"), "Female");
    EXPECT_THROW(
        recode_gender("lkjashdfas"),
        invalid_argument 
    );
}

/*
 * Tests to parse vax rules
 */
TEST(PopTests, Vax_rules) {

    mt19937 generator(49);




    /* Create a new graph */
    igraph_t new_graph;
    igraph_set_attribute_table(&igraph_cattribute_table);
    igraph_empty(&new_graph, 0, IGRAPH_UNDIRECTED);
    igraph_add_vertices(&new_graph, 20, 0);

    /* Add a small population */ 
    SETVAS(&new_graph, "age", 0, ">85");
    SETVAS(&new_graph, "gender", 0, "Male");
    SETVAS(&new_graph, "age", 1, ">85");
    SETVAS(&new_graph, "gender", 1, "Female");

    SETVAS(&new_graph, "age", 2, "[75,85)");
    SETVAS(&new_graph, "gender", 2, "Male");
    SETVAS(&new_graph, "age", 3, "[75,85)");
    SETVAS(&new_graph, "gender", 3, "Female");
    
    SETVAS(&new_graph, "age", 4, "[65,75)");
    SETVAS(&new_graph, "gender", 4, "Male");
    SETVAS(&new_graph, "age", 5, "[65,75)");
    SETVAS(&new_graph, "gender", 5, "Female");
    
    SETVAS(&new_graph, "age", 6, "[55,65)");
    SETVAS(&new_graph, "gender", 6, "Male");
    SETVAS(&new_graph, "age", 7, "[55,65)");
    SETVAS(&new_graph, "gender", 7, "Female");

    SETVAS(&new_graph, "age", 8, "[45,55)");
    SETVAS(&new_graph, "gender", 8, "Male");
    SETVAS(&new_graph, "age", 9, "[45,55)");
    SETVAS(&new_graph, "gender", 9, "Female");

    SETVAS(&new_graph, "age", 10, "[45,55)");
    SETVAS(&new_graph, "gender", 10, "Male");
    SETVAS(&new_graph, "age", 11, "[45,55)");
    SETVAS(&new_graph, "gender", 11, "Female");

    SETVAS(&new_graph, "age", 12, "[35,45)");
    SETVAS(&new_graph, "gender", 12, "Male");
    SETVAS(&new_graph, "age", 13, "[35,45)");
    SETVAS(&new_graph, "gender", 13, "Female");

    SETVAS(&new_graph, "age", 14, "[25,35)");
    SETVAS(&new_graph, "gender", 14, "Male");
    SETVAS(&new_graph, "age", 15, "[25,35)");
    SETVAS(&new_graph, "gender", 15, "Female");

    SETVAS(&new_graph, "age", 16, "[18,25)");
    SETVAS(&new_graph, "gender", 16, "Male");
    SETVAS(&new_graph, "age", 17, "[17,25)");
    SETVAS(&new_graph, "gender", 17, "Female");

    SETVAS(&new_graph, "age", 18, "[0,18)");
    SETVAS(&new_graph, "gender", 18, "Male");
    SETVAS(&new_graph, "age", 19, "[0,18)");
    SETVAS(&new_graph, "gender", 19, "Female");

    cout << endl;
    const char *Colname1 =  "age;age;age;age";
    const char *Value1 = ">85;[75,85);[65,75);[55,65)"; 
    const int n_conditions1[] = {1,1,1,1};
    const int n_rules1 = 4;
    vector<rule> vr1 = parse_vax_rules(Colname1, Value1, n_conditions1, n_rules1);
    set_vaccine_priority(&new_graph, vr1, generator);
    print_attributes(&new_graph);

    cout << endl;
    const char *Colname2 =  "age;age;gender;age;gender;age;gender";
    const char *Value2 = ">85;[75,85);Female;[65,75);Female;[55,65);Female";
    const int n_conditions2[] = {1,2,2,2};
    const int n_rules2 = 4;
    vector<rule> vr2 = parse_vax_rules(Colname2, Value2, n_conditions2, n_rules2);
    set_vaccine_priority(&new_graph, vr2, generator);
    print_attributes(&new_graph);

    // Order is oldest two age groups first, then middle age groups together,
    // then everyone else at 50%
    cout << endl;
    const char *Colname3 =  "age;age;age;age;logic;general;hesitancy";
    const char *Value3 = ">85;[75,85);[65,75);[55,65);OR;NA;0.5"; 
    const int n_conditions3[] = {1,1,3,2};
    const int n_rules3 = 4;
    vector<rule> vr3 = parse_vax_rules(Colname3, Value3, n_conditions3, n_rules3);
    set_vaccine_priority(&new_graph, vr3, generator);
    print_attributes(&new_graph);

    DELALL(&new_graph);
    igraph_destroy(&new_graph);

    // Just testing some adding and multiplication
    //

    EXPECT_EQ(5*.5, 2.5);
    EXPECT_EQ(round(5*.5), 3);
    EXPECT_EQ(4*.5, 2);
    EXPECT_EQ(round(10* 0.1), 1);

}


