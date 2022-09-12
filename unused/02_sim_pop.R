# -----------------------------------------------------------------------------
# 02_sim_pop.R

# This script adds functions to the BICS environment that simulate a starting
# population based on the egos 


# Author: Ethan Roubenoff
# Date: 10 Dec 2021
# -----------------------------------------------------------------------------

#' sim_pop
#' 
#' Simulates a starting pop from the BICS based on probability of inclusion
#' weights
#' 
#' Assumes that BICS$egos exists
#' 
#' @param n int starting size 
#' @param weights str of weight column name
#' @return none. Adds resulting DF to BICS$pop
sim_pop <- function(n = 1000, weights = "weight_pooled"){
  
  if (!exists("egos")) {
    stop("Must load data first")
  }
  
  pop <- slice_sample(egos, n = n, weight_by = weight_pooled, replace = TRUE)
  
  assign("pop", pop, rlang::env_parent())
}

environment(sim_pop) <- BICS

if (run) {
  sim_pop()
}

