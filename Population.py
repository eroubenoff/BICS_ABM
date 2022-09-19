import itertools
import pdb

import networkx as nx
import pandas as pd
from uuid import uuid4
from scipy.stats import bernoulli, poisson, randint
from random import shuffle
from itertools import chain
import pickle


def invert_dict(d):
    ret = dict()
    for key, value in d.items():
        if value in ret:
            ret[value].append(key)
        else:
            ret[value] = [key]

    return ret

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

    return l[:l_len // 2], l[l_len // 2:]

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

    # households is iter of hhs
    households = {}

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

        # Add to hhs list
        if hhid in self.households:
            self.households[hhid].append(id)
        else:
            self.households[hhid] = [id]

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


        # hhidlist = self.G.nodes.data('hhid')
        #
        # for h in self.hhids:
        #     node_ids = [n for n, hh in hhidlist if h == hh]
        #     self.add_edges([(u, v, {'protection': False, 'household': True})
        #                     for u in node_ids for v in node_ids if u != v])

        for _, idlist in self.households.items():
            self.add_edges([
                (u, v, {'protection': False, 'household': True}) for u, v in itertools.combinations(idlist, 2)
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



    def add_history(self):
        """
        Saves the current state of the model to the history object

        Returns
        -------
        None.

        """
        # n = [x for x in self.G.nodes.data()]
        # e = [x for x in self.G.edges.data()]
        #
        # self.history.append({'nodes': deepcopy(n), 'edges': deepcopy(e)})

        self.history.append({
            'nodes': pickle.loads(pickle.dumps(self.G.nodes.data())),
            'edges': pickle.loads(pickle.dumps(self.G.edges.data()))
        })


    def process_history(self):
        """
        Turns the history object into a pd.DataFrame
        Returns
        -------
        pd.DataFrame

        """

        if not bool(self.history):
            raise ValueError("Must have at least one saved history")


        disease_status = []
        vaccine_status = []
        edges = []

        for time, ne in enumerate(self.history):

            disease_status.append(pd.Series({n[0]: n[1]['disease_status'] for n in ne['nodes']}, name=time))
            vaccine_status.append(pd.Series({n[0]: n[1]['vaccine_status'] for n in ne['nodes']}, name=time))
            edges.append([e for e in ne['edges']])

        return pd.concat(disease_status, axis=1), pd.concat(vaccine_status, axis=1), edges


    def transmit(self, beta, ve, E_dist, I_dist):
        """

        Parameters
        ----------
        Transmits between any eligible nodes

        Returns
        -------
        list of newly infected nodes


        """

        transmission_list = []


        for n1, n2, data in self.G.edges.data():
            # See if either node is infected
            n1_ds = self.G.nodes('disease_status')[n1]
            n2_ds = self.G.nodes('disease_status')[n2]

            # Make sure only one is I
            if (n1_ds == 'I' and n2_ds == 'S') or (n1_ds == 'S' and n2_ds == 'I'):
                # Do stuff
                # For consistency, check data['either sick']
                # if not data['either_sick']:
                #     raise ValueError("Either node is sick, but edge attribute either_sick is false")

                # Identify which node is sick
                node_to_infect = n1 if n1_ds == 'S' else n2



                # Protection
                mask_usage = data['protection']
                vaccine_status = self.G.nodes('vaccine_status')[node_to_infect]

                # Flip coins
                if bernoulli(beta[mask_usage]) and bernoulli(1-ve[vaccine_status]):
                    self.set_sick(node_to_infect, E_dist.rvs(), I_dist.rvs())
                    print("Node", node_to_infect, "has been infected")
                    transmission_list.append(node_to_infect)


            else:
                continue


        return transmission_list

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

        # Create a subpopulation of people having contacts every hour
        # by bootstrapping the population. Number of times a node appears in the population
        # poisson sampling with lambda=num_cc_nonhh/10
        # subpop = list(chain(*[
        #     [n[0]] * rep
        #     for n in self.G.nodes.data('num_cc_nonhh')
        #     for rep in range(poisson.rvs(n[1] / 10))
        # ]))

        # Generate the number of stubs for each node
        num_cc_nonhh = nx.get_node_attributes(self.G, 'num_cc_nonhh')
        subpop = list(chain(*[
            [k]*v for k, v in zip(num_cc_nonhh.keys(), poisson.rvs([*num_cc_nonhh.values()]) // 10)
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

        self.transmit(beta, ve, E_dist, I_dist)
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


        self.transmit(beta, ve, E_dist, I_dist)
        self.add_history()
        self.decrement()

        return

    def distribute_vax(self, n_daily: int, time_until_v2: int = 25 * 24):
        """
        Distributes n_daily doses of the vaccine to the population, by priority
        group
        Parameters
        ----------
        pop: Population
            The object to modify
        n_daily: int
            Number of vaccines distributed every day

        Returns
        -------

        """


        n_remaining = n_daily
        n_remaining_2 = n_daily



        # Invert the dict of priorities
        v1_prior = self.vaccine_priority
        v1_prior = invert_dict(v1_prior)
        del v1_prior[-1]



        # V1
        while n_remaining > 0:
            # The node that we're vaccinating
            try :
                max_prior = max(v1_prior.keys())
                if (len(v1_prior[max_prior]) == 0):
                    del v1_prior[max_prior]
                max_prior = max(v1_prior.keys())
            except ValueError:
                break

            shuffle(v1_prior[max_prior])
            nid = v1_prior[max_prior].pop()

            print("Node", nid, "got the first dose of vaccine")

            # Set the node's status to V1
            self.V1(nid, time_until_v2)


            n_remaining -= 1


        # V2
        v2_prior = self.time_until_v2
        v2_prior = invert_dict(v2_prior)
        try:
            v2_prior = v2_prior[0]
            print(v2_prior)
        except KeyError:
            return

        while n_remaining_2 > 0 and len(v2_prior) > 0:
            # The node that we're vaccinating

            shuffle(v2_prior)
            nid = v2_prior.pop()

            # print(nid)

            print("Node", nid, "got the second dose of vaccine")

            # Set the node's status to V1
            self.V2(nid)

            n_remaining_2 -= 1

        return
