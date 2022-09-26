import itertools
import pandas as pd
from uuid import uuid4
from random import shuffle
import igraph as ig
from array import array

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


    def __init__(self, G=None):
        # History is a list of dictionaries, with keys 'nodes' and 'edges'
        self.history = []
        self.households = {}

        # Create arrays for node statuses
        self.node_ids_S = array('I')
        self.node_ids_E = array('I')
        self.node_ids_I = array('I')
        self.node_ids_R = array('I')
        self.node_ids_D = array('I')
        self.node_ids_V1 = array('I')
        self.node_ids_V2 = array('I')

        self.mu = {
            '[0,18)': 0,
            '[18,25)': 0,
            '[25,35)': 0,
            '[35,45)': 0,
            '[45,55)': 0,
            '[55,65)': 0,
            '[65,75)': 0,
            '[75,85)': 0,
            '>85' : 0
        }

        if G is None:
            self.G = ig.Graph(directed=False)
        else:
            self.G = G

        self.hhedges = []

    def add_node(self, age, ethnicity, gender, num_cc_nonhh, hhid=None,
                 disease_status='S', remaining_days_sick=0,
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

        # if id is None:
        #     id = uuid4().hex

        try:
            # assert isinstance(id, str) | isinstance(id, int)
            assert isinstance(age, int) | isinstance(age, str)
            assert disease_status in ['S', 'E', 'I', 'R', 'D']
            assert isinstance(remaining_days_sick, int)
            assert isinstance(ethnicity, str)
            assert isinstance(num_cc_nonhh, int) | isinstance(num_cc_nonhh, float)

        except AssertionError as e:
            print(e)
            raise ValueError("Data passed:", id, age, disease_status, remaining_days_sick, ethnicity, num_cc_nonhh,
                             hhid)

        # Creates node in place and returns the index of the newly created node
        id = self.G.add_vertex(age=age,
                               disease_status=disease_status,
                               remaining_days_sick=remaining_days_sick,
                               gender=gender,
                               ethnicity=ethnicity,
                               num_cc_nonhh=num_cc_nonhh,
                               hhid=hhid,
                               vaccine_priority=vaccine_priority,
                               time_until_v2=-1,
                               vaccine_status=vaccine_status).index

        self.G.vs[id]['name'] = id
        # self.node_ids_S.append(id)

        # Add to hhs list
        if hhid in self.households:
            self.households[hhid].append(id)
        else:
            self.households[hhid] = [id]

        return id

    def add_edges(self, es, attributes):
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
        self.G.add_edges(es=es, attributes=attributes)

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

            # Add to hhedges list
            [self.hhedges.append((u, v)) for u, v in itertools.combinations(node_ids, 2)]
            self.node_ids_S.extend(node_ids)

        return hhids

    def connect_hh_edges(self):
        """
        Adds edges for all nodes in the same household

        Returns
        -------

        """
        self.remove_edges()
        self.add_edges(self.hhedges, attributes={'protection': False, 'household': True})

    def remove_edge(self, u, v):
        """
        Removes an edge.

        Returns
        -------
        None

        """

        if u not in self.G.nodes() or v not in self.G.nodes():
            raise ValueError("nodes must be present in graph")

        # self.G.remove_edge(u, v)
        self.G.delete_edges([(u, v)])
        return

    def remove_edges(self, keep_hh=False):
        """
        Drops all edges in the population

        Returns
        -------
        None

        """
        # if keep_hh:

        # self.G.remove_edges_from(
        #     [(u, v) for u, v, d in self.G.edges.data('household') if not d]
        # )
        # else:
        # self.G.remove_edges_from(self.G.edges)
        self.G.delete_edges()

        return

    def set_vax_priority(self, n, priority_level):
        # self.G.nodes()[n]['vaccine_priority'] = priority_level
        self.G.vs[n]['vaccine_priority'] = priority_level
        return

    def V1(self, n, time_until_v2):

        self.G.vs[n]['vaccine_status'] = 'V1'
        self.G.vs[n]['time_until_v2'] = time_until_v2
        self.G.vs[n]['vaccine_priority'] = -1
        self.node_ids_V1.append(n)

        return

    def V2(self, n):
        self.G.vs[n]['vaccine_status'] = 'V2'
        self.G.vs[n]['time_until_v2'] = -1
        self.node_ids_V1.remove(n)
        self.node_ids_V2.append(n)

        return

    def set_mu(self, mu):
        """

        Parameters
        ----------
        mu: function

        Returns
        -------
        None
        """

        self.mu = mu

        return

    @property
    def nodes(self, nodes=None):
        return self.G.vs

    @property
    def edges(self, edges=None):
        return self.G.es

    @property
    def node_ids(self):
        return self.G.vs['name']

    @property
    def hhids(self):
        return {h for _, h in self.G.nodes.data('hhid')}

    @property
    def vaccine_priority(self):
        # return {k:v for k, v in self.G.nodes.data('vaccine_priority')}
        return self.G.vs['vaccine_priority']

    @property
    def time_until_v2(self):
        # return {k:v for k, v in self.G.nodes.data('time_until_v2')}
        return self.G.vs['time_until_v2']

    def set_sick(self, node, E_n=3 * 24, I_n=5 * 24):
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

        # pdb.set_trace()

        self.G.vs[node]['disease_status'] = 'E'
        self.G.vs[node]['remaining_days_exposed'] = E_n
        self.G.vs[node]['remaining_days_sick'] = I_n
        self.node_ids_S.remove(node)
        self.node_ids_E.append(node)
        return

    def decrement(self):

        # disease_status = nx.get_node_attributes(self.G, 'disease_status')
        # remaining_days_sick = self.G.vs['remaining_days_sick']
        # remaining_days_exposed = self.G.vs['remaining_days_exposed']
        # time_until_v2 = self.G.vs['time_until_v2']

        for i in self.node_ids_I:
            if self.G.vs[i]['remaining_days_sick'] == 0:
                # Determine if the nodes die based off of their age
                if next(self.mu[self.G.vs[i]['age']]):
                    self.node_ids_I.remove(i)
                    self.node_ids_D.append(i)
                    self.G.vs[i]['disease_status'] = 'D'
                    # print("Node", i, "has died :(")
                else:
                    self.G.vs[i]['disease_status'] = 'R'
                    self.node_ids_I.remove(i)
                    self.node_ids_R.append(i)
                    # print("Node", i, "recovers")
                continue
            else:
                self.G.vs[i]['remaining_days_sick'] -= 1

        for e in self.node_ids_E:
            if self.G.vs[e]['remaining_days_exposed'] == 0:
                self.G.vs[e]['disease_status'] = 'I'
                self.node_ids_E.remove(e)
                self.node_ids_I.append(e)
                # print("Node", e, "became symptomatic")
                continue
            else:
                self.G.vs[e]['remaining_days_exposed'] -= 1

        for v1 in self.node_ids_V1:
            if self.G.vs[v1]['time_until_v2'] > 0:
                self.G.vs[v1]['time_until_v2'] -= 1
            else:
                continue

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
            'nodes': self.G.get_vertex_dataframe(),  # pickle.loads(pickle.dumps(self.G.vs)),
            'edges': self.G.get_edge_dataframe()  # pickle.loads(pickle.dumps(self.G.es))
        })

    def process_history(self):
        """
        Turns the history object into a pd.DataFrame
        Returns
        -------
        pd.DataFrame

        """

        # pdb.set_trace()

        if not bool(self.history):
            raise ValueError("Must have at least one saved history")

        disease_status = []
        vaccine_status = []
        edges = []

        for time, ne in enumerate(self.history):
            disease_status.append(ne['nodes']['disease_status'].rename(time))
            vaccine_status.append(ne['nodes']['vaccine_status'].rename(time))
            edges.append(ne['edges']['source'].rename(time))
            # edges.append([e for e in ne['edges']])

        return pd.concat(disease_status, axis=1), pd.concat(vaccine_status, axis=1), pd.concat(edges, axis=1)

    def transmit(self, beta, ve, E_dist, I_dist):
        """

        Parameters
        ----------
        Transmits between any eligible nodes

        Returns
        -------
        list of newly infected nodes


        """

        disease_status = self.G.vs['disease_status']
        vaccine_status = self.G.vs['vaccine_status']

        for e in self.G.es:
            # See if either node is infected
            n1 = e.source
            n2 = e.target
            mask_usage = e['protection']

            n1_ds = disease_status[n1]
            n2_ds = disease_status[n2]

            # Make sure only one is I
            if (n1_ds == 'I' and n2_ds == 'S') or (n1_ds == 'S' and n2_ds == 'I'):
                # Identify which node is sick
                node_to_infect = n1 if n1_ds == 'S' else n2

                # Protection
                vaccine = vaccine_status[node_to_infect]

                # Flip coins
                m = next(beta[mask_usage])
                v = next(ve[vaccine])
                if m and v:
                    self.set_sick(node_to_infect, next(E_dist), next(I_dist))
                    # Need to update the disease_status dict so that we don't double infect
                    disease_status[node_to_infect] = 'E'
                    # print("Node", node_to_infect, "has been infected")

            else:
                continue

        return

    def transmit_daytime(self, beta: dict, p_mask: float, E_dist, I_dist, ve, stubs) -> None:
        """
        Transmits during each hour of the workday
        Assume a 10 hour day. Probability of a node having a contact each hour is num_cc_nonhh/10.
        Create subpopulation of nodes that have any contact in a given hour and join them to each other.

        Returns
        -------
        Modifies pop in place, returns nx.Graph for anim_list

        """

        self.remove_edges()

        # Generate the number of stubs for each node
        # num_cc_nonhh = dict(zip(self.node_ids, self.G.vs['num_cc_nonhh']))
        # subpop = list(chain(*[
        #     [k] * next(stubs[v // 10]) for k,v in num_cc_nonhh.items()
        # ]))
        #
        # # Shuffle this list and split into two sublists
        # shuffle(subpop)
        # s1, s2 = split_list(subpop)
        #
        # # Add edges
        # # Probability that node is wearing a mask is p_mask.
        # edges = [(n1, n2, {'protection': next(p_mask), 'household': False})
        #          for n1, n2 in zip(s1, s2) if n1 != n2]
        #
        # self.add_edges([(n1, n2) for n1, n2, d in edges],
        #                attributes={'protection': [d['protection'] for n1, n2, d in edges], 'household': False})

        degdist = [next(stubs[v // 10]) for v in self.G.vs['num_cc_nonhh']]
        # Check to make sure degdist is valid
        if (sum(degdist) % 2) != 0:
            # Pick a random nonzero node and decrement
            # Do this by generating a random list of the indices, shuffling it, and then traversing until we find the
            # first nonzero
            index = [x for x in range(len(degdist))]
            shuffle(index)
            for i in index:
                if degdist[i] == 0:
                    continue
                elif degdist[i] > 0:
                    degdist[i] -= 1
                break

        if not ig.is_graphical(degdist):
            raise ValueError("Invalid degree sequence!")


        edges = self.G.Degree_Sequence(degdist, method="no_multiple").es
        edges = [(e.source, e.target)
                 for e in edges]
        self.add_edges(edges, attributes={'protection': [next(p_mask) for _ in edges], 'household': False})

        self.transmit(beta, ve, E_dist, I_dist)
        self.decrement()

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
        Modifies pop in place

        """

        # Loop through each hhid and transmit through all the tuples

        self.transmit(beta, ve, E_dist, I_dist)
        # self.profileTransmit(beta, ve, E_dist, I_dist)
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
        v1_prior = dict(zip(self.node_ids, self.G.vs['vaccine_priority']))  # self.vaccine_priority
        v1_prior = invert_dict(v1_prior)
        del v1_prior[-1]

        # V1
        # Shuffle all lists:
        for k in v1_prior.keys():
            shuffle(v1_prior[k])

        while n_remaining > 0:
            # The node that we're vaccinating
            try:
                max_prior = max(v1_prior.keys())
                if (len(v1_prior[max_prior]) == 0):
                    del v1_prior[max_prior]
                max_prior = max(v1_prior.keys())
            except ValueError:
                break

            nid = v1_prior[max_prior].pop()

            # print("Node", nid, "got the first dose of vaccine")

            # Set the node's status to V1
            self.V1(nid, time_until_v2)

            n_remaining -= 1

        # V2
        v2_prior = dict(zip(self.node_ids, self.G.vs['time_until_v2']))
        v2_prior = invert_dict(v2_prior)
        try:
            v2_prior = v2_prior[0]
        except KeyError:
            return

        shuffle(v2_prior)

        while n_remaining_2 > 0 and len(v2_prior) > 0:
            # The node that we're vaccinating
            nid = v2_prior.pop()
            # print("Node", nid, "got the second dose of vaccine")

            # Set the node's status to V1
            self.V2(nid)

            n_remaining_2 -= 1

        return


