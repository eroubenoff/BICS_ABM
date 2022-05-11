"""
Loads BICS data
"""
import pdb
import numpy as np
import pandas as pd
from Population import Population, Household
import rpy2.robjects as robjects
from rpy2.robjects import pandas2ri

pandas2ri.activate()


def load_rds(f: str) -> pd.DataFrame:
    """
    Loads the RDS files.

    Parameters
    ----------
    f: str
        path of file to load

    Returns
    -------
    pd.DataFrame

    """

    readRDS = robjects.r['readRDS']
    df = readRDS(f)
    df = pandas2ri.rpy2py(df)

    return df


lucid_data = {
    'wave0': load_rds('lucid/national_wave0.rds'),
    'wave0_alters': load_rds('lucid/national_alters_wave0.rds'),
    'wave1': load_rds('lucid/national_wave1.rds'),
    'wave1_alters': load_rds('lucid/national_alters_wave1.rds'),
    'wave2': load_rds('lucid/national_wave2.rds'),
    'wave2_alters': load_rds('lucid/national_alters_wave2.rds'),
    'wave3': load_rds('lucid/national_wave3.rds'),
    'wave3_alters': load_rds('lucid/national_alters_wave3.rds'),
    'wave4': load_rds('lucid/national_wave4.rds'),
    'wave4_alters': load_rds('lucid/national_alters_wave4.rds'),
    'wave5': load_rds('lucid/national_wave5.rds'),
    'wave5_alters': load_rds('lucid/national_alters_wave5.rds')
}


def agecat_remap(x: float) -> str | None:
    """
    Remaps a numeric vector x to categorical age

    Parameters
    ----------
    x: list, vector, anything that can be passed to map

    Returns
    -------
    str or None

    """
    if x is None:
        return None
    elif np.isnan(x):
        return None
    elif x < 18:
        return '[0,18)'
    elif x < 25:
        return '[18,25)'
    elif x < 35:
        return '[25,35)'
    elif x < 45:
        return '[35,45)'
    elif x < 55:
        return '[45,55)'
    elif x < 65:
        return '[55,65)'
    elif x < 75:
        return '[65,75)'
    elif x < 85:
        return '[75,85)'
    elif x >= 85:
        return '>85'
    else:
        return None


def gender_remap(x: str) -> str | None:
    """
    Remaps F, M to Female, Male

    Returns
    -------
    str or None

    """

    if x is None:
        return None
    elif x == 'F':
        return 'Female'
    elif x == 'M':
        return 'Male'
    else:
        return None


# def bool_remap(x: str) -> bool | None:
#     """
#     POLYMOD data comes in TRUE/FALSE booleans, not pythonic True/False.
#     Parameters
#     ----------
#     x: str
#
#     Returns
#     -------
#     bool
#
#     """
#
#     if not isinstance(x, str):
#         raise ValueError("x must be a string")
#     if x is None:
#         return None
#     elif x == 'TRUE':
#         return True
#     elif x == 'FALSE':
#         return False
#     else:
#         return None


def load_polymod(path: str = 'POLYMOD') -> pd.DataFrame:
    """
    Loads POLYMOD data for deriving child-child contacts.

    Parameters
    ----------
    path: str

    Returns
    -------
    pd.DataFrame

    """

    # Load contacts
    polymod_participants = pd.read_csv(path + "/2008_Mossong_POLYMOD_participant_common.csv")
    polymod_contacts = pd.read_csv(path + "/2008_Mossong_POLYMOD_contact_common.csv")
    polymod_households = pd.read_csv(path + '/2008_Mossong_POLYMOD_hh_common.csv')

    # Subset nonhh contacts and count
    polymod_contacts = polymod_contacts.loc[polymod_contacts['cnt_home'] == False].copy(deep=True)
    polymod_contacts = polymod_contacts.groupby('part_id')['part_id'].count().rename('num_cc_nonhh')

    # Merge num_cc_nonhh
    polymod = pd.merge(polymod_participants, polymod_contacts.to_frame(), on='part_id', how='left')
    polymod['num_cc_nonhh'] = polymod['num_cc_nonhh'].fillna(0)

    # Merge hhsize
    polymod = pd.merge(polymod, polymod_households, on='hh_id')

    # Rename columns to match BICS
    polymod = polymod.rename(columns={'part_age': 'age', 'part_gender': 'gender',
                                      'part_id': 'rid', 'hh_size': 'hhsize'}).copy(deep=True)
    polymod['rid'] = ['polymod_' + str(x) for x in polymod['rid']]

    # Remap gender
    polymod['gender'] = polymod['gender'].apply(gender_remap)

    # Remap age
    polymod['age'] = polymod['age'].apply(agecat_remap)

    return polymod


def sim_pop(n_households: int,
            df: pd.DataFrame,
            fill_polymod: bool = True) -> Population:
    """
    Simulates n households from df.

    Does not include of size n>6

    Steps:
        - Generate hhsize distribution from df
        - Sample f(n) individuals of hhsize n from df
        - For each hh where n > 1, look at the reported hh members for the first person, and sample
          the closest person in the dataset that matches age, gender, and hhsize
        - Repeat through n=6


    Parameters
    ----------
    n_households: int
        number of households to simulate
    df: pd.DataFrame
        BICS wave to resample from
    fill_polymod : bool
        if True, will draw children from the polymod survey.

    Returns
    -------
    """

    polymod = load_polymod()
    polymod = polymod[polymod['age'] == '[0,18)']

    # Bin age
    df['age'] = df['age'].apply(agecat_remap)
    df['resp_hh_roster#1_1_1'] = df['resp_hh_roster#1_1_1'].apply(agecat_remap)
    df['resp_hh_roster#1_2_1'] = df['resp_hh_roster#1_2_1'].apply(agecat_remap)
    df['resp_hh_roster#1_3_1'] = df['resp_hh_roster#1_3_1'].apply(agecat_remap)
    df['resp_hh_roster#1_4_1'] = df['resp_hh_roster#1_4_1'].apply(agecat_remap)
    df['resp_hh_roster#1_5_1'] = df['resp_hh_roster#1_5_1'].apply(agecat_remap)
    df['resp_hh_roster#2_1'] = df['resp_hh_roster#2_1'].apply(gender_remap)
    df['resp_hh_roster#2_2'] = df['resp_hh_roster#2_2'].apply(gender_remap)
    df['resp_hh_roster#2_3'] = df['resp_hh_roster#2_3'].apply(gender_remap)
    df['resp_hh_roster#2_4'] = df['resp_hh_roster#2_4'].apply(gender_remap)
    df['resp_hh_roster#2_5'] = df['resp_hh_roster#2_5'].apply(gender_remap)

    # Generate weighted distribution of household sizes
    hhsize_dist = df.groupby('hhsize')['weight_pooled'].sum()[0:6]
    hhsize_dist.sort_index(inplace=True)
    hhsize_dist = round(n_households * (hhsize_dist / sum(hhsize_dist))).to_dict()

    hh_list = []

    # Randomly sample a respondent of the corresponding hhsize
    for n, f_n in hhsize_dist.items():
        # Take df, subset to respondents living in households of size n
        # Sample with replacement f_n times with corresponding weights
        bs_df = df[df['hhsize'] == n].sample(n=int(f_n), replace=True, weights='weight_pooled')

        # Append to household list a household with a member living in size n
        for i in range(int(f_n)):
            hh_list.append(Household(hhsize=n, head=bs_df.iloc[i,].to_dict()))

    # Now, cycle through each household in hh_list and grow it by
    # re-sampling according to the first member's reported hhmembers
    for i in range(len(hh_list)):
        hh = hh_list[i]
        if hh.hhsize == 1:
            continue

        head_data = {
            'age': hh.head['age'],
            'gender': hh.head['gender'],
            'hhr_1_age': hh.head['resp_hh_roster#1_1_1'],
            'hhr_2_age': hh.head['resp_hh_roster#1_2_1'],
            'hhr_3_age': hh.head['resp_hh_roster#1_3_1'],
            'hhr_4_age': hh.head['resp_hh_roster#1_4_1'],
            'hhr_5_age': hh.head['resp_hh_roster#1_5_1'],
            'hhr_1_gender': hh.head['resp_hh_roster#2_1'],
            'hhr_2_gender': hh.head['resp_hh_roster#2_2'],
            'hhr_3_gender': hh.head['resp_hh_roster#2_3'],
            'hhr_4_gender': hh.head['resp_hh_roster#2_4'],
            'hhr_5_gender': hh.head['resp_hh_roster#2_5']
        }

        for hhr in range(1, 6):
            age = head_data['hhr_' + str(hhr) + '_age']
            gen = head_data['hhr_' + str(hhr) + '_gender']

            if age is None or gen is None:
                print("HH Member Age or Gender is None")
                continue

            if age != '[0,18)':
                print("HH size:", hh.hhsize, "HH Member age:", age, ", HH Member gender: ", gen)

                # Eligible population from BICS that matches the hhsize, age, and gender of the respondent's hh member
                df_sub = df[
                    (df['hhsize'] == hh.hhsize) &
                    (df['age'] == age) &
                    (df['gender'] == gen)
                    ]

                # Print if there is no suitable member
                if len(df_sub.index) == 0:
                    print("No suitable adult hh member")
                    continue

                new_member = df_sub.sample(1, weights='weight_pooled').iloc[0, :].to_dict()
                print(new_member)

                hh_list[i].add_member(new_member)

            elif age == '[0,18)' and not fill_polymod:
                print("Household member is a child and fill_polymod=False")

            elif age == '[0,18)' and fill_polymod:
                new_member = polymod[
                                 (polymod['age'] == age) & (polymod['gender'] == gen)
                             ].sample(1).iloc[0, :].to_dict()
                new_member['ethnicity'] = str(None)
                print(new_member)

                hh_list[i].add_member(new_member)

            else:
                continue

    pop = Population()
    pop.add_household(hh_list)

    return (pop)


def sim_individuals(n, df, weights='weight_pooled'):
    """
    Simulates a population of size n from df. Does not account for hh structure

    Parameters
    ---------
    n: int
        Size of population
    df: pd.dataframe
    weights: str
        column name of weights


    Returns
    -------
    Population

    """

    def f(x):
        return {'ethnicity': x['ethnicity'],
                'age': int(x['age']),
                'gender': x['gender'],
                'num_cc_nonhh': 0 if pd.isna(x['num_cc_nonhh']) else int(x['num_cc_nonhh'])}

    pop = Population()
    df = df.sample(n=n, weights=weights, replace=True)

    for i, r in df.iterrows():
        pop.add_node(**f(r))

    return pop


if __name__ == "__main__":
    sim_pop(100, lucid_data['wave4'])
    pdb.set_trace()