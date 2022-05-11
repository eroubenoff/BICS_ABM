import networkx as nx
import pandas as pd
from uuid import uuid4
from scipy.stats import bernoulli


class Household:
    def __init__(self, hhsize, head, hhid=None):
        if hhid is None:
            hhid = uuid4().hex
        self.hhid = hhid
        self.hhsize = hhsize
        self.head = head
        self.memberlist = []

        assert self.hhsize == self.head['hhsize']

    def add_member(self, r):
        self.memberlist.append(r)


class Population:
    G = nx.Graph()
    history = []

    def __init__(self, G=None):
        if G is None:
            self.G = nx.Graph()
        else:
            # Make sure that every node has a disease status, rds, ethnicity, num_cc_nonhh, hhsize
            n_nodes = len(G.nodes())

            try:
                assert len(G.nodes("age")) == n_nodes
                assert len(G.nodes("disease_status")) == n_nodes
                assert len(G.nodes("remaining_days_sick")) == n_nodes
                assert len(G.nodes("ethnicity")) == n_nodes
                assert len(G.nodes("gender")) == n_nodes
                assert len(G.nodes("num_cc_nonhh")) == n_nodes

            except ValueError:
                raise ValueError("All nodes must have valid fields")

            self.G = G

    def add_node(self, age, ethnicity, gender, num_cc_nonhh, hhid=None,
                 id=None, disease_status='S', remaining_days_sick=0):
        """
        Adds a node to the graph. Makes sure that all node attributes are included.

        Parameters
        ----------
        gender
        id : str or int
            Default: None. If none, will generate an id from uuid4().hex
        hhid: str or int, defualt= None
        age: int
        disease_status : str
        remaining_days_sick: int
        ethnicity: str
        num_cc_nonhh: int

        Returns
        -------
        Node id of the added node

        """

        if id is None:
            id = uuid4().hex

        try:
            assert isinstance(id, str) | isinstance(id, int)
            assert isinstance(age, int) | isinstance(age, str)
            assert disease_status in ['S', 'E', 'I', 'R']
            assert isinstance(remaining_days_sick, int)
            assert isinstance(ethnicity, str)
            assert isinstance(num_cc_nonhh, int) | isinstance(num_cc_nonhh, float)

        except AssertionError as e:
            print(e)
            raise ValueError("Data passed:", id, age, disease_status, remaining_days_sick, ethnicity, num_cc_nonhh, hhid)

        self.G.add_nodes_from([(id,
                                {'age': age,
                                 'disease_status': disease_status,
                                 'remaining_days_sick': remaining_days_sick,
                                 'gender': gender,
                                 'ethnicity': ethnicity,
                                 'num_cc_nonhh': num_cc_nonhh,
                                 'hhid': hhid})])
        return id

    def add_edges(self, el, check_input=False):
        """
        Adds an edge to the graph in the format: [(u,v,d)]

        Parameters
        ----------
        u : str or int
        v : str or int
        protection: bool
            Are the two nodes wearing protection? If True, reduces probability of transmission.
        household: bool
            Are the two nodes in the same household?

        Returns
        -------
        Tuple of edge added. Returns None if u=v

        """
        if check_input:
            for e in el:
                try:
                    assert e[0] in self.G.nodes()
                    assert e[1] in self.G.nodes()
                except AssertionError:
                    raise ValueError("Both nodes must be present in graph")

                try:
                    assert all([x in ['household', 'protection'] for x in e[2].keys()])
                except AssertionError:
                    raise ValueError("Node must contain Household and Protection")

                if e[0] == e[1]:
                    el.pop(e)

        self.G.add_edges_from(el)

        return

    def add_household(self, hh: Household):
        """
        Add a household or a list of households to the graph
        Parameters
        ----------
        hh : Household or list<Household>

        Returns
        -------
        Dict of hhids

        """
        if isinstance(hh, Household):
            hh = [hh]

        # Make sure all of them are households
        assert all([isinstance(h, Household) for h in hh])

        # Add all the nodes within each household
        hhids = {}

        for h in hh:
            node_ids = list()

            node_ids.append(
                self.add_node(age=h.head['age'],
                              ethnicity=h.head['ethnicity'],
                              num_cc_nonhh=h.head['num_cc_nonhh'],
                              gender=h.head['gender'],
                              hhid=h.hhid)
            )

            for m in h.memberlist:
                node_ids.append(
                    self.add_node(age=m['age'],
                                  ethnicity=m['ethnicity'],
                                  num_cc_nonhh=m['num_cc_nonhh'],
                                  gender=m['gender'],
                                  hhid=h.hhid)
                )

            hhids[h.hhid] = node_ids

            self.add_edges([(u, v, {'protection': False, 'household': True})
                            for u in node_ids for v in node_ids if u != v])

        return hhids

    def connect_hh_edges(self):
        """
        Adds edges for all nodes in the same household

        Returns
        -------

        """

        hhidlist = self.G.nodes.data('hhid')

        for h in self.hhids:
            node_ids = [n for n, hh in hhidlist if h == hh]
            self.add_edges([(u, v, {'protection': False, 'household': True})
                            for u in node_ids for v in node_ids if u != v])


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

    def remove_edges(self, keep_hh=True):
        """
        Drops all edges in the population

        Returns
        -------
        None

        """
        if keep_hh:
            self.G.remove_edges_from(
                [(u, v) for u, v, d in self.G.edges.data('household') if not d]
            )
        else:
            self.G.remove_edges_from(self.G.edges)


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
    def node_ids_E(self):
        return [k for (k, v) in self.G.nodes.data() if v['disease_status'] == 'E']

    @property
    def node_ids_I(self):
        return [k for (k, v) in self.G.nodes.data() if v['disease_status'] == 'I']

    @property
    def node_ids_R(self):
        return [k for (k, v) in self.G.nodes.data() if v['disease_status'] == 'R']

    @property
    def hhids(self):
        return {h for _, h in self.G.nodes.data('hhid')}

    def set_sick(self, node, E_n=3*24, I_n=5*24):
        """

        Parameters
        ----------
        node: int or str
            id of the node to set disease status
        E_n, I_n: int
            Number of days to be Exposed or sick

        Returns
        -------
        None

        """
        # assert isinstance(E_n, int)

        self.G.nodes()[node]['disease_status'] = 'E'
        self.G.nodes()[node]['remaining_days_exposed'] = E_n
        self.G.nodes()[node]['remaining_days_sick'] = I_n
        return

    def decrement(self):

        for i in self.node_ids_I:
            if self.G.nodes[i]['remaining_days_sick'] == 0:
                self.G.nodes[i]['disease_status'] = 'R'
                print("Node", i, "recovers")
                continue
            else:
                self.G.nodes[i]['remaining_days_sick'] -= 1

        for e in self.node_ids_E:
            if self.G.nodes[e]['remaining_days_exposed'] == 0:
                self.G.nodes[e]['disease_status'] = 'I'
                print("Node", e, "became symptomatic")
                continue
            else:
                self.G.nodes[e]['remaining_days_exposed'] -= 1

        return

    def transmit(self, el):
        """

        Parameters
        ----------
        el: list of tuples of format:
                (n1, n2, E_n, I_n)
            List of nodes to transmit between

        Returns
        -------
        List of newly-sick nodes


        """

        transmission_list = []

        for n1, n2, E_n, I_n in el:
            if self.G.nodes('disease_status')[n1] == 'S' and self.G.nodes('disease_status')[n2] == 'I':
                print("Node", n1, "has been infected")
                self.set_sick(n1, E_n=E_n, I_n=I_n)
                transmission_list.append(n1)
            elif self.G.nodes('disease_status')[n2] == 'S' and self.G.nodes('disease_status')[n1] == 'I':
                print("Node", n2, "has been infected")
                self.set_sick(n2, E_n=E_n, I_n=I_n)
                transmission_list.append(n2)

        return transmission_list

    def add_history(self):
        """
        Saves the current state of the model to the history object

        Returns
        -------
        None.

        """
        d = {k: v for k, v in self.G.nodes.data('disease_status')}

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
