import pdb
import random

import networkx as nx
import numpy as np
from scipy.stats import bernoulli, poisson
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


def transmit_hh(pop: Population, beta: dict) -> None:
    """
    Transmits within the household

    Parameters
    ----------
    pop: Population
    beta: dict
        Probability of transmission, conditional on mask usage

    Returns
    -------
    None; modifies object in place.

    """

    pop.transmit(
        [
            (u, v, random.randint(2 * 24, 4 * 24), random.randint(4 * 24, 7 * 24))
            for u, v, d in pop.edges.data()
            if d['household'] and bernoulli(beta[d['protection']])
        ]
    )
    anim_list.append(deepcopy(pop.G))
    pop.add_history()
    pop.decrement()

    return

def transmit_daytime(pop: Population, beta: dict, p_mask: float) -> None:
    """
    Transmits during each hour of the workday
    Assume a 10 hour day. Probability of a node having a contact each hour is num_cc_nonhh/10.
    Create subpopulation of nodes that have any contact in a given hour and join them to each other.

    Returns
    -------
    None; modifies objects from outer scope

    """

    # Create subpop by poisson sampling waith lambda=num_cc_nonhh/10
    subpop = list(chain(*[
        [n[0]] * rep
        for n in pop.G.nodes.data('num_cc_nonhh')
        for rep in range(poisson.rvs(n[1] / 10))
    ]))
    shuffle(subpop)
    subpop = split_list(subpop)

    # Add edges and transmit
    pop.add_edges(
        [(n1, n2, {'protection': bernoulli.rvs(p_mask), 'household': False})
         for n1, n2 in zip(*subpop) if n1 != n2]
    )
    pop.transmit(
        [
            (u, v, random.randint(2 * 24, 4 * 24), random.randint(4 * 24, 7 * 24))
            for u, v, d in pop.edges.data()
            if not d['household'] and bernoulli(beta[d['protection']])
        ]
    )

    anim_list.append(deepcopy(pop.G))
    pop.add_history()
    pop.decrement()
    pop.remove_edges(keep_hh=True)


def household_mixing_w_degree_dist(
        n_hh: int = 1000,
        initial_sick: int = 1,
        n_days: int = None,
        beta: dict[bool, float] = {True: 0.01, False: 0.1},
        p_mask: float = 0.8
) -> tuple:
    pop = sim_pop(n_hh, lucid_data['wave4'])

    for _ in range(initial_sick):
        pop.set_sick(random.choice(pop.node_ids))

    anim_list = [pop.G]

    if n_days is None:
        n_days = np.Inf

    day = 0

    while (len(pop.node_ids_I) + len(pop.node_ids_E) > 0) and (n_days > 0):

        # Morning
        # pop.connect_hh_edges()
        for hour in range(0, 8):
            print("Day", day, "Hour", hour)
            transmit_hh(pop, beta)

        # Workday
        for hour in range(8, 18):
            print("Day", day, "Hour", hour)
            transmit_daytime(pop, beta, p_mask)

        # Evening
        # pop.connect_hh_edges()
        for hour in range(18, 24):
            print("Day", day, "Hour", hour)
            transmit_hh(pop, beta)

        day += 1
        n_days -= 1

    return pop, pop.process_history(), anim_list


if __name__ == "__main__":
    pop, history, anim_list = household_mixing_w_degree_dist(100, initial_sick=1)

    coldict = {'S': 'blue', 'E': 'purple', 'I': 'red', 'R': 'green'}
    edgedict = {True: '-', False: '--'}

    plt.plot((history == 'S').sum(axis=0))
    plt.plot((history == 'E').sum(axis=0))
    plt.plot((history == 'I').sum(axis=0))
    plt.plot((history == 'R').sum(axis=0))

    plt.savefig("testanim.png")

    run_animation = True
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
            ax[1].set_xlim([0, len(history.columns)])


        # np = Graph(anim_list[0], scale=(5,5), node_layout='spring', node_layout_kwargs=dict(k=0.1)).node_positions

        # def animate(i):
        #     ax[0].clear()
        #     ax[0].set_title("Day " + str(i // 24) + " hour " + str(i % 24))
        #
        #     nc: dict = {k: coldict[v] for k, v in anim_list[i].nodes('disease_status')}
        #
        #     if i > 0:
        #         print('\r', round(i/len(anim_list)*100), '%:', round((time()-t0) - (time()-t0)/(i/len(anim_list))),
        #               'seconds estimated remaining', end='')
        #
        #     Graph(anim_list[i],
        #           # node_size=1,
        #           node_color=nc,
        #           node_layout=np,
        #           scale=(5,5),
        #           ax=ax[0])
        #
        #     ax[1].clear()
        #     ax[1].plot((history.iloc[:, :i] == 'S').sum(axis=0), color=coldict['S'])
        #     ax[1].plot((history.iloc[:, :i] == 'E').sum(axis=0), color=coldict['E'])
        #     ax[1].plot((history.iloc[:, :i] == 'I').sum(axis=0), color=coldict['I'])
        #     ax[1].plot((history.iloc[:, :i] == 'R').sum(axis=0), color=coldict['R'])
        #     ax[1].set_xlim([0, len(history.columns)])

        anim = animation.FuncAnimation(fig, animate, frames=len(anim_list), interval=200)

        writervideo = animation.FFMpegWriter(fps=60)
        anim.save("testanim.gif")
        plt.close()
