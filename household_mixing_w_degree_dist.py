import pdb
import random

import networkx as nx
from scipy.stats import bernoulli
from matplotlib import pyplot as plt, animation
from load_data import sim_pop, lucid_data
from random import shuffle
from copy import deepcopy


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


def household_mixing_w_degree_dist(n_hh=1000, initial_sick=1, n_days=100):
    pop = sim_pop(n_hh, lucid_data['wave4'])

    for _ in range(initial_sick):
        pop.set_sick(random.choice(pop.node_ids), 5)

    beta = {True: 0.01, False: 0.1}

    anim_list = [pop.G]

    for day in range(n_days):

        # Morning
        pop.connect_hh_edges()
        for hour in range(0, 8):
            print("Day", day, "Hour", hour)
            pop.transmit(
                [(u, v) for u, v, p in pop.edges.data('protection') if bernoulli(beta[p])]
            )
            anim_list.append(deepcopy(pop.G))
        pop.remove_edges(keep_hh=False)

        # Workday
        for hour in range(8, 18):
            # Assume a 10 hour day. Probability of a node having a contact each hour is num_cc_nonhh/10.
            # Create subpopulation of nodes that have any contact in a given hour and join them to each other.
            print("Day", day, "Hour", hour)

            # Subpop:
            subpop = [n[0] for n in pop.G.nodes.data('num_cc_nonhh') if bernoulli.rvs(min(n[1]/10, 1))]
            shuffle(subpop)
            subpop = split_list(subpop)

            pop.add_edges(
                [(n1, n2, {'protection': bernoulli.rvs(0.5), 'household': False})
                 for n1, n2 in zip(*subpop)]
            )

            pop.transmit(
                [(u, v) for u, v, p in pop.edges.data('protection') if bernoulli(beta[p])]
            )
            anim_list.append(deepcopy(pop.G))
            pop.remove_edges(keep_hh=False)

        # Evening
        pop.connect_hh_edges()
        for hour in range(18, 24):
            print("Day", day, "Hour", hour)
            pop.transmit(
                [(u, v) for u, v, p in pop.edges.data('protection') if bernoulli(beta[p])]
            )
            anim_list.append(deepcopy(pop.G))
        pop.remove_edges(keep_hh=False)

        pop.decrement()
        pop.add_history()

        if len(pop.node_ids_I) == 0:
            return pop, pop.process_history(), anim_list

    return pop, pop.process_history(), anim_list


if __name__ == "__main__":
    pop, history, anim_list = household_mixing_w_degree_dist(100, initial_sick=3, n_days=60)

    print(history)

    plt.plot((history == 'S').sum(axis=0))
    plt.plot((history == 'I').sum(axis=0))
    plt.plot((history == 'R').sum(axis=0))

    plt.show()

    for i in anim_list:
        print(i.nodes.data('disease_status'))

    coldict = {'S': 'blue', 'I': 'red', 'R': 'green'}

    fig = plt.figure()
    pos = nx.spring_layout(anim_list[1])


    run_animation = True
    if run_animation:
        def animate(i):
            fig.clear()
            coldict = {'S': 'blue', 'I': 'red', 'R': 'green'}
            nc = [coldict[v] for k, v in anim_list[i].nodes('disease_status')]
            nx.draw(anim_list[i], pos=pos, node_color=nc, node_size=10)
            fig.suptitle(str(i))
            return fig


        anim = animation.FuncAnimation(fig, animate, frames=len(anim_list), interval=100)

        # plt.show()
        writervideo = animation.FFMpegWriter(fps=60)
        anim.save("testanim.gif")
        plt.close()
