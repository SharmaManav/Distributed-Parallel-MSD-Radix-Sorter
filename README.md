# Distributed-Parallel-MSD-Radix-Sorter

Distributed parallel most significant digit radix sorter in C++
charAt, sort, and sort2 functions are all in regards to the radix sorting.

ParallelRadixSort::msd takes in a vector of a reference of a vector of unsigned integers. For each list passed through the function, a
new thread is created and is then sorted. This could be optimized by maybe creating a thread pool as there is some overhead associated with
creating a new thread for each list.

After that the distributed portion of the program comes in with the client and server. Server listens for a connection and accepts anyones
that are incoming. The recv function is called and receives messages. Upon receiving, message is converted from network byte order to host
byte order so that the message can be interpreted on the receiving computer's architecture. The message is then sorted and sent back to the
client.

The client sends the server the lists & the server then sorts them and sends them back as they finish. Once received back from the server,
the list is returned to the original test vector and checked for correctness.
