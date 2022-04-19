"""
Loads BICS data
"""
from Population import Population
import pandas as pd

# Load in the data
df_all_waves = pd.read_csv("df_all_waves.csv")
df_alters_all_waves = pd.read_csv("df_alters_all_waves.csv")


def sim_pop(n, weights='weight_pooled'):
    """
    Simulates a population of size n from df

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
        return {'id': x['rid'],
                'ethnicity': x['ethnicity'],
                'age': int(x['age']),
                'gender': x['gender'],
                'num_cc': 0 if pd.isna(x['num_cc_nonhh']) else int(x['num_cc_nonhh']),
                'hhsize': 0 if pd.isna(x['hhsize']) else int(x['hhsize'])}

    pop = Population()
    df = df_all_waves.sample(n=n, weights=weights, replace=True)

    for i, r in df.iterrows():
        pop.add_node(**f(r))

    return pop

