import random
import numpy as np
import pandas as pd
from scipy.stats import randint
from matplotlib import pyplot as plt, animation
from load_data import sim_pop
from time import time
import igraph as ig
from scipy.stats import bernoulli, poisson
from itertools import cycle
import pdb


def household_mixing_w_degree_dist(
        n_hh: int = 1000,
        initial_sick: int = 1,
        n_days: int = None,
        beta=None,
        p_mask: float = 0.8,
        E_dist=randint(2 * 24, 4 * 24),
        I_dist=randint(4 * 24, 7 * 24),
        ve=None,
        n_vax_daily: int = 10,
        full_history=False,
        mu=None
) -> tuple:
    """

    Parameters
    ----------
    full_history
    n_vax_daily
    ve: dict[str, float]
        Vaccine efficacy
    n_hh: int
        Number of households
    initial_sick: int
        Number of intially sick individuals. Randomly chosen from pop.
    n_days: int
        Stop simulation after a number of days. if None (default), will terminate
        when number of exposed or infected individuals is 0.
    beta: dict[bool,float]
        Probability of transmission conditional on mask usage.
    p_mask:
        Probability that both nodes in an edge are wearing a mask.
    E_dist, I_dist:
        Distribution of time exposed and infectious, in hours. Can either be single integer,
        list of integers, or distribution function in the format like, poisson(3*24)

    Returns
    -------

    """

    if ve is None:
        ve = {'V1': 0.75, 'V2': 0.95, False: 0}
    if beta is None:
        beta = {True: 0.01, False: 0.1}
    if mu is None:
        mu = {
            '[0,18)': 0.0001,
            '[18,25)': 0.0001,
            '[25,35)': 0.0001,
            '[35,45)': 0.001,
            '[45,55)': 0.001,
            '[55,65)': 0.001,
            '[65,75)': 0.01,
            '[75,85)': 0.01,
            '>85': 0.01
        }

    # Define generators for RVs
    E_dist_gen = cycle(E_dist.rvs(n_hh * 10000))
    I_dist_gen = cycle(I_dist.rvs(n_hh * 10000))
    beta_gen = {True: cycle(bernoulli.rvs(beta[True], size=n_hh * 10000)),
                False: cycle(bernoulli.rvs(beta[False], size=n_hh * 10000))}
    ve_gen = {False: cycle([1]),
              'V1': cycle(bernoulli.rvs(1 - ve['V1'], size=n_hh * 10000)),
              'V2': cycle(bernoulli.rvs(1 - ve['V2'], size=n_hh * 10000))}
    p_mask_gen = cycle(bernoulli.rvs(p_mask, size=10000))

    stubs_gen = dict()
    # Assume the max number of stubs we can have in a single timestep is 100
    for s in range(100):
        stubs_gen[s] = cycle(poisson.rvs(s, size=10000))

    for k, v in mu.items():
        mu[k] = cycle(bernoulli.rvs(v, size=10000))

    pop = sim_pop(n_hh, wave=4)
    pop.set_mu(mu)

    for _ in range(initial_sick):
        pop.set_sick(random.choice(pop.node_ids))

    if n_days is None:
        n_days = np.Inf

    day = 0

    # Randomly assing priority levels -1-2
    for nid in pop.node_ids:
        pop.set_vax_priority(nid, random.choice([-1, 0, 1, 2]))

    pop.connect_hh_edges()

    while (len(pop.node_ids_I) + len(pop.node_ids_E) > 0) and (n_days > 0):

        # Morning
        for hour in range(0, 8):
            pop.transmit_hh(beta_gen, E_dist_gen, I_dist_gen, ve_gen)
            if full_history: pop.add_history()

        # Distribute vaccines
        pop.distribute_vax(n_vax_daily)

        # Workday
        for hour in range(8, 18):
            pop.transmit_daytime(beta_gen, p_mask_gen, E_dist_gen, I_dist_gen, ve_gen, stubs_gen)
            if full_history: pop.add_history()

        pop.connect_hh_edges()

        # Evening
        for hour in range(18, 24):
            pop.transmit_hh(beta_gen, E_dist_gen, I_dist_gen, ve_gen)
            if full_history: pop.add_history()

        if not full_history: pop.add_history()
        day += 1
        n_days -= 1

        # Print status
        print("\r", "Day", day,
              "S:", len(pop.node_ids_S),
              "E:", len(pop.node_ids_E),
              "I:", len(pop.node_ids_I),
              "R:", len(pop.node_ids_R),
              "D:", len(pop.node_ids_D),
              "V1:", len(pop.node_ids_V1),
              "V2:", len(pop.node_ids_V2),
              end="")

    return pop


if __name__ == "__main__":

    n_hh = 1000
    mu = {
        '[0,18)': 0.0001,
        '[18,25)': 0.0001,
        '[25,35)': 0.0001,
        '[35,45)': 0.001,
        '[45,55)': 0.001,
        '[55,65)': 0.001,
        '[65,75)': 0.01,
        '[75,85)': 0.01,
        '>85': 0.01
    }

    run_graphic = True
    run_animation = False
    run_check = False

    pop = household_mixing_w_degree_dist(n_hh,
                                         initial_sick=5,
                                         n_vax_daily=n_hh // 20,
                                         full_history=(run_animation | run_check),
                                         mu=mu)

    disease_status, vaccine_status, edges_history = pop.process_history()


    coldict = {'S': 'blue', 'E': 'purple', 'I': 'red', 'R': 'green', 'D': 'black', 'V1': 'peru', 'V2': 'saddlebrown'}
    edgedict = {True: '-', False: '--'}

    # Consistency check to make sure that nodes are, on average, getting the right number of nonhh contacts per day
    is_daytime = []
    if run_check:
        degdf = pd.DataFrame()
        for i in range(len(pop.history)):
            # Get hour in day
            hr = i % 24

            if hr > 8 and hr < 18:
                is_daytime.append(True)
            else:
                is_daytime.append(False)

            g = ig.Graph.DataFrame(edges=pop.history[i]['edges'], vertices=pop.history[i]['nodes'], directed=False)
            degdf = pd.concat([degdf, pd.Series(g.degree())], axis=1)

        # Get sequences of hh hours and nonhh hours
        daytime_total = degdf.loc[:, is_daytime].sum(axis=1)

        plt.scatter(pd.Series(pop.G.vs['num_cc_nonhh']), daytime_total/(len(pop.history)/24))
        plt.show()

        print(daytime_total)

    if run_graphic:
        plt.plot((disease_status == 'S').sum(axis=0), color=coldict['S'], label='S')
        plt.plot((disease_status == 'E').sum(axis=0), color=coldict['E'], label='E')
        plt.plot((disease_status == 'I').sum(axis=0), color=coldict['I'], label='I')
        plt.plot((disease_status == 'R').sum(axis=0), color=coldict['R'], label='R')
        plt.plot((disease_status == 'D').sum(axis=0), color=coldict['D'], label='D')
        plt.plot((vaccine_status == 'V1').sum(axis=0), color=coldict['V1'], label='V1')
        plt.plot((vaccine_status == 'V2').sum(axis=0), color=coldict['V2'], label='V2')
        plt.legend()
        plt.savefig("testanim.png")

    if run_animation:

        fig, ax = plt.subplots(ncols=2, figsize=(12, 8))
        pop.connect_hh_edges()
        ig.plot(pop.G, target=ax[0])
        # pos = nx.spring_layout(pop.G, k=2 / np.sqrt(len(pop.G.nodes)))
        pos = pop.G.layout("fr")
        t0 = time()


        def animate(i):
            if i > 0:
                print('\r', round(i / len(pop.history) * 100), '%:',
                      round((time() - t0) - (time() - t0) / (i / len(pop.history))),
                      'seconds estimated remaining', end='')

            ax[0].clear()

            # Recreate the igraph from the dumped node and edge lists
            g = ig.Graph.DataFrame(edges=pop.history[i]['edges'], vertices=pop.history[i]['nodes'], directed=False)
            colorlist = [coldict[d] for d in g.vs['disease_status']]
            ig.plot(g, target=ax[0], vertex_color=colorlist, layout=pos, vertex_size=1)
            ax[0].set_title("Day " + str(i // 24) + " hour " + str(i % 24))
            # ax[0].set_xlim(-1.5, 1.5)
            # ax[0].set_ylim(-1.5, 1.5)

            ax[1].clear()
            ax[1].plot((disease_status.iloc[:, :i] == 'S').sum(axis=0), color=coldict['S'], label='S')
            ax[1].plot((disease_status.iloc[:, :i] == 'E').sum(axis=0), color=coldict['E'], label='E')
            ax[1].plot((disease_status.iloc[:, :i] == 'I').sum(axis=0), color=coldict['I'], label='I')
            ax[1].plot((disease_status.iloc[:, :i] == 'R').sum(axis=0), color=coldict['R'], label='R')
            ax[1].plot((vaccine_status.iloc[:, :i] == 'V1').sum(axis=0), color=coldict['V1'], label='V1')
            ax[1].plot((vaccine_status.iloc[:, :i] == 'V2').sum(axis=0), color=coldict['V2'], label='V2')
            ax[1].set_xlim([0, len(pop.history)])
            ax[1].legend()


        anim = animation.FuncAnimation(fig, animate, frames=len(pop.history), interval=50)

        writervideo = animation.FFMpegWriter(fps=60)
        anim.save("testanim.gif")
        plt.close()
