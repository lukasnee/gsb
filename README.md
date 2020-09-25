# gsb
global static buffer accessible in push/pop stack-like fashion. 
Intended for short lifetime buffer allocation in low-to-medium complexity 
embedded systems as a more predictable alternative to standard heap 
allocation.

- To allocate data from the static buffer use GSB_push(size, ppData).
- To deallocate some data (without erasing) use GSB_pop(size). 

Allocation head pointer grows from gsb head with address 
growing up. 

> NOTE! Make sure you do GSB_pop() shortly after a GSB_push() of 
the same size when allocated buffer is no longer needed !

