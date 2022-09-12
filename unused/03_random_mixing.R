# -----------------------------------------------------------------------------
# 03_random_mixing.R

# This script includes the simplest random-mixing network simulation model
# based off of the total number of contacts

# Author: Ethan Roubenoff
# Date: 10 Dec 2021
# -----------------------------------------------------------------------------

library(igraph)

sim_pop(n=1000)

df <- BICS$pop %>% mutate(id = row_number()) %>% select(id, num_cc, age)

# Make connections at random
sum(df$num_cc)

# Choose two stubs uniformly at random and connect them to form an edge. Choose another pair from the remaining {\displaystyle 2m-2}{\displaystyle 2m-2} stubs and connect them. Continue until you run out of stubs. The result is a network with the pre-defined degree sequence. The realization of the network changes with the order in which the stubs are chosen, they might include cycles (b), self-loops (c) or multi-links (d) (Figure 1). Yet, the expected number of self-loops and multi-links goes to zero in the N → ∞ limit.[1]





# Helper function to choose stubs 
choose_stubs <- function(df) {
  chosen_stubs <- df %>% 
    filter(remaining_stubs > 0) %>% 
    slice_sample(n=2) %>% 
    pull(id)
  
  return(chosen_stubs)
}


#' gen_network()
#' 
#

gen_network <- function(df) {
  
  df$remaining_stubs <- df$num_cc
  edge_list <- matrix(data = NA, nrow =2* sum(df$num_cc), ncol =2 )
  
  # While there are still remaining stubs:
  counter = 0
  while(sum(df$remaining_stubs) > 1) {
    
    # print(counter)
    # print(sum(df$remaining_stubs))
    
    # choose two stubs:
    chosen_stubs <- choose_stubs(df)
    
    # check that they are valid
    if (length(chosen_stubs) < 2) {
     break 
    }
    if (chosen_stubs[1] == chosen_stubs[2]){print("self-edge"); next}
    
    # if (
    #     (sum(edge_list[,1] == chosen_stubs[1] & edge_list[,2] == chosen_stubs[2],na.rm = TRUE) + 
    #      sum(edge_list[,2] == chosen_stubs[1] & edge_list[,1] == chosen_stubs[2], na.rm=TRUE)
    #     ) > 0 ){
    #   print("multi-edge"); next
    #  }
    
    
    # add to edge list:
    edge_list[counter, ] <- chosen_stubs
    
    # Decrement remaining stubs
    df[df$id %in% chosen_stubs, "remaining_stubs"] <- df[df$id %in% chosen_stubs, "remaining_stubs"] -1
  
    # Increment counter
    counter <- counter + 1  
  }
  
  # drop NA rows
  edge_list <- edge_list[!is.na(edge_list[,1]), ]
  
  
  G <- graph_from_edgelist(edge_list, directed=FALSE)
  G <- set_vertex_attr(G, name = "age", index = df$id, value = df$age)
  
  return(G)
  
}

G <- gen_network(df)


# plot(G)


G


run_sim <- function(G, n_initial = 1) {
  
  G <- set_vertex_attr(G, name = "status", value = "S")
  G <- set_vertex_attr(G, name = "days_since_exposure", value = 0)
  # Randomly get one nodes sick
  initial_sick <- sample(V(G), n_initial)
  G <- set_vertex_attr(G, name = "status", index = initial_sick, value = "I")
  G <- set_vertex_attr(G, name = "days_since_exposure", index = initial_sick, value = 1)
  get.vertex.attribute(G, "status")
  
  
  # pal = c(S="green", I = "red", R = "blue")
  # plot(G, vertex.color = pal[vertex_attr(G, "status")])
  
  status_mat <- matrix(NA_character_, ncol = 365, nrow = length(get.vertex.attribute(G, "status")))
  
  for (i in 1:365) {
    # Evaluate which nodes are infected
    
    status <- get.vertex.attribute(G)
    status_mat[,i] <- status$status
    I <- which(status$status == "I")
    
    # If any I have been sick for more than 10 days, have them recover
    G <- set_vertex_attr(G, name = "status", I[status$days_since_exposure[I] > 10], value = "R")
    
    # Get susceptible neighbors of I
    exposed <- neighbors(G, I) 
    G <- set_vertex_attr(G, name = "status", 
                         exposed[
                           get.vertex.attribute(G,"status", exposed) == "S" & 
                             rbernoulli(length(exposed), p=0.5)], 
                         value = "I")
    
    # Increment I 
    G <- set_vertex_attr(G, name = "days_since_exposure", I, value = status$days_since_exposure[I] +1)
    
  }
  
  status_mat <- tibble(
    S = colSums(status_mat == "S"),
    I = colSums(status_mat == "I"),
    R = colSums(status_mat == "R")
  ) %>% 
    mutate(time = 1:n() )
  
  
  return(status_mat)
  
}


ggplot(run_sim(gen_network(df), n_initial = 50) ) + 
  geom_line(aes(time, S, col = "S"), size = 2) + 
  geom_line(aes(time, I, col = "I"), size = 2) + 
  geom_line(aes(time, R, col = "R"), size = 2) +
  cowplot::theme_cowplot() +
  scale_color_viridis_d()+
  theme(legend.title = element_blank())


