import pdb

from Node import Node
from Edge import Edge


class Population:

    def __init__(self, nodelist: list, edgelist: list):
        self._nodelist = {}  # {n.node_id: n for n in nodelist}
        self._edgelist = {}  # {e.edge_id: e for e in edgelist}
        self._node_ids = self._nodelist.keys()
        self._edge_ids = self._edgelist.keys()
        self.add_nodes(nodelist)
        self.add_edges(edgelist)

    @property
    def nodelist(self):
        return self._nodelist

    @nodelist.setter
    def nodelist(self, new_value):
        self._nodelist = new_value

    @property
    def edgelist(self):
        return self._edgelist

    @edgelist.setter
    def edgelist(self, new_value):
        self._edgelist = new_value

    @property
    def node_ids(self):
        return self._node_ids

    @node_ids.setter
    def node_ids(self, new_value):
        self._node_ids = new_value

    @property
    def edge_ids(self):
        return self._edge_ids

    @edge_ids.setter
    def edge_ids(self, new_value):
        self._edge_ids = new_value

    @property
    def node_ids_S(self):
        return [n.node_id for n in list(self.nodelist.values()) if n.disease_status == "S"]

    @property
    def node_ids_I(self):
        return [n.node_id for n in list(self.nodelist.values()) if n.disease_status == "I"]

    @property
    def node_ids_R(self):
        return [n.node_id for n in list(self.nodelist.values()) if n.disease_status == "R"]

    @property
    def disease_status(self):
        return self.node_ids_S, self.node_ids_I, self.node_ids_R

    def add_nodes(self, new_nodelist, overwrite=False):
        """
        Adds new nodes to the population by appending them to nodelist (dict) and node_ids (list).
        Duplicates are never allowed.

        Parameters
        ----------
        new_nodelist: Node or list of Nodes
            Nodes to add or replace existing nodelist
        overwrite: Bool
            Should the new list replace the old?

        Returns
        -------
        none

        """

        if isinstance(new_nodelist, Node):
            new_nodelist = [new_nodelist]

        old_nodelist = self.nodelist
        old_node_ids = self.node_ids
        new_node_ids = [x.node_id for x in new_nodelist]
        new_nodelist = {n.node_id: n for n in new_nodelist}

        if not overwrite:
            i = [item for item in new_node_ids if item in old_node_ids]
            if len(i) > 0:
                raise ValueError("Node ids " + str(i) + " already present in nodelist")

            self.nodelist = {**old_nodelist, **new_nodelist}
            self.node_ids = [*old_node_ids, *new_node_ids]

            return

        else:
            self.nodelist = new_nodelist
            self.node_ids = new_node_ids

            return

    def add_edges(self, new_edgelist, dupliucates_ok=True, overwrite=False):
        """
        Adds new edges to the population by appending them to edgelist (dict) and edge_ids (list).
        TODO: Add edge connects to Node objects in Nodelist.

        Parameters
        ----------
        new_edgelist: Edge or list of Edges
            New edges to add or replace existing list
        dupliucates_ok: Bool
            If an edge already exists in the dataset, should throw error?
        overwrite: Bool
            Should the passed new_edgelist replace the existing list?

        Returns
        -------
        none

        """

        if isinstance(new_edgelist, Edge):
            new_edgelist = [new_edgelist]

        old_edgelist = self.edgelist
        old_edge_ids = self.edge_ids
        new_edge_ids = [x.edge_id for x in new_edgelist]
        new_edgelist = {e.edge_id: e for e in new_edgelist}

        # Add connections to the nodes
        # for e in new_edgelist:
        #     # if e.node1_id not in self.node_ids or e.node2_id not in self.node_ids:
        #     #     # Make sure nodes in the new edgelist are in the nodelist
        #     #     raise ValueError("Nodes in Edge must be found in population")
        #     self.nodelist[e.node1_id].add_connection(e.node2_id)
        #     self.nodelist[e.node2_id].add_connection(e.node1_id)

        if not overwrite:
            if not dupliucates_ok:
                i = [item for item in new_edge_ids if item in old_edge_ids]
                if len(i) > 0:
                    raise ValueError("Edge ids " + str(i) + "already present in edgelist")

            self.edgelist = {**old_edgelist, **new_edgelist}
            self.edge_ids = [*old_edge_ids, *new_edge_ids]

        else:
            self.edgelist = new_edge_ids
            self.edge_ids = new_edge_ids

        return

    def delete_edge(self, del_edgelist=None):
        """
        Deletes edges passed from the Population

        Parameters
        ----------
        del_edgelist: List of edge ids to delete. If None is passed, deletes all edges.

        Returns
        -------
        none

        """

        if del_edgelist is None:
            self.edgelist = {}
            self.edge_ids = []
            return

        if isinstance(del_edgelist, Edge):
            del_edgelist = [del_edgelist]

        old_edge_ids = self.edge_ids
        del_edge_ids = [x.edge_id for x in del_edgelist]

        i = [item for item in del_edge_ids if item not in old_edge_ids]
        if len(i) > 0:
            raise ValueError("Edge ids " + str(i) + "not present in edgelist")

        self.edgelist.pop(del_edge_ids)
        self.edge_ids.pop(del_edge_ids)

        return

    def _transmit(self, e: Edge, n: int = 10):
        """
        Transmits infection, if present, between two nodes. Updates the corresponding
        node's disease status

        Parameters
        ----------
        e: Edge id
            The edge id (format: node1_id-node2_id) to transmit
        n: int
            Number of days they will be sick for. Defaults to 10.

        Returns
        -------
        None
        """

        e = self.edgelist[e]

        if self.nodelist[e.node1_id].disease_status == "S" and self.nodelist[e.node2_id].disease_status == "I":
            self.nodelist[e.node1_id].set_sick(n)
            return e.node1_id
        elif self.nodelist[e.node2_id].disease_status == "S" and self.nodelist[e.node1_id].disease_status == "I":
            self.nodelist[e.node2_id].set_sick(n)
            return e.node2_id

        return

    def transmit(self):
        """
        Cycles through all edges and transmits.

        Returns
        -------
        tuple: list of ids of the newly-infected nodes

        """

        return list(map(self._transmit, self.edgelist))

    def decrement(self, n=1):
        """
        Decrements remaining days sick for all infected nodes. If rds = 0, then recover.

        Parameters
        ----------
        n : int
            number of days to decrement

        Returns
        -------

        """
        for i in self.node_ids_I:
            self.nodelist[i].remaining_days_sick -= n
            if self.nodelist[i].remaining_days_sick == 0:
                self.nodelist[i].disease_status = "R"





