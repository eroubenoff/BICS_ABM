# import networkx as nx
import pandas as pd
from uuid import uuid4
from scipy.stats import bernoulli

class Node():
    def __init__(self, id, age, ethnicity, gender, num_cc_nonhh):
        self.edges = []
        self.id = id
        self.age = age
        self.ethnicity = ethnicity
        self.gender = gender
        self.num_cc_nonhh = num_cc_nonhh
        self.disease_status = "S"
        self.remaining_days_sick = 0

    def edges(self):
        return self.edges

    def add_edge(self, id):
        self.edges.append(id)

    def remove_edge(self, id):
        self.edges.remove(id)



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



    # G = nx.Graph()
    history = []

    def __init__(self):
        self.__nodes__ = dict()
        self.__edges__ = dict()

    def nodes(self):
        return self.__nodes__
    def edges(self):
        return self.__edges__


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
            assert disease_status in ['S', 'E', 'I', 'R', 'D', 'V1', 'V2']
            assert isinstance(remaining_days_sick, int)
            assert isinstance(ethnicity, str)
            assert isinstance(num_cc_nonhh, int) | isinstance(num_cc_nonhh, float)

        except AssertionError as e:
            print(e)
            raise ValueError("Data passed:", id, age, disease_status, remaining_days_sick, ethnicity, num_cc_nonhh, hhid)

        self.__nodes__[id] = Node(id, age, ethnicity, gender, num_cc_nonhh)
        # self.G.add_nodes_from([(id,
        #                         {'age': age,
        #                          'disease_status': disease_status,
        #                          'remaining_days_sick': remaining_days_sick,
        #                          'gender': gender,
        #                          'ethnicity': ethnicity,
        #                          'num_cc_nonhh': num_cc_nonhh,
        #                          'hhid': hhid})])
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
                    assert e[0] in self.nodes().keys()
                    assert e[1] in self.nodes().keys()
                except AssertionError:
                    raise ValueError("Both nodes must be present in graph")

                try:
                    assert all([x in ['household', 'protection'] for x in e[2].keys()])
                except AssertionError:
                    raise ValueError("Node must contain Household and Protection")

                if e[0] == e[1]:
                    el.pop(e)

        self.__edges__[(el[0], el[1])] = el[2]
        self.__edges__[(el[1], el[2])] = el[2]
        self.__nodes__[el[0]].add_edge(el[1])
        self.__nodes__[el[1]].add_edge(el[0])

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

        if u not in self.nodes() or v not in self.nodes():
            raise ValueError("nodes must be present in graph")

        del self.__edges__[(u, v)]
        del self.__edges__[(v, u)]
        self.__nodes__[u].remove_edge(v)
        self.__nodes__[v].remove_edge(u)

        return

    def remove_edges(self, keep_hh=True):
        """
        Drops all edges in the population

        Returns
        -------
        None

        """
        for ((u, v), d) in self.__edges__:
            if keep_hh and d['household']:
                continue
            self.remove_edge(u, v)


        return

    @property
    def nodes(self, nodes=None):
        if nodes is None:
            return self.__nodes__
        else:
            return [self.__nodes__[n] for n in nodes]

    @property
    def edges(self, edges=None):
        if edges is None:
            return self.__edges__
        else:
            return [self.__edges__[e] for e in edges]

    @property
    def node_ids(self):
        return self.__nodes__.keys()

    @property
    def node_ids_S(self):
        return [k for (k, v) in self.__nodes__ if v['disease_status'] == 'S']

    @property
    def node_ids_E(self):
        return [k for (k, v) in self.__nodes__ if v['disease_status'] == 'E']

    @property
    def node_ids_I(self):
        return [k for (k, v) in self.__nodes__ if v['disease_status'] == 'I']

    @property
    def node_ids_R(self):
        return [k for (k, v) in self.__nodes__ if v['disease_status'] == 'R']

    @property
    def node_ids_D(self):
        return [k for (k, v) in self.__nodes__ if v['disease_status'] == 'D']

    @property
    def node_ids_V1(self):
        return [k for (k, v) in self.__nodes__ if v['disease_status'] == 'V1']

    @property
    def node_ids_V2(self):
        return [k for (k, v) in self.__nodes__ if v['disease_status'] == 'V2']

    @property
    def hhids(self):
        return {h['hhid'] for _, h in self.__nodes__}

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

        self.__nodes__[node]['disease_status'] = 'E'
        self.__nodes__[node]['remaining_days_exposed'] = E_n
        self.__nodes__[node]['remaining_days_sick'] = I_n
        return

    def decrement(self):

        for i in self.node_ids_I:
            if self.__nodes__[i]['remaining_days_sick'] == 0:
                self.__nodes__[i]['disease_status'] = 'R'
                print("Node", i, "recovers")
                continue
            else:
                self.G.nodes[i]['remaining_days_sick'] -= 1

        for e in self.node_ids_E:
            if self.__nodes__[e]['remaining_days_exposed'] == 0:
                self.__nodes__[e]['disease_status'] = 'I'
                print("Node", e, "became symptomatic")
                continue
            else:
                self.__nodes__[e]['remaining_days_exposed'] -= 1

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
            if self.__nodes__[n1]['disease_status'] == 'S' and self.__nodes__[n2]['disease_status'] == 'I':
                print("Node", n1, "has been infected")
                self.set_sick(n1, E_n=E_n, I_n=I_n)
                transmission_list.append(n1)
            elif self.__nodes__[n2]['disease_status'] == 'S' and self.__nodes__[n1]['disease_status'] == 'I':
                print("Node", n2, "has been infected")
                self.set_sick(n2, E_n=E_n, I_n=I_n)
                transmission_list.append(n2)

        return transmission_list

    def vaccinate(self, node):
        """
        If node is in state S or R, sets to V1; if in V1, set to V2
        Parameters
        ----------
        node

        Returns
        -------
        None

        """
        self.__nodes__[node]

    def add_history(self):
        """
        Saves the current state of the model to the history object

        Returns
        -------
        None.

        """
        d = {k: v['disease_status'] for k, v in self.__nodes__}

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
