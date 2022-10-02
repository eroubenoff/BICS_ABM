# cython: profile=True
# cython: linetrace=True
import numpy as np
cimport numpy as np
import cython

cpdef decrement_cy(
    ds: unicode[:],
    rds: cython.int[:],
    rde: cython.int[:],
    tv2: cython.int[:],
    mu: cython.bint[:]
    ):

    cdef int n_nodes = len(ds)
    cdef int i = 0
    for i in range(0, n_nodes):
        if ds[i] == "I":
            if rds[i] == 0:
                if mu[i]:
                    ds[i] = "D"
                else:
                    ds[i] = "R"
            else:
                rds[i] -= 1
        elif ds[i] == "E":
            if rde[i] == 0:
                ds[i] = "I"
            else:
                rde[i] -= 1

        if tv2[i] > 0:
            tv2[i] -= 1


    return ds, rds, rde, tv2


cpdef transmit_cy(
    np.ndarray edgelist,
    np.ndarray mask_usage,
    np.ndarray ds,
    np.ndarray vs,
    beta,
    ve):

    cdef int n_edges = edgelist.shape[0]

    nodes_to_infect = []

    cdef int n1, n2
    for e in range(n_edges):
        n1 = edgelist[e,0]
        n2 = edgelist[e,1]

        if (ds[n1] == "I" and ds[n2] == "S") or (ds[n1] == "S" and ds[n2] == "I"):
            node_to_infect = n1 if ds[n1] == "S" else n2

            b = next(beta[mask_usage[e]])
            v = next(ve[vs[node_to_infect]])

            if b and v:
                nodes_to_infect.append(node_to_infect)
                ds[node_to_infect] = "E"

    return nodes_to_infect



cpdef extract_nodes_from_edgelist_cy(edgelist):
    cdef int n_edges = len(edgelist)
    sourcelist = np.zeros((n_edges), dtype=np.int)
    targetlist = np.zeros((n_edges), dtype=np.int)

    for i in range(n_edges):
        sourcelist[i] = edgelist[i].source
        targetlist[i] = edgelist[i].target
        # sourcelist.append(edgelist[i].source)
        # targetlist.append(edgelist[i].target)

    return sourcelist, targetlist
