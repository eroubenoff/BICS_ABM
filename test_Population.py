from Population import Population
import networkx as nx

pop = Population()
pop.add_node(id='1', age=26, disease_status='S', remaining_days_sick=0,
             gender='male', ethnicity='white', num_cc=5)
pop.add_node(id='2', age=30, disease_status='S', remaining_days_sick=0,
             gender='female',  ethnicity='white', num_cc=5)
pop.add_node(id='3', age=40, disease_status='S', remaining_days_sick=0,
             gender='male', ethnicity='white', num_cc=5)
pop.add_node(id='4', age=40, disease_status='S', remaining_days_sick=0,
             gender='female', ethnicity='white', num_cc=5)
pop.add_node(id='5', age=95, disease_status='S', remaining_days_sick=0,
             gender='female', ethnicity='white', num_cc=5)


pop.add_edges([('1', '2', {'protection': True, 'household': False}),
               ('3', '4', {'protection': False, 'household': True})
               ])


def test_set_sick():
    assert pop.node_ids_S == ['1', '2', '3', '4', '5']
    assert pop.node_ids_I == []
    assert pop.node_ids_R == []

    pop.set_sick('2', 10)

    assert pop.node_ids_S == ['1', '3', '4', '5']
    assert pop.node_ids_I == ['2']
    assert pop.node_ids_R == []

    return


def test_transmit():
    pop.set_sick('2', 10)
    print(pop.edges)
    pop.transmit
    assert pop.node_ids_S == ['3', '4', '5']
    assert pop.node_ids_I == ['1', '2']
    assert pop.node_ids_R == []

    return


def test_decrement():
    for d in range(15):
        pop.decrement()
        pop.transmit

    assert pop.node_ids_S == ['3', '4', '5']
    assert pop.node_ids_I == []
    assert pop.node_ids_R == ['1', '2']

    return