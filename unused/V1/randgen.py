from scipy.stats import bernoulli, poisson
from timeit import default_timer
from itertools import cycle

n = 10000
# Test one: generate random one by one
start = default_timer()
for i in range(n):
    bernoulli.rvs(0.5)
end = default_timer()
print(end - start)

# Test two: generate all at once
start = default_timer()
bernoulli.rvs(0.5, size=n)
end = default_timer()
print(end - start)

# Test three: generate all at once and then yield each
start = default_timer()
l = bernoulli.rvs(0.5, size=n)
print(len(l))
l = iter(l)
for i in l:
    next(l)
end = default_timer()
print(end - start)

l = poisson.rvs(5, size=n)
l = cycle(l)
for i in range(10):
    print(next(l))