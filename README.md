## This is a program written by Ethan Woollet for COP4520 with professor Juan Parra at UCF.

There are 2 C++ files in this repository because one of them was my original approach via 'smart' brute force. I left it in the repo for comparisons and benchmarking.
Also, note that the instructions for this assignment are included in the PDF file in this repository.


# Compilation Instructions

To compile and run this program, download 'prime_sieve.cpp' or clone this repo.
Then, navigate the the directory containing that file and perform the following commands

- `g++ -o <executable name> prime_sieve.cpp`
- `./<executable name> > primes.txt`


# Notes On The Program

There are 2 variables in the code called `THREAD_COUNT` and `MAX_VAL` that can be changed before compilation to affect how the program runs.

Changing `THREAD_COUNT` will affect how many threads are spawned in the thread pool and will affect performance. The default is 8.
Changing `MAX_VAL` will just change the range of primes the algorithm will search for. The default is 100,000,000 

Interestingly, I tested the program using a single thread and it actually runs noticably faster on my machine (about ~0.3s faster), which could mean several things: 

- My program is poorly made
- My implementation of this algorithm does not work well with multiple cores
- My program does not reach a point where the runtime costs of multithreading are overtaken by the parallel processing benefits

Regardless of the answer, it is interesting.


# Proof of Correctness

This program implements a classic prime finding algorithm called the Sieve of Eratosthenes. There are plenty of descriptions and implementations of this algorithm, but is the basic algorithm is as follows:

Create a list of all integers from 2 to n.
For each number in the list up to the square root of n starting at 2, mark off all multiples of that number.
All numbers that are not marked off are prime.

That's it. It's pretty simple.
It works on a 2 simple premises: 

1. Prime numbers are numbers that are only divisible by 1 and itself.
2. All composite numbers are multiples of one or more primes

From these, we can conclude that, if you go through a list of integers and remove any integer that is a multiple of another, then the remaining integers *must* be prime.
It is also important to note that we only mark off multiples of numbers up to the square root of `MAX_VAL` because it is impossible for any two factors of a number to both be greater than that numbers square root.

There are many papers discussing variations and improvements for both single threaded and multithreaded implementations of the sieve, but I just read the wikipedia page and wrote it in C++, I'm not trying to break new ground here.

I also verfied accuracy independently using the Wolfram Alpha math engine, and it matches digit for digit.

In terms of runtime, on my machine using 8 cores of an Intel i-10600K CPU, and searching for all prime numbers up to 100,000,000 , it usually completes in ~1.6s ± 0.2s.

I did include certain attempts at improvements such as only counting odd numbers as 'candidates' to pass to threads for evaluation as every even integer after 2 is a multiple of 2, and several other minor things, but I doubt that those other improvements will have any major impact on runtime.

As for proof of an equal distribution of labor among the threads, I ensured this by using a thread pool and a job queue to ensure that jobs are always being accepted by threads soon after they idle, meaning that they all perform close to the same number of jobs during the program's runtime.

Something I would also like to note about this program is that there is a lot of waiting done due to the shared `table` variable which threads have to perform long I/O operations on when marking off multiples. There are surely ways of reducing the time spent waiting, but they are not necessary for this program's requirements. 
