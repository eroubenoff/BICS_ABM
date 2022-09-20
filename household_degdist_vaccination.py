import random
import networkx as nx
import numpy as np
from scipy.stats import  randint
from matplotlib import pyplot as plt, animation
from load_data import sim_pop, lucid_data
from time import time



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
        pop.distribute_vax(n_vax_daily)

        # Workday
        for hour in range(8, 18):
            print("Day", day, "Hour", hour)
            pop.transmit_daytime(beta, p_mask, E_dist, I_dist, ve)

        pop.connect_hh_edges()

        # Evening
        for hour in range(18, 24):
            print("Day", day, "Hour", hour)
            pop.transmit_hh(beta, E_dist, I_dist, ve)

        pop.add_history()
        day += 1
        n_days -= 1


    return pop


if __name__ == "__main__":

    pop = household_mixing_w_degree_dist(10000, initial_sick=5, n_vax_daily=500)

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

    run_animation = False

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
            ax[1].plot((disease_status.iloc[:,:i] == 'S').sum(axis=0), color=coldict['S'], label='S')
            ax[1].plot((disease_status.iloc[:,:i] == 'E').sum(axis=0), color=coldict['E'], label='E')
            ax[1].plot((disease_status.iloc[:,:i] == 'I').sum(axis=0), color=coldict['I'], label='I')
            ax[1].plot((disease_status.iloc[:,:i] == 'R').sum(axis=0), color=coldict['R'], label='R')
            ax[1].plot((vaccine_status.iloc[:,:i] == 'V1').sum(axis=0), color=coldict['V1'], label='V1')
            ax[1].plot((vaccine_status.iloc[:,:i] == 'V2').sum(axis=0), color=coldict['V2'], label='V2')
            ax[1].set_xlim([0, len(disease_status.columns)])
            ax[1].legend()


        anim = animation.FuncAnimation(fig, animate, frames=len(edges_history), interval=50)

        writervideo = animation.FFMpegWriter(fps=60)
        anim.save("testanim.gif")
        plt.close()
