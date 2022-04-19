from Node import Node

class Edge:
    def __init__(self, node_1: Node, node_2: Node):
        if node_1.node_id == node_2.node_id:
            raise ValueError("Cannot contain self-edge")

        self.node1_id = node_1.node_id
        self.node2_id = node_2.node_id
        self.edge_id = str(min(self.node1_id, self.node2_id)) + "-" + str(max(self.node1_id, self.node2_id))
