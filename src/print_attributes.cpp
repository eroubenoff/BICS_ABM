#include<igraph.h>
#include<stdio.h>
#include<string.h>
#include<stdbool.h>
#include<vector>
#include<string>
#include<iostream>
#include<fstream>
#include <sstream>  
#include <unordered_map>
// #include "BICS_ABM.h" 

/* Prints graph, vertex and edge attributes stored in a graph. */
void print_attributes(const igraph_t *g, bool nodes_only) {
    igraph_vector_int_t gtypes, vtypes, etypes;
    igraph_strvector_t gnames, vnames, enames;
    igraph_integer_t i, j;

    igraph_vector_int_init(&gtypes, 0);
    igraph_vector_int_init(&vtypes, 0);
    igraph_vector_int_init(&etypes, 0);
    igraph_strvector_init(&gnames, 0);
    igraph_strvector_init(&vnames, 0);
    igraph_strvector_init(&enames, 0);

    igraph_cattribute_list(g,
            &gnames, &gtypes,
            &vnames, &vtypes,
            &enames, &etypes);

    /* graph attributes */
    for (i = 0; i < igraph_strvector_size(&gnames); i++) {
        printf("%s=", STR(gnames, i));
        if (VECTOR(gtypes)[i] == IGRAPH_ATTRIBUTE_NUMERIC) {
            igraph_real_printf(GAN(g, STR(gnames, i)));
            putchar(' ');
        } else {
            printf("\"%s\" ", GAS(g, STR(gnames, i)));
        }
    }
    printf("\n");

    /* vertex attributes */
    for (i = 0; i < igraph_vcount(g); i++) {
        printf("Vertex %" IGRAPH_PRId ": ", i);
        for (j = 0; j < igraph_strvector_size(&vnames); j++) {
            printf("%s=", STR(vnames, j));
            if (VECTOR(vtypes)[j] == IGRAPH_ATTRIBUTE_NUMERIC) {
                igraph_real_printf(VAN(g, STR(vnames, j), i));
                putchar(' ');
            } else {
                printf("\"%s\" ", VAS(g, STR(vnames, j), i));
            }
        }
        printf("\n");
    }

    if (!nodes_only) {
        /* edge attributes */
        for (i = 0; i < igraph_ecount(g); i++) {
            printf("Edge %" IGRAPH_PRId " (%" IGRAPH_PRId "-%" IGRAPH_PRId "): ", i, IGRAPH_FROM(g, i), IGRAPH_TO(g, i));
            for (j = 0; j < igraph_strvector_size(&enames); j++) {
                printf("%s=", STR(enames, j));
                if (VECTOR(etypes)[j] == IGRAPH_ATTRIBUTE_NUMERIC) {
                    igraph_real_printf(EAN(g, STR(enames, j), i));
                    putchar(' ');
                } else {
                    printf("\"%s\" ", EAS(g, STR(enames, j), i));
                }
            }
            printf("\n");
        }
    }

    igraph_strvector_destroy(&enames);
    igraph_strvector_destroy(&vnames);
    igraph_strvector_destroy(&gnames);
    igraph_vector_int_destroy(&etypes);
    igraph_vector_int_destroy(&vtypes);
    igraph_vector_int_destroy(&gtypes);
}
