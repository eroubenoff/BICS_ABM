This project contains an agent based epidemiological 
modeling study utilizing data from the Berkeley Interpersonal
Contact Survey (BICS). In this project, we investigate the role of
network patterns in COVID-19 transmission.

### Overview
Respondents in the BICS survey are subsampled with replacement 
to create a set of households.
Then, in 1-hour increments, individuals in the population
form ties with other individuals at random. 
Ties are randomly assigned 'mask' or 'no mask'
status. If one individual in the tie is infected
but the other is not, transmission occurs 
with probability $p$ chosen according to the
tie's mask status. Individuals are assumed to have contact
only with members of their own household between midnight and 
8am and 6pm to midnight; during working hours,
ties are formed only with non-household alters.

### Population Generation
A number of households $$N_{hh}$$ is supplied by the 
researcher. To generate households, respondents in the BICS
survey are sampled with replacement to serve as 
the head of household. A respondent's
probability of being selected as household head
is proportional to the survey weights provided by
Feehan and Mahmud (2020). 
Information provided on the household head's 
household members is used to populate the household.
BICS respondents matching the head's household size
and reported age and gender are sampled with replacement
until the household is of the desired size.
For example, consider a head reporting 2 additional household members
with age and gender (18-25, Male) and (25-35, Female).
BICS respondents matching these criteria who are also in households
of size 3 are  





