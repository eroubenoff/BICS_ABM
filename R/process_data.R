# Script to take the df_all_waves.csv file and add hh member columns 

library(tidyverse)
setwd("~/BICS_ABM")
path = "~/BICS_ABM/lucid-pipeline"

df_all_waves <- readRDS(file.path(path, "df_all_waves.rds")) %>% filter(wave!= 0 )
# national_wave0 <- readRDS(path, file.path(path, "national_wave0_unweighted.rds")) 
national_wave1 <- readRDS(file.path(path, "national_wave1_unweighted.rds")) %>%
  mutate(across(where(is.ordered), ~as.character(.)))
national_wave2 <- readRDS(file.path(path, "national_wave2_unweighted.rds")) %>%
  mutate(across(where(is.ordered), ~as.character(.)))
national_wave3 <- readRDS(file.path(path, "national_wave3_unweighted.rds")) %>%
  mutate(across(where(is.ordered), ~as.character(.)))
national_wave4 <- readRDS(file.path(path, "national_wave4_unweighted.rds")) %>%
  mutate(across(where(is.ordered), ~as.character(.)))
national_wave5 <- readRDS(file.path(path, "national_wave5_unweighted.rds")) %>%
  mutate(across(where(is.ordered), ~as.character(.)))
national_wave6 <- readRDS(file.path(path, "national_wave6_unweighted.rds")) %>%
  mutate(across(where(is.ordered), ~as.character(.)))

df_join <- bind_rows(
  # national_wave0,
  national_wave1,# %>% select(rid, contains("resp_hh_roster")),
  national_wave2,# %>% select(rid, contains("resp_hh_roster")),
  national_wave3,# %>% select(rid, contains("resp_hh_roster")),
  national_wave4,# %>% select(rid, contains("resp_hh_roster")),
  national_wave5,# %>% select(rid, contains("resp_hh_roster")),
  national_wave6# %>% select(rid, contains("resp_hh_roster")),
)

# There are 19694 responses in df_join but 21142 in df_all_waves
df_all_waves <- left_join(df_all_waves %>% select(rid, weight_pooled), df_join, by = "rid") 

write_csv(df_all_waves, "cpp_version/data/df_all_waves.csv")

