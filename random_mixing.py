import random
from Population import Population
from scipy.stats import bernoulli
from scipy.stats import randint
import numpy as np
from matplotlib import pyplot as plt
from load_data import sim_pop

# pop = Population()

# for i in range(1000):
#     pop.add_node(id=str(i),
#                  age=randint.rvs(18, 85),
#                  ethnicity=['black', 'white'][bernoulli.rvs(0.5)],
#                  gender=['male', 'female'][bernoulli.rvs(0.5)],
#                  num_cc=randint.rvs(1, 10),
#                  hhsize=randint.rvs(1, 5))

pop = sim_pop(1000)

for _ in range(3):
    pop.set_sick(random.choice(pop.node_ids), 10)

n_days = 100

history = [None] * n_days

for day in range(n_days):
    for hour in range(24):
        if hour < 8 or hour > 20:
            continue

        for n1, n2 in zip(random.sample(pop.node_ids, 50), random.sample(pop.node_ids, 50)):
            if n1 == n2:
                continue
            if bernoulli.rvs(0.1):
                pop.add_edge(n1, n2, protection=bernoulli.rvs(0.5))

        pop.transmit()

    pop.decrement()

    pop.add_history()


history = pop.process_history()
print(history)

plt.plot((history == 'S').sum(axis=0))
plt.plot((history == 'I').sum(axis=0))
plt.plot((history == 'R').sum(axis=0))

plt.show()

# ret = np.ndarray(shape=(100, 3), dtype=int)
# ret[day, 0] = len(pop.node_ids_S)
# ret[day, 1] = len(pop.node_ids_I)
# ret[day, 2] = len(pop.node_ids_R)
# plt.plot(range(n_days), ret[:, 0], label='S')
# plt.plot(range(n_days), ret[:, 1], label='I')
# plt.plot(range(n_days), ret[:, 2], label='R')
#
# plt.show()
