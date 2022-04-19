# -----------------------------------------------------------------------------
# 01_load_data.R

# This script first creates the BICS environment and then loads data.
# This script assumes that the raw data files are in 
# ~/90days/eroubenoff/contact-surveys-group


# Author: Ethan Roubenoff
# Date: 10 Dec 2021
# -----------------------------------------------------------------------------

library(roxygen2)
library(tidyverse)
run <- TRUE 


# Initialize environment BICS that will contain the data and functions
BICS <- new.env()
BICS$data_path <- file.path("~/90days", "eroubenoff", "contact-surveys-group", "data")


# Test data sources
if (!dir.exists(BICS$data_path)){
  stop("BICS Data not found")
}


#' Load data function
#' 
#' @param wave int between 1 and 5
#' @param path the data path (defualts to BICS$data_path)
#' @return None. sets BICS$egos and BICS$alters 
load_data <- function(wave, path = data_path) {
  
  if (!wave %in% 1:5) {
    stop("Wave must be in range 1, 5")
  }
  
  egos <- file.path(path, "lucid", paste0("national_wave", wave, ".rds"))
  alters <- file.path(path, "lucid", paste0("national_alters_wave", wave, ".rds")) 
  
  assign("egos", readRDS(egos), rlang::env_parent())
  assign("alters", readRDS(alters), rlang::env_parent())
  

  return()
}

environment(load_data) <- BICS

if (run) {
  load_data(wave = 4)
}







