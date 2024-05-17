# Computer Architecture Simulator

*Overview*
This simulator emulates the pipeline stages of a simple computer architecture, including instruction fetching, decoding, execution, memory access, and write-back.

*How it works?*
by giving it any assembly-like instructions and it simply pasre them into binary according to the ISA and store them in instructions-part of the unified memory and then apply the pipelining stages fetch() decode() execute() memoryAccess() and writeBack()

*Challenges and Solutions*
we encountered an issue where each clock cycle the values brought from the previous stage so we decided to call the stages methods backwards, it gives the same result however keeps the data without it being overwritten.
