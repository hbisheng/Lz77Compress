=============================
Lz77 compression
=============================

An implementation of Lz77 compression algorithm


Introduction
------------

- Hardware implementation of lossless data compression is important for optimizing the
capacity/cost/power of storage devices. 
- GZIP is a file format used for file compression and
decompression, which is based on one of the most popular algorithms for lossless storage -
DEFLATE. 
- DEFLATE uses a combination of the LZ77 algorithm and Huffman coding.

In this work we use the MaxCompiler programming tool suite to implement LZ77 algorithm
on a FPGA. 

Features
--------

Performance:

- Throughput: 8 input bytes per cycle clock
- Compression ratio: 1.88
- Compression speed: 1.6GB/s on FPGA with a clock frequency of 200MHz

Resources Usage:

- Logic resources: 255764/262400 (97.47%)
- Block memory: 1315/2567 (51.23%)

Usage
----

**Folder Hierarchy(More info can be found in each folder)**

- APP/
	- build/ `(Where you can build/run DFE or simulations)`
	- src/ `(Where you can find all the source codes)`
		- EngineCode/
		- CPUCode/
- TEST/
	- Calgary Corpus/ `(The dataset that can be used)`
- DOCS/ `(The detailed documentations for the project)`
- ORIG/ `(An open-source software implementation of Lz77)`
	- software/


About the design
-----

- One can refer to the DOCS/GZip project_documentation for more information about the project(e.g. detailed explanations about the design, the limitations, and the options.)
