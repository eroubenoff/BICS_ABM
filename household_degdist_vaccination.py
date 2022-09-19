import pdb
import random
import networkx as nx
import numpy as np
from scipy.stats import bernoulli, poisson, randint
from matplotlib import pyplot as plt, animation
from load_data import sim_pop, lucid_data
from random import shuffle
from copy import deepcopy
from itertools import chain
from time import time
from Population import Population
import pickle









def distribute_vax(pop: Population, n_daily: int, time_until_v2: int = 25 * 24):
    """
    Distributes n_daily doses of the vaccine to the population, by priority
    group
    Parameters
    ----------
    pop: Population
        The object to modify
    n_daily: int
        Number of vaccines distributed every day

    Returns
    -------

    """


    n_remaining = n_daily
    n_remaining_2 = n_daily

    def invert_dict(d):
        ret = dict()
        for key, value in d.items():
            if value in ret:
                ret[value].append(key)
            else:
                ret[value] = [key]

        return ret

    # Invert the dict of priorities
    v1_prior = pop.vaccine_priority
    v1_prior = invert_dict(v1_prior)
    del v1_prior[-1]



    # V1
    while n_remaining > 0:
        # The node that we're vaccinating
        try :
            max_prior = max(v1_prior.keys())
            if (len(v1_prior[max_prior]) == 0):
                del v1_prior[max_prior]
            max_prior = max(v1_prior.keys())
        except ValueError:
            break

        random.shuffle(v1_prior[max_prior])
        nid = v1_prior[max_prior].pop()

        print("Node", nid, "got the first dose of vaccine")

        # Set the node's status to V1
        pop.V1(nid, time_until_v2)


        n_remaining -= 1


    # V2
    v2_prior = pop.time_until_v2
    v2_prior = invert_dict(v2_prior)
    try:
        v2_prior = v2_prior[0]
        print(v2_prior)
    except KeyError:
        return deepcopy(pop.G)

    while n_remaining_2 > 0 and len(v2_prior) > 0:
        # The node that we're vaccinating

        random.shuffle(v2_prior)
        nid = v2_prior.pop()

        # print(nid)

        print("Node", nid, "got the second dose of vaccine")

        # Set the node's status to V1
        pop.V2(nid)

        n_remaining_2 -= 1

    return deepcopy(pop.G)






def household_mixing_w_degree_dist(
        n_hh: int = 1000,
        initial_sick: int = 1,
        n_days: int = None,
        beta: dict[bool, float] = {True: 0.01, False: 0.1},
        p_mask: float = 0.8,
        E_dist=randint(2 * 24, 4 * 24),
        I_dist=randint(4 * 24, 57 * 24),
        ve: dict[str, float] = {'V1': 0.75, 'V2': 0.95, False: 0},
        n_vax_daily: int = 10
) -> tuple:
    """

    Parameters
    ----------
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

    pop = sim_pop(n_hh, lucid_data['wave4'])

    for _ in range(initial_sick):
        pop.set_sick(random.choice(pop.node_ids))

    # anim_list = [pop.G]

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
            print("Day", day, "Hour", hour)
            pop.transmit_hh(beta, E_dist, I_dist, ve)

        # Distribute vaccines
        distribute_vax(pop, n_vax_daily)

        # Workday
        for hour in range(8, 18):
            print("Day", day, "Hour", hour)
            pop.transmit_daytime(beta, p_mask, E_dist, I_dist, ve)

        pop.connect_hh_edges()

        # Evening
        for hour in range(18, 24):
            print("Day", day, "Hour", hour)
            pop.transmit_hh(beta, E_dist, I_dist, ve)

        day += 1
        n_days -= 1

    return pop


if __name__ == "__main__":

    pop = household_mixing_w_degree_dist(50, initial_sick=1, n_vax_daily = 5)

    disease_status, vaccine_status, edges_history = pop.process_history()

    # Print stats
    print("Susceptible: ", len(pop.node_ids_S), "of", len(pop.node_ids))
    print("Infected: ", len(pop.node_ids_I), "of", len(pop.node_ids))
    print("Recovered: ", len(pop.node_ids_R), "of", len(pop.node_ids))
    print("Deaths: ", len(pop.node_ids_D), "of", len(pop.node_ids))
    print("Vaccinated, dose 1: ", len(pop.node_ids_V1), "of", len(pop.node_ids))
    print("Vaccinated, dose 2: ", len(pop.node_ids_V2), "of", len(pop.node_ids))
    # print("Breakthrough cases: ", len(pop.node_ids_V2 in pop.node_ids_R), "of", len(pop.node_ids_V2))

    coldict = {'S': 'blue', 'E': 'purple', 'I': 'red', 'R': 'green', 'D': 'black', 'V1': 'peru', 'V2': 'saddlebrown'}
    edgedict = {True: '-', False: '--'}


    plt.plot((disease_status == 'S').sum(axis=0), color=coldict['S'], label='S')
    plt.plot((disease_status == 'E').sum(axis=0), color=coldict['E'], label='E')
    plt.plot((disease_status == 'I').sum(axis=0), color=coldict['I'], label='I')
    plt.plot((disease_status == 'R').sum(axis=0), color=coldict['R'], label='R')
    plt.plot((disease_status == 'D').sum(axis=0), color=coldict['D'], label='D')
    plt.plot((vaccine_status == 'V1').sum(axis=0), color=coldict['V1'], label='V1')
    plt.plot((vaccine_status == 'V2').sum(axis=0), color=coldict['V2'], label='V2')
    plt.legend()
    plt.savefig("testanim.png")

    run_animation = True

    if run_animation:

        fig, ax = plt.subplots(ncols=2, figsize=(12, 8))
        pop.connect_hh_edges()
        pos = nx.spring_layout(pop.G, k=2 / np.sqrt(len(pop.G.nodes)))
        t0 = time()


        def animate(i):
            if i > 0:
                print('\r', round(i / len(edges_history) * 100), '%:',
                      round((time() - t0) - (time() - t0) / (i / len(edges_history))),
                      'seconds estimated remaining', end='')

            ax[0].clear()
            # Set edges
            pop.G.remove_edges_from(list(pop.G.edges()))
            pop.G.add_edges_from(edges_history[i])

            nc = [coldict[d] for d in disease_status.iloc[:,i].tolist()]
            # ec = [edgedict[v] for k, v in anim_list[i].edges('household')]
            # nx.draw(anim_list[i], pos=pos, node_color=nc, style=ec, node_size=5, ax=ax[0])
            nx.draw(pop.G, pos=pos, node_color=nc, node_size=5, ax=ax[0])
            ax[0].set_title("Day " + str(i // 24) + " hour " + str(i % 24))
            ax[0].set_xlim(-1.5, 1.5)
            ax[0].set_ylim(-1.5, 1.5)

            ax[1].clear()
            ax[1].plot((disease_status.iloc[:,:i] == 'S').sum(axis=0), color=coldict['S'])
            ax[1].plot((disease_status.iloc[:,:i] == 'E').sum(axis=0), color=coldict['E'])
            ax[1].plot((disease_status.iloc[:,:i] == 'I').sum(axis=0), color=coldict['I'])
            ax[1].plot((disease_status.iloc[:,:i] == 'R').sum(axis=0), color=coldict['R'])
            ax[1].plot((vaccine_status.iloc[:,:i] == 'V1').sum(axis=0), color=coldict['V1'])
            ax[1].plot((vaccine_status.iloc[:,:i] == 'V2').sum(axis=0), color=coldict['V2'])
            ax[1].set_xlim([0, len(disease_status.columns)])


        anim = animation.FuncAnimation(fig, animate, frames=len(edges_history), interval=50)

        writervideo = animation.FFMpegWriter(fps=60)
        anim.save("testanim.gif")
        plt.close()
