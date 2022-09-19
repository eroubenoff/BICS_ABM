import random

import networkx as nx
from scipy.stats import bernoulli
from matplotlib import pyplot as plt, animation
from load_data import sim_pop, lucid_data
from itertools import combinations
from copy import deepcopy


def household_mixing_w_random(n_hh=1000, initial_sick=1, n_days=100, n_edges=10):

    pop = sim_pop(n_hh, lucid_data['wave4'])

    for _ in range(initial_sick):
        pop.set_sick(random.choice(pop.node_ids), 5)

    node_combinations = list(combinations(pop.node_ids, 2))

    beta = {True: 0.01, False: 0.1}

    anim_list = [pop.G]

    for day in range(n_days):

        # Morning
        pop.connect_hh_edges()
        for hour in range(0, 8):
            print("Day", day, "Hour", hour)
            pop.transmit
            anim_list.append(deepcopy(pop.G))
        pop.remove_edges(keep_hh=False)

        # Workday
        for hour in range(8, 18):
            print("Day", day, "Hour", hour)
            pop.add_edges(
                [(n1, n2, {'protection': bernoulli.rvs(0.5), 'household': False})
                 for n1, n2 in random.sample(node_combinations, n_edges)]
            )
            pop.transmit
            anim_list.append(deepcopy(pop.G))
            pop.remove_edges(keep_hh=False)

        # Evening
        pop.connect_hh_edges()
        for hour in range(18, 24):
            print("Day", day, "Hour", hour)
            pop.transmit
            anim_list.append(deepcopy(pop.G))
        pop.remove_edges(keep_hh=False)


        pop.decrement()
        pop.add_history()

    return pop, pop.process_history(), anim_list


if __name__ == "__main__":
    pop, history, anim_list = household_mixing_w_random(100, initial_sick=1, n_days=60, n_edges=2)

    plt.plot((history == 'S').sum(axis=0))
    plt.plot((history == 'E').sum(axis=0))
    plt.plot((history == 'I').sum(axis=0))
    plt.plot((history == 'R').sum(axis=0))

    plt.show()

    for i in anim_list:
        print(i.nodes.data('disease_status'))

    coldict = {'S': 'blue', 'E': 'purple', 'I': 'red', 'R': 'green'}

    fig = plt.figure()
    pos = nx.spring_layout(anim_list[1])

    def animate(i):
        fig.clear()
        nc = [coldict[v] for k, v in anim_list[i].nodes('disease_status')]
        nx.draw(anim_list[i], pos=pos, node_color= nc, node_size=10)
        fig.suptitle(str(i))
        return fig


    anim = animation.FuncAnimation(fig, animate, frames=len(anim_list), interval=100)

    # plt.show()
    writervideo = animation.FFMpegWriter(fps=60)
    anim.save("testanim.gif")
    plt.close()
