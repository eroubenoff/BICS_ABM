from Node import Node
from Edge import Edge
from Population import Population

nodelist = [
    Node(node_id=1, age=26, hhsize=2, num_cc_nonhh=6, ethnicity="white", disease_status="I"),
    Node(node_id=2, age=30, hhsize=1, num_cc_nonhh=2, ethnicity="white"),
    Node(node_id=3, age=18, hhsize=6, num_cc_nonhh=10, ethnicity="black"),
    Node(node_id=4, age=65, hhsize=6, num_cc_nonhh=1, ethnicity="black"),
    Node(node_id=5, age=95, hhsize=1, num_cc_nonhh=0, ethnicity="white")
]

edgelist = [
    Edge(nodelist[0], nodelist[1]),
    Edge(nodelist[2], nodelist[3])
]

test_pop = Population(nodelist, edgelist)
