# Computer Architecture Simulator

### Overview
This simulator emulates the pipeline stages of a simple computer architecture, including instruction fetching, decoding, execution, memory access, and write-back.

### How it works?
by simply calling fetch(), decode(),execute(), memoryAccess(), writeBack() all at every clock cycle.

### Challenges and Solutions
we encountered an issue where each clock cycle the values brought from the previous stage so we decided to call the stages methods backwards, it gives the same result however keeps the data without it being overwritten.
