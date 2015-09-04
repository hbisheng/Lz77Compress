=============================
About building the design	
=============================

- The design can be built with MaxCompiler2014.1 and run on MAIA.
- Make sure the environment variables have been set before building the project. 

Build
--------

Under build/ folder,

- Simulations
	- `make runsim` - run simulations to test the design (may take up to 30 minutes) 
	- `make cleansim` or `make distcleansim` - clean temporary files for simulation

- DFE
	- `make build` - build the design on hardware (may take one day)
	- `make run` - run the design on hardware
	- `make cleandfe` or `make distcleanhw` - clean temporary files for DFE
	
Src
----

Under src/ folder,

- CPUCode/ contains the C++ host code.
- EngineCode/ contains the DFE maxj codes.
