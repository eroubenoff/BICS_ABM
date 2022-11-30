"""
Python equivalent of create_pop.cpp

This is actually faster and more reliable
than the CPP version

"""

import pandas as pd
import numpy as np




def recode_age(age):
    
    if np.isnan(age):
        return -1

    if (age < 18):
        return 0
    elif (age < 25):
        return 1
    elif (age < 35):
        return 2
    elif (age < 45):
        return 3
    elif (age < 55):
        return 4
    elif (age < 65):
        return 5
    elif (age < 75):
        return 6
    elif (age < 85):
        return 7
    elif (age >= 85):
        return 8

    else:
        raise ValueError("Invalid age: "  + str(age))

def recode_gender(gender):
    try:
        if np.isnan(gender):
            return -1
    except:
        pass


    if (gender == "F"):
        return 1
    elif gender == "Female":
        return 1
    elif (gender == "M"):
        return 0
    elif (gender == "Male"):
        return 0

    else:
        raise ValueError("Invalid Gender: " + str(gender))

def recode_lefthome(lefthome_num):
    if lefthome_num == "More than 5":
        return 6
    else:
        try: 
            return int(lefthome_num)
        except:
            raise ValueError("Invalid lefthome num: " +  lefthome_num)

def create_pop(n_hh = 1000, wave = 6):

    BICS = pd.read_csv("data/df_all_waves.csv")
    BICS = BICS[BICS["wave"] == 6].copy(deep=True).reset_index()

    # Need to recode all age and gender columns
    BICS["age"] = BICS["age"].apply(recode_age)
    # BICS["resp_hh_roster#1_0_1"] = BICS["resp_hh_roster#1_0_1"].apply(recode_age)
    BICS["resp_hh_roster#1_1_1"] = BICS["resp_hh_roster#1_1_1"].apply(recode_age)
    BICS["resp_hh_roster#1_2_1"] = BICS["resp_hh_roster#1_2_1"].apply(recode_age)
    BICS["resp_hh_roster#1_3_1"] = BICS["resp_hh_roster#1_3_1"].apply(recode_age)
    BICS["resp_hh_roster#1_4_1"] = BICS["resp_hh_roster#1_4_1"].apply(recode_age)
    BICS["resp_hh_roster#1_5_1"] = BICS["resp_hh_roster#1_5_1"].apply(recode_age)

    BICS["gender"] = BICS["gender"].apply(recode_gender)
    # BICS["resp_hh_roster#2_0"] = BICS["resp_hh_roster#2_0"].apply(recode_gender)
    BICS["resp_hh_roster#2_1"] = BICS["resp_hh_roster#2_1"].apply(recode_gender)
    BICS["resp_hh_roster#2_2"] = BICS["resp_hh_roster#2_2"].apply(recode_gender)
    BICS["resp_hh_roster#2_3"] = BICS["resp_hh_roster#2_3"].apply(recode_gender)
    BICS["resp_hh_roster#2_4"] = BICS["resp_hh_roster#2_4"].apply(recode_gender)
    BICS["resp_hh_roster#2_5"] = BICS["resp_hh_roster#2_5"].apply(recode_gender)

    BICS["lefthome_num"] = BICS["lefthome_num"].apply(recode_lefthome)

    pop = list()

    for hh in range(n_hh):
        hhead = BICS.sample(weights = BICS.weight_pooled)
        hhead['hhid'] = hh
        pop.append(hhead)

        hhsize = hhead.hhsize.iloc[0]

        if hhsize > 1: 
            for hhmember in range(min(hhsize, 5)):
                hhmember = hhmember + 1
                hhmember_age = hhead["resp_hh_roster#1_" + str(hhmember) + "_1"].iloc[0]
                hhmember_gender = hhead["resp_hh_roster#2_" + str(hhmember)].iloc[0]

                # Sample a corresponding person
                hhmember = BICS.query("age == " + str(hhmember_age) + " & gender == " + str(hhmember_gender))
                if hhmember.shape[0] == 0:
                    hhmember = BICS
                hhmember = hhmember.sample(weights = hhmember.weight_pooled)
                hhmember['hhid'] = hh
                pop.append(hhmember)


    pop = pd.concat(pop)


    return pop


def create_vax_priority(pop, vax_rules = None ): 
    """
    Assume that vax_rules is in the form of:
    [
        [("age", ">85")],
        [("age", "[75,85)")],
        [("age", "[65,75)"), ("hesitancy", "0.4")]
    ]

    """

    pop["vaccine_priority"] = 0

    if vax_rules is None: 
        return pop

    # Parse rules into a more ordered format 
    for i, v in enumerate(vax_rules):
        pop.loc[pop.eval(v), "vaccine_priority"] = i

    return pop 


    




def pop_to_np(pop: pd.DataFrame):

    # Pull out the columns relevant and put everybody in a COLUMN-MAJOR (fortran-style) matrix! 
    # All data types are numeric

    pop_np = np.zeros((pop.shape[0], 5), dtype = float, order = 'F')

    pop_np[:,0] = pop.hhid
    pop_np[:,1] = pop.age
    pop_np[:,2] = pop.gender
    pop_np[:,3] = pop.num_cc_nonhh
    pop_np[:,4] = pop.lefthome_num

    return pop_np


if __name__ == "__main__":
    pop = create_pop()



