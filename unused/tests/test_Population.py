from unused.test_pop import test_pop

# {
#     1: Node(node_id=1, age=26, hhsize=2, num_cc_nonhh=6, ethnicity="white"),
#     2: Node(node_id=2, age=30, hhsize=1, num_cc_nonhh=2, ethnicity="white"),
#     3: Node(node_id=3, age=18, hhsize=6, num_cc_nonhh=10, ethnicity="black"),
#     4: Node(node_id=4, age=65, hhsize=6, num_cc_nonhh=1, ethnicity="black"),
#     5: Node(node_id=5, age=95, hhsize=1, num_cc_nonhh=0, ethnicity="white")
# }
def test_getters():
    assert list(test_pop.nodelist.keys()) == ['1', '2', '3', '4', '5']
    assert list(test_pop.edgelist.keys()) == ['1-2', '3-4']
    assert test_pop.node_ids == ['1', '2', '3', '4', '5']
    assert test_pop.edge_ids == ['1-2', '3-4']
    assert test_pop.node_ids_S == ['2', '3', '4', '5']
    assert test_pop.node_ids_I == ['1']
    assert test_pop.node_ids_R == []

def test__transmit():
    assert test_pop.nodelist['1'].disease_status == 'I'
    assert test_pop.nodelist['2'].disease_status == 'S'
    test_pop._transmit('1-2')
    assert test_pop.node_ids_I == ['1', '2']

def test_transmit():
    test_pop.nodelist['2'].disease_status = 'S'
    assert test_pop.disease_status == (['2', '3', '4', '5'], ['1'], [])
    test_pop.nodelist['3'].disease_status = 'I'
    assert test_pop.transmit() == ['2', '4']

def test_decrement():
    test_pop.decrement()
    print([x.remaining_days_sick for x in test_pop.nodelist if x.node_id in test_pop.node_ids_I])
    # assert [x.remaining_days_sick for x in test_pop.nodelist[test_pop.node_ids_I]] == [10, 10]

