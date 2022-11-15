#include <gtest/gtest.h>
#include "../BICS_ABM.h"
#include <string>
#include <vector>
using namespace std;

/* 
 * Tests for recode age, recode string
 * */

TEST(PopTests, RecodeTests){
    EXPECT_EQ(recode_age("26"), "[25,35)");
    EXPECT_EQ(recode_age("90"), "[>85)");
    EXPECT_EQ(recode_age("NA"), "NA");
    EXPECT_THROW(
        recode_age("lkjashdfas"),
        runtime_error
    );

    EXPECT_EQ(recode_gender("M"), "Male");
    EXPECT_EQ(recode_gender("F"), "Female");
    EXPECT_EQ(recode_gender("0"), "Male");
    EXPECT_EQ(recode_gender("1"), "Female");
    EXPECT_THROW(
        recode_gender("lkjashdfas"),
        runtime_error
    );
}

/*
 * Tests to parse vax rules
 */
TEST(PopTests, Vax_rules) {


    const char *Colname1 =  "age;age;age;age;age";
    const char *Value1 = ">85;[75,85);[65,75);[55,65)"; 
    const int n_conditions1[] = {1,1,1,1};
    const int n_rules1 = 4;
    vector<rule> vr1 = create_vax_rules(Colname1, Value1, n_conditions1, n_rules1);

    const char *Colname2 =  "age;age;gender;age;gender;age;gender";
    const char *Value2 = ">85;[75,85);Female;[65,75);Female;[55,65);Female";
    const int n_conditions2[] = {1,2,2,2};
    const int n_rules2 = 4;
    vector<rule> vr2 = create_vax_rules(Colname2, Value2, n_conditions2, n_rules2);



}
