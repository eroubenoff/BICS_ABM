import random

import networkx as nx
from scipy.stats import bernoulli
from matplotlib import pyplot as plt, animation
from load_data import sim_individuals, lucid_data
from itertools import combinations
import cProfile


def random_mixing(n_nodes=1000, initial_sick=1, n_days=100, n_edges=10):

    pop = sim_individuals(n_nodes, lucid_data['wave4'])
    # nx.draw(pop.G, with_labels=False)

    for _ in range(initial_sick):
        pop.set_sick(random.choice(pop.node_ids), 5)

    node_combinations = list(combinations(pop.node_ids, 2))

    beta = {True: 0.01, False: 0.1}

    for day in range(n_days):
        for hour in range(24):
            print("Day", day, "Hour", hour)
            if hour < 8 or hour > 20:
                continue

            pop.add_edges(
                [(n1, n2, {'protection': bernoulli.rvs(0.5), 'household': False})
                 for n1, n2 in random.sample(node_combinations, n_edges)]
            )

            # Choose edges to transmit between
            # Beta is probability of transmission if wearing a mask
            pop.transmit(
                [(u, v) for u, v, p in pop.edges.data('protection') if bernoulli(beta[p])]
            )
            # nx.draw(pop.G, with_labels=False)
            # plt.pause(0.1)
            pop.remove_edges(keep_hh=False)

        pop.decrement()
        pop.add_history()

    return pop.process_history()


if __name__ == "__main__":
    # cProfile.run('random_mixing(100)')

    history = random_mixing(1000, initial_sick=5)

    print(history)

    plt.plot((history == 'S').sum(axis=0))
    plt.plot((history == 'I').sum(axis=0))
    plt.plot((history == 'R').sum(axis=0))

    plt.show()


