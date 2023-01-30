# Robin Hood hash table
In this repository you can find realization of open addressing hash table, which uses Robin Hood optimization.

## Key Idea of Robin Hood optimization
The algorithm is based on the notion of probe sequence lengths (PSL). The PSL of a key is the number of probes required to find the key during lookup.


A key with a low PSL can be thought of as rich, and a key with a high PSL can be thought of as poor. 
When inserting a new key the algorithm moves the rich in favor of the 
poor (“takes from the rich and gives to the poor”), hence the name Robin Hood hashing.

## Run code
* To make sure that the programm passes all unit tests you can run [tester.cpp](tester.cpp) file
* Usage example you can find in [main.cpp](main.cpp) file

## Supported operations
Full description of the problem that this realization of hash table solve and all supported opearations you can find in [statement.pdf](statement.pdf)
