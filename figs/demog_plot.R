library(tidyverse)
setwd("~/BICS_ABM/figs")

age = c("[0,18)", "[18,25)", "[25,35)", "[35,45)", "[45,55)", "[55,65)","[65,75)", "[75,85)", "85+")

fertility = c(13.9/(1000 / (3/18)), 61.5/1000, (93.0 + 97.6)/(2*1000), (53.7 + 12.0)/(2*1000), 0, 0, 0, 0, 0)
mortality = c(14.3/100000, 88.9/100000, 180.8/100000, 287.9/100000, 531.0/100000, 1117.1/100000, 2151.3/100000, 5119.4/100000, 15743.3/100000)

df = tibble(
  Age =age, 
  "Fertility*" = fertility, 
  Mortality = mortality
)
df

df <- df %>% pivot_longer(-Age)

demog_plot <- ggplot(df) + 
  geom_col(aes(x = Age, y = value, fill = name), position = "dodge") + 
  xlab("Age") + ylab("Stat (rate per capita)") + 
  cowplot::theme_cowplot(10) + 
  labs(title="Demographic Vital Rates Used At Baseline", caption = "*Fertility rates are for Females only") + 
  theme(legend.title= element_blank())

demog_plot

ggsave("demog_plot.png", demog_plot, width = 7, height= 5)

mortality = c(mortality, 1)
fertility = c(fertility, 0)
lx = cumprod(1-mortality)
plot(lx)

le_exp <- function(a, r, l, f) {
  return(exp(-a*r) * l * f)
}

le_fn <- function(r) {
  age_mins <- c(0, 18, 25, 35, 45, 55, 65, 75, 85, 100)
  le = 0
  for (a in 1:100) {
    a = which.min(abs(a - age_mins))
    le = le + le_exp(a, r, lx[a], fertility[a])
  }
  
  return(abs(le-1))
}

range = seq(-10, 10, by=.1)
plot(range, le_fn(range))

optimize(le_fn, c(-10, 10))
