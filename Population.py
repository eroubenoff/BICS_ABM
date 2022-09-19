import pdb

import networkx as nx
import pandas as pd
from uuid import uuid4
from scipy.stats import bernoulli, poisson, randint
from random import shuffle
from copy import deepcopy
from itertools import chain

def split_list(l: list[str]) -> tuple[list[str], list[str]]:
    """
    Takes a list and splits it in half, returning a tuple of both halves. If the list is odd length, will silently
    drop the last element.

    Parameters
    ----------
    l: list

    Returns
    -------
    tuple

    """

    l_len = len(l)

    if l_len % 2 == 0:
        l = l[0:(l_len - 1)]
        l_len = len(l)

    return deepcopy(l[:l_len // 2]), deepcopy(l[l_len // 2:])

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

    # History is a list of dictionaries, with keys 'nodes' and 'edges'
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
                 id=None, disease_status='S', remaining_days_sick=0,
                 vaccine_priority=-1, vaccine_status=False):
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
        vaccine_priority

        Returns
        -------
        Node id of the added node

        """

        if id is None:
            id = uuid4().hex

        try:
            assert isinstance(id, str) | isinstance(id, int)
            assert isinstance(age, int) | isinstance(age, str)
            assert disease_status in ['S', 'E', 'I', 'R', 'D']
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
                                 'hhid': hhid,
                                 'vaccine_priority': vaccine_priority,
                                 'time_until_v2' : -1,
                                 'vaccine_status' : vaccine_status})])
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

        node_ids_I = self.node_ids_I
        # Check to see if one or more of the nodes in the edges are sick
        for e in range(len(el)):
            if el[e][0] in node_ids_I or el[e][1] in node_ids_I:
                el[e][2]['either_sick'] = True
            else:
                el[e][2]['either_sick'] = False

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

    def set_vax_priority(self, n, priority_level):
        self.G.nodes()[n]['vaccine_priority'] = priority_level
        return

    def V1(self, n, time_until_v2):

        self.G.nodes()[n]['vaccine_status'] = 'V1'
        self.G.nodes()[n]['time_until_v2'] = time_until_v2
        self.G.nodes()[n]['vaccine_priority'] = -1

        return


    def V2(self, n):
        self.G.nodes[n]['vaccine_status'] = 'V2'
        self.G.nodes[n]['time_until_v2'] = -1

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
    def node_ids_D(self):
        return [k for (k, v) in self.G.nodes.data() if v['disease_status'] == 'D']

    @property
    def node_ids_V1(self):
        return [k for (k, v) in self.G.nodes.data() if v['vaccine_status'] == 'V1']

    @property
    def node_ids_V2(self):
        return [k for (k, v) in self.G.nodes.data() if v['vaccine_status'] == 'V2']

    @property
    def hhids(self):
        return {h for _, h in self.G.nodes.data('hhid')}

    @property
    def vaccine_priority(self):
        return {k:v for k, v in self.G.nodes.data('vaccine_priority')}

    @property
    def time_until_v2(self):
        return {k:v for k, v in self.G.nodes.data('time_until_v2')}

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

        for v1, t in self.time_until_v2.items():
            if t == -1:
                continue
            if t == 0:
                continue
            else:
                self.G.nodes[v1]['time_until_v2'] -= 1

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
        n = [x for x in self.G.nodes.data()]
        e = [x for x in self.G.edges.data()]

        self.history.append({'nodes': deepcopy(n), 'edges': deepcopy(e)})

    def process_history(self):
        """
        Turns the history object into a pd.DataFrame
        Returns
        -------
        pd.DataFrame

        """

        if not bool(self.history):
            raise ValueError("Must have at least one saved history")


        disease_status = pd.DataFrame()
        vaccine_status = pd.DataFrame()
        edges = []

        for time, ne in enumerate(self.history):

            disease_status_temp = {n[0]: n[1]['disease_status'] for n in ne['nodes']}
            vaccine_status_temp = {n[0]: n[1]['vaccine_status'] for n in ne['nodes']}
            edges_temp = [e for e in ne['edges']]

            disease_status = pd.concat([disease_status, pd.Series(disease_status_temp, name=time)], axis=1)
            vaccine_status = pd.concat([vaccine_status, pd.Series(vaccine_status_temp, name=time)], axis=1)

            edges.append(edges_temp)

        return pd.DataFrame(disease_status), pd.DataFrame(vaccine_status), edges




    def transmit_daytime(self, beta: dict, p_mask: float, E_dist, I_dist, ve) -> None:
        """
        Transmits during each hour of the workday
        Assume a 10 hour day. Probability of a node having a contact each hour is num_cc_nonhh/10.
        Create subpopulation of nodes that have any contact in a given hour and join them to each other.

        Returns
        -------
        Modifies pop in place, returns nx.Graph for anim_list

        """

        self.remove_edges()

        node_ids_V1 = self.node_ids_V1
        node_ids_V2 = self.node_ids_V2

        # Create a subpopulation of people having contacts every hour
        # by bootstrapping the population. Number of times a node appears in the population
        # poisson sampling with lambda=num_cc_nonhh/10
        subpop = list(chain(*[
            [n[0]] * rep
            for n in self.G.nodes.data('num_cc_nonhh')
            for rep in range(poisson.rvs(n[1] / 10))
        ]))
        # Shuffle this list and split into two sublists
        shuffle(subpop)
        subpop = split_list(subpop)

        # Add edges
        # Probability that node is wearing a mask is p_mask.
        self.add_edges(
            [(n1, n2, {'protection': bernoulli.rvs(p_mask), 'household': False})
             for n1, n2 in zip(*subpop) if n1 != n2]
        )

        # Transmit
        t = list()
        for u, v, d in self.edges.data():
            if not d['either_sick']:
                continue
            else:
                # If either node is vaccinated:
                if u in node_ids_V1 or v in node_ids_V1:
                    if bernoulli(beta[d['protection']]) and bernoulli(1 - ve["V1"]):
                        t.append((u, v, E_dist.rvs(), I_dist.rvs()))

                elif u in node_ids_V2 or v in node_ids_V2:
                    if bernoulli(beta[d['protection']]) and bernoulli(1 - ve["V2"]):
                        t.append((u, v, E_dist.rvs(), I_dist.rvs()))
                elif bernoulli(beta[d['protection']]):
                    t.append((u, v, E_dist.rvs(), I_dist.rvs()))

        self.transmit(t)
        self.add_history()
        self.decrement()
        self.remove_edges()

        return

    def transmit_hh(self, beta: dict, E_dist, I_dist, ve) -> None:
        """
        Transmits within the household

        Parameters
        ----------
        pop: Population
        beta: dict
            Probability of transmission, conditional on mask usage
        E_dist, I_dist:
            distribution function of hours exposed and infectious, in hours

        Returns
        -------
        Modifies pop in place, returns nx.Graph for anim_list

        """

        # Loop through each hhid and transmit through all the tuples
        node_ids_I = self.node_ids_I
        t = list()
        for u, v, d in self.edges.data():
            if not d['either_sick']:
                continue
            if d['household'] and (u in node_ids_I or v in node_ids_I) and bernoulli(beta[d['protection']]):
                t.append((u, v,E_dist.rvs(), I_dist.rvs()))


        self.transmit(t)
        self.add_history()
        self.decrement()

        return
