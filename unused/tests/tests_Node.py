from unused.Node import Node

n = Node(node_id=1, age=26, hhsize=2, num_cc_nonhh=6, ethnicity="white")

def test_attributes():
    assert n.node_id == 1
    assert n.age == 26
    assert n.hhsize == 2
    assert n.num_cc_nonhh == 6
    assert n.ethnicity == "white"
    assert n.disease_status == "S"
    assert n.remaining_days_sick == 0

def test_disease_status_set():
    n.disease_status = "I"
    assert n.disease_status == "I"
    n.disease_status = "S"
    assert n.disease_status == "S"

def test_remaining_days_sick_set():
    n.remaining_days_sick = 14
    assert n.remaining_days_sick == 14
    n.remaining_days_sick = 0
    assert n.remaining_days_sick == 0

def test_set_sick():
    n.set_sick(14)
    assert n.disease_status == "I"
    assert n.remaining_days_sick == 14

def test_add_connection():
    n.add_connection(2)
    n.add_connection([3, 4])
    assert n.connected_nodes == {2, 3, 4}

