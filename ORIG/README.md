=============================
Lz77 Software Implementation
=============================

- This is an open-source Lz77 software implementation by Michael Dipperstein. One can look for more information about it on <http://michael.dipperstein.com/lzss/index.html>. 

- In our project we modified the hash.c to follow the same hashing strategy as what we used in the hardware design. The new file is called hash_dfe.c.   

- In the software/ folder, simply `make` will generate the executable file **Lz77Compress**. 

- The executable file will be used to decode the file encoded by the DFE, and be used to encode the input so that we can compare its speed with the DFE.
