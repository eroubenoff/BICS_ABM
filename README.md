# An Agent-Based Network Simulation for COVID-19 transmission
## Ethan Roubenoff

This repository contains code for running the BICS ABM and replicating
dissertation work by Ethan Roubenoff.

BICS ABM is a microsimulation model for SARS-CoV-2 transmission that utilizes
contact data collected by the Berkeley Interpersonal Contact Survey (BICS; Feehan and Mahmud 2020, 2021).
The simulation involves using BICS respondents demographic and household data to
create a population universe of household contacts, then connecting random
contacts randomly according to the Configuration Model for network connection. 
More details can be provided upon request to eroubenoff@berkeley.edu.

The simulation is written in C++ using the iGraph library and is compiled
as a dynamic library. An API is written in Python to run the simulation.

To use the BICS ABM simulation, install the program 
according to the procedure below, making sure that CMake
is able to access the iGraph library properly (please consult the 
iGraph documentation for this). 
Then, run the base simulation in 
python as:

```python
>> from BICS_ABM import BICS_ABM
>> BICS_ABM()
```

A number of parameters can be passed to the simulation:

| Parameter   | Description | Default Value |
| :---        | :----       | :---          |
| n\_hh        | Number of Households | 1000 |
| wave | BICS wave to use | 6 |
| gamma\_min, \_max| Lower, upper bounds for latent period (in hours) | 2\*24, 4\*24 |  

# Requirements: 

* A C++ compiler with the C++17 standard (I use Apple Clang++)
* [CMake](https://cmake.org/) Version > 3.18
* [igraph C library](https://igraph.org/c/html/latest/igraph-Installation.html)
* Python 3.10 or greater
* (optional) [gTest](http://google.github.io/googletest/)

# Installation

To install, just run `make`, which will call `cmake` to build the 
dynamic library and excecutable. 

# Running the model

It is recommended that you use the python interface to run the code. 



