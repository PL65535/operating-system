# operating-system
code about operating system experiment
Two process share a shared memory-buffer, writer process get string from user, and put the string into buffer.
If the buffer is full, writer process blocks. Reader process get content from the buffer, and blocks if the buffer is empty.
This experiment is about process synchronization using Linux function.
