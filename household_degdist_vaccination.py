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


def split_list(l: list[str]) -> tuple[list[str], list[str]]:
    """
    Takes a list and splits it in half, returning a tuple of both halves. If the list is odd length, will silently
    drop the last element.

    Parameters
    ----------
    l: list

    Returns
    -------
    tuple

    """

    l_len = len(l)

    if l_len % 2 == 0:
        l = l[0:(l_len - 1)]
        l_len = len(l)

    return deepcopy(l[:l_len // 2]), deepcopy(l[l_len // 2:])


def transmit_hh(pop: Population, beta: dict, E_dist, I_dist, ve) -> None:
    """
    Transmits within the household

    Parameters
    ----------
    pop: Population
    beta: dict
        Probability of transmission, conditional on mask usage
    E_dist, I_dist:
        distribution function of hours exposed and infectious, in hours

    Returns
    -------
    Modifies pop in place, returns nx.Graph for anim_list

    """

    # Loop through each hhid and transmit through all the tuples
    for u, v, d in pop.edges.data():
        if d['household'] and (u in pop.node_ids_I or v in pop.node_ids_I) and bernoulli(beta[d['protection']]):
            t = (u, v,E_dist.rvs(), I_dist.rvs())
            pop.transmit([t])


    # pop.transmit(
    #     [
    #         # Tuple format: (node1, node2, number of hrs exposed, num hrs sick
    #         (u, v, E_dist.rvs(), I_dist.rvs())
    #         # Get all edges
    #         for u, v, d in pop.edges.data()
    #         # If they are in the same household
    #         if d['household']
    #             # Flip a coin with p chosen if they wear masks
    #             and bernoulli(beta[d['protection']])
    #             # And at least one of them is sick
    #             and (u in pop.node_ids_I or v in pop.node_ids_I)
    #     ]
    # )
    pop.add_history()
    pop.decrement()

    return deepcopy(pop.G)


def transmit_daytime(pop: Population, beta: dict, p_mask: float, E_dist, I_dist, ve) -> None:
    """
    Transmits during each hour of the workday
    Assume a 10 hour day. Probability of a node having a contact each hour is num_cc_nonhh/10.
    Create subpopulation of nodes that have any contact in a given hour and join them to each other.

    Returns
    -------
    Modifies pop in place, returns nx.Graph for anim_list

    """

    # Create a subpopulation of people having contacts every hour
    # by bootstrapping the population. Number of times a node appears in the population
    # poisson sampling with lambda=num_cc_nonhh/10
    subpop = list(chain(*[
        [n[0]] * rep
        for n in pop.G.nodes.data('num_cc_nonhh')
        for rep in range(poisson.rvs(n[1] / 10))
    ]))
    # Shuffle this list and split into two sublists
    shuffle(subpop)
    subpop = split_list(subpop)

    # Add edges
    # Probability that node is wearing a mask is p_mask.
    pop.add_edges(
        [(n1, n2, {'protection': bernoulli.rvs(p_mask), 'household': False})
         for n1, n2 in zip(*subpop) if n1 != n2]
    )

    # Transmit
    for u, v, d in pop.edges.data():
        if not d['household'] and \
                (u in pop.node_ids_I or v in pop.node_ids_I):

            # If either node is vaccinated:
            if u in pop.node_ids_V1 or v in pop.node_ids_V1:
                if bernoulli(beta[d['protection']]) and bernoulli(1-ve["V1"]):
                    t = (u, v, E_dist.rvs(), I_dist.rvs())
                    pop.transmit([t])
            elif u in pop.node_ids_V2 or v in pop.node_ids_V2:
                if bernoulli(beta[d['protection']]) and bernoulli(1-ve["V2"]):
                    t = (u, v, E_dist.rvs(), I_dist.rvs())
                    pop.transmit([t])
            elif bernoulli(beta[d['protection']]):
                t = (u, v, E_dist.rvs(), I_dist.rvs())
                pop.transmit([t])


    pop.add_history()
    pop.decrement()
    pop.remove_edges(keep_hh=True)

    return deepcopy(pop.G)

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

    print(n_remaining_2)
    print(len(v2_prior))
    while n_remaining_2 > 0 and len(v2_prior) > 0:
        # The node that we're vaccinating

        random.shuffle(v2_prior)
        nid = v2_prior.pop()

        print(nid)

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
        ve: dict[str, float] = {'V1': 0.75, 'V2': 0.95, False: 0}
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

    anim_list = [pop.G]

    if n_days is None:
        n_days = np.Inf

    day = 0

    # Randomly assing priority levels -1-2
    for nid in pop.node_ids:
        pop.set_vax_priority(nid, random.choice([-1, 0, 1, 2]))

    while (len(pop.node_ids_I) + len(pop.node_ids_E) > 0) and (n_days > 0):

        # Morning
        for hour in range(0, 8):
            print("Day", day, "Hour", hour)
            anim_list.append(transmit_hh(pop, beta, E_dist, I_dist, ve))

        # Distribute vaccines
        distribute_vax(pop, 10)

        # Workday
        for hour in range(8, 18):
            print("Day", day, "Hour", hour)
            anim_list.append(transmit_daytime(pop, beta, p_mask, E_dist, I_dist, ve))

        # Evening
        for hour in range(18, 24):
            print("Day", day, "Hour", hour)
            anim_list.append(transmit_hh(pop, beta, E_dist, I_dist, ve))

        day += 1
        n_days -= 1

    return pop, pop.process_history(), anim_list


if __name__ == "__main__":
    pop, history, anim_list = household_mixing_w_degree_dist(100, initial_sick=1)


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

    plt.plot((history == 'S').sum(axis=0), color=coldict['S'], label='S')
    plt.plot((history == 'E').sum(axis=0), color=coldict['E'], label='E')
    plt.plot((history == 'I').sum(axis=0), color=coldict['I'], label='I')
    plt.plot((history == 'R').sum(axis=0), color=coldict['R'], label='R')
    plt.plot((history == 'D').sum(axis=0), color=coldict['D'], label='D')
    plt.plot((history == 'V1').sum(axis=0), color=coldict['V1'], label='V1')
    plt.plot((history == 'V2').sum(axis=0), color=coldict['V2'], label='V2')
    plt.legend()
    plt.savefig("testanim.png")

    run_animation = False
    fig, ax = plt.subplots(ncols=2, figsize=(12, 8))
    pos = nx.spring_layout(anim_list[0], k=2 / np.sqrt(len(anim_list[0].nodes)))

    t0 = time()

    if run_animation:
        def animate(i):
            if i > 0:
                print('\r', round(i / len(anim_list) * 100), '%:',
                      round((time() - t0) - (time() - t0) / (i / len(anim_list))),
                      'seconds estimated remaining', end='')

            ax[0].clear()
            ds = [v for k, v in anim_list[i].nodes('disease_status')]
            nc = [coldict[d] for d in ds]
            ec = [edgedict[v] for k, v in anim_list[i].edges('household')]
            nx.draw(anim_list[i], pos=pos, node_color=nc, style=ec, node_size=5, ax=ax[0])
            ax[0].set_title("Day " + str(i // 24) + " hour " + str(i % 24))
            ax[0].set_xlim(-1.5, 1.5)
            ax[0].set_ylim(-1.5, 1.5)

            ax[1].clear()
            ax[1].plot((history.iloc[:, :i] == 'S').sum(axis=0), color=coldict['S'])
            ax[1].plot((history.iloc[:, :i] == 'E').sum(axis=0), color=coldict['E'])
            ax[1].plot((history.iloc[:, :i] == 'I').sum(axis=0), color=coldict['I'])
            ax[1].plot((history.iloc[:, :i] == 'R').sum(axis=0), color=coldict['R'])
            ax[1].plot((history.iloc[:, :i] == 'V1').sum(axis=0), color=coldict['V1'])
            ax[1].plot((history.iloc[:, :i] == 'V2').sum(axis=0), color=coldict['V2'])
            ax[1].set_xlim([0, len(history.columns)])


        anim = animation.FuncAnimation(fig, animate, frames=len(anim_list), interval=200)

        writervideo = animation.FFMpegWriter(fps=60)
        anim.save("testanim.gif")
        plt.close()
