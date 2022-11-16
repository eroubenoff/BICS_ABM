#include <gtest/gtest.h>
#include "../BICS_ABM.h"
#include <vector>
#include <string>

using namespace std;

TEST(DataTests, LoadDataTest) {
    Data data(4);

    EXPECT_EQ(data.BICS_nrow, 2993);
    EXPECT_EQ(data.POLYMOD_nrow, 7290);
    
    // Pick some random columns from the data
    EXPECT_EQ(data.BICS(152, "ResponseId"), "R_22qSMMHNhKfSOFZ");
    EXPECT_EQ(data.BICS(189, "resp_zip"), "2155");
    EXPECT_EQ(data.BICS(361, "resp_employ"), "2");
    EXPECT_EQ(data.BICS(441, "resp_focal_industry_DO_3"), "3.0");
    EXPECT_EQ(data.BICS(713, "resp_occupation_27_TEXT"), "");
    // EXPECT_EQ(data.BICS(181728471, "lkjasd fads"), "");
     



}
