A simple application demonstrating the use of threads to parallelize matrix multiplication.
Parallelization here is done by splitting the lhs matrix into equal-sized chunks of rows,
multiplying each block by the entire rhs matrix, and concatenating the result. In this process,
all memory is handled using pointers to the original dense matrix. Since the chunking is done on
rows of the lhs matrix, the stride is unmodified and no copies of the data are required.

On my system, the threaded version runs in ~0.9-1.1s per iteration whereas the naive version
runs in 3+ seconds for equivalently sized input. However, the threaded runtime increases after each
iteration so I think I have a leak of some sort...
