import networkx as nx
import pandas as pd
import numpy as np
from scipy.stats import bernoulli


class Population:
    G = nx.Graph()
    history = []

    def __init__(self, G=None):
        if G is None:
            self.G = nx.Graph()
        else:
            # Make sure that every node has a disease status, rds, ethnicity, num_cc, hhsize
            n_nodes = len(G.nodes())

            try:
                assert len(G.nodes("age")) == n_nodes
                assert len(G.nodes("disease_status")) == n_nodes
                assert len(G.nodes("remaining_days_sick")) == n_nodes
                assert len(G.nodes("ethnicity")) == n_nodes
                assert len(G.nodes("gender")) == n_nodes
                assert len(G.nodes("num_cc")) == n_nodes
                assert len(G.nodes("hhsize")) == n_nodes

            except:
                raise ValueError("All nodes must have valid fields")

            self.G = G

    def add_node(self, id, age, ethnicity, gender, num_cc, hhsize, disease_status='S', remaining_days_sick=0):
        """
        Adds a node to the graph. Makes sure that all node attributes are included.

        Parameters
        ----------
        gender
        id : str or int
        age: int
        disease_status : str
        remaining_days_sick: int
        ethnicity: str
        num_cc: int
        hhsize: int

        Returns
        -------
        None

        """

        try:
            assert isinstance(id, str) | isinstance(id, int)
            assert isinstance(age, int)
            assert disease_status in ['S', 'I', 'R']
            assert isinstance(remaining_days_sick, int)
            assert isinstance(ethnicity, str)
            assert isinstance(num_cc, int)
            assert isinstance(hhsize, int)

        except AssertionError as e:
            print(e)
            raise ValueError("Data passed:", id, age, disease_status, remaining_days_sick, ethnicity, num_cc, hhsize)

        self.G.add_nodes_from([(id,
                              {'age': age,
                               'disease_status': disease_status,
                               'remaining_days_sick': remaining_days_sick,
                               'gender': gender,
                               'ethnicity': ethnicity,
                               'num_cc': num_cc,
                               'hhsize': hhsize})])
        return

    def add_edge(self, u, v, protection=False):
        """
        Adds an edge to the graph. Both nodes u and v must be already present.

        Parameters
        ----------
        u : str or int
        v : str or int
        protection: bool
            Are the two nodes wearing protection? If True, reduces probability of transmission.

        Returns
        -------

        None

        """
        if u not in self.G.nodes() or v not in self.G.nodes():
            raise ValueError("nodes must be already present in graph")

        self.G.add_edges_from([
            (u, v, {'protection': protection})
        ])

    def remove_edge(self, u, v):
        """
        Removes an edge.

        Returns
        -------
        None

        """

        if u not in self.G.nodes() or v not in self.G.nodes():
            raise ValueError("nodes must be present in graph")

        self.G.remove_edge(u, v)
        return

    @property
    def nodes(self, nodes=None):
        return self.G.nodes(nodes)

    @property
    def edges(self, edges=None):
        return self.G.edges(edges)

    @property
    def node_ids(self):
        return [i[0] for i in self.nodes]

    @property
    def node_ids_S(self):
        return [k for (k, v) in self.G.nodes.data() if v['disease_status'] == 'S']

    @property
    def node_ids_I(self):
        return [k for (k, v) in self.G.nodes.data() if v['disease_status'] == 'I']

    @property
    def node_ids_R(self):
        return [k for (k, v) in self.G.nodes.data() if v['disease_status'] == 'R']

    def set_sick(self, node, n):
        """

        Parameters
        ----------
        node: int or str
            id of the node to set disease status
        n: int
            Number of days to be sick

        Returns
        -------
        None

        """
        assert isinstance(n, int)

        self.G.nodes()[node]['disease_status'] = 'I'
        self.G.nodes()[node]['remaining_days_sick'] = n
        return

    def decrement(self):
        for i in self.node_ids_I:
            if self.G.nodes[i]['remaining_days_sick'] == 0:
                self.G.nodes[i]['disease_status'] = 'R'
                continue
            else:
                self.G.nodes[i]['remaining_days_sick'] -= 1
        return

    def transmit(self):
        """

        Returns
        -------

        """

        transmission_list = []

        beta = {True: 0.1, False: 0.01}

        for e in self.G.edges.data():
            n1, n2, data = e
            if self.G.nodes('disease_status')[n1] == 'S' and self.G.nodes('disease_status')[n2] == 'I':
                if bernoulli.rvs(beta[data['protection']]):
                    self.set_sick(n1, n=10)
                    transmission_list.append(e[0])
                else:
                    continue
            elif self.G.nodes('disease_status')[n2] == 'S' and self.G.nodes('disease_status')[n1] == 'I':
                if bernoulli.rvs(beta[data['protection']]):
                    self.set_sick(n2, n=10)
                    transmission_list.append(e[1])
                else:
                    continue

        return transmission_list

    def add_history(self):
        """
        Saves the current state of the model to the history object

        Returns
        -------
        None.

        """
        d = {k:v for k, v in self.G.nodes.data('disease_status')}

        self.history.append(d)

    def process_history(self):
        """
        Turns the history object into a pd.DataFrame
        Returns
        -------
        pd.DataFrame

        """

        if not bool(self.history):
            raise ValueError("Must have at least one saved history")

        return pd.DataFrame(
            {i: pd.Series(val.values(), index=val.keys()) for i, val in enumerate(self.history)}
        )

