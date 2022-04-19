class Node:
    def __init__(self, node_id: int, age: int, hhsize: int,
                 num_cc_nonhh: int, ethnicity: str,
                 disease_status: str = "S", remaining_days_sick: int = 0,
                 connected_nodes=set()):
        self._node_id = str(node_id)
        self._age = age
        self._hhsize = hhsize
        self._num_cc_nonhh = num_cc_nonhh
        self._ethnicity = ethnicity
        self._disease_status = disease_status
        self._remaining_days_sick = remaining_days_sick
        self._connected_nodes = connected_nodes

    @property
    def node_id(self):
        return self._node_id

    @property
    def age(self):
        return self._age

    @property
    def hhsize(self):
        return self._hhsize

    @property
    def num_cc_nonhh(self):
        return self._num_cc_nonhh

    @property
    def ethnicity(self):
        return self._ethnicity

    @property
    def disease_status(self):
        return self._disease_status

    @disease_status.setter
    def disease_status(self, new_status: str):

        if new_status not in ["S", "I", "R"]:
            raise ValueError("new_status must be either S, I, or R")

        self._disease_status = new_status

    @property
    def remaining_days_sick(self):
        return self._remaining_days_sick

    @remaining_days_sick.setter
    def remaining_days_sick(self, new_value=None):
        self._remaining_days_sick = new_value

    def set_sick(self, duration: int = 10):
        self.disease_status = "I"
        self.remaining_days_sick = duration

    @property
    def connected_nodes(self):
        return self._connected_nodes

    @connected_nodes.setter
    def connected_nodes(self, c):
        self._connected_nodes = c

    @connected_nodes.deleter
    def connected_nodes(self):
        self._connected_nodes = set()

    def add_connection(self, node_ids):
        if isinstance(node_ids, int):
            node_ids = {node_ids}
        if isinstance(node_ids, list):
            node_ids = set(node_ids)
        self.connected_nodes = self.connected_nodes.union(node_ids)
