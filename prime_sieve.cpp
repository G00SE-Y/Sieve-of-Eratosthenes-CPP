/*
 *  Multithreaded Prime Enumerator
 *
 *  Author: Ethan Woollet
 *  Date: Jan 2023 
 * 
 *  Professor: Juan Parra
 *  Class: Parallel and Distributed Processing (COP4520)
 * 
 */


#include <iostream>
#include <cmath>
#include <chrono>
#include <mutex>
#include <queue>
#include <thread>

using namespace std;
using namespace std::chrono;


// function prototypes
void fill_table(); // initializes the array used in sieve()
void sieve(int max); // runs the Sieve of Eratosthenes algorithm to generate primes
void mark_multiples(int val); // function used by the sieve
void print_top_primes(); // prints the largest 10 primes
void print_primes(); // debugging function
void thread_function(); // function that each thread runs
void print_prime_stats(int duration); // output for my preferences
void print_assignment_output(int duration); // output for the assignment


// global variables
int THREAD_COUNT = 8; // number of threads spawned in threadpool
const int MAX_VAL = 100000000; // max number to check for primeness


// thread shared values
queue<int> jobs; // queue of jobs to be performed by the threadpool
bool are_jobs = false; // flag for threads to exit idle state and attempt to start a new job
bool all_jobs_done = false; // flag for all jobs having been queued and completed
int table[MAX_VAL + 1]; // shared table used by the sieve algorithm


// mutex locks for shared values in the form `mut_<variable name>`
mutex mut_jobs;
mutex mut_are_jobs;
mutex mut_all_jobs_done;
mutex mut_table;


int main(void) {

    fill_table(); // fill table with 1's

    auto start = high_resolution_clock::now(); // start time
    sieve(MAX_VAL);
    auto stop = high_resolution_clock::now(); // end time

    auto duration = duration_cast<microseconds>(stop - start);

    // format output
    // print_prime_stats(duration.count());
    print_assignment_output(duration.count());

    return 0;
}


void sieve(int max) {
    
    jobs.push(2); // hardcode in 2 as the first job
    are_jobs = true;

    for(int i = 3; i*i <= max; i+= 2 ) { // queue up remaining jobs
        jobs.push(i);
    }

    queue<thread> pool;
    
    for(int i = 0; i < THREAD_COUNT; i++) { // spawn all threads in the pool
        thread t(thread_function);
        pool.push(move(t));
    }

    while(true) { // wait for all jobs to be completed, then notify threads
        mut_jobs.lock();
        if(jobs.empty()) {
            mut_jobs.unlock();

            mut_all_jobs_done.lock();
            all_jobs_done = true;
            mut_all_jobs_done.unlock();
            break;
        }
        else {
            mut_jobs.unlock();
        }
    }

    while(!pool.empty()) { // wait for all threads to finish, then destroy them (with lasers)
        pool.front().join();
        pool.pop();
    }

    return;
}


void thread_function() {

    mut_all_jobs_done.lock();
    while(!all_jobs_done) { // idle loop
        mut_all_jobs_done.unlock();

        mut_are_jobs.lock();
        if(are_jobs) { // if jobs in queue, then perform one and return to idle
            mut_jobs.lock();

            int val = jobs.front();
            jobs.pop(); // remove job from queue
            if(jobs.empty()) are_jobs = false;

            mut_jobs.unlock();
            mut_are_jobs.unlock();

            mark_multiples(val); // do job
        }
        else {
            mut_are_jobs.unlock();
        }

        mut_all_jobs_done.lock();
    }

    mut_all_jobs_done.unlock();

    return;
}


void mark_multiples(int val) {

    mut_table.lock();

    if(table[val] == 0) { // if it has been marked off, ignore it
        mut_table.unlock();
        return;
    }

    for(int i = 2; val * i <= MAX_VAL; i++) { // mark off all multiples of the current value
        table[val * i] = 0;
    }

    mut_table.unlock();
    return;
}


void fill_table() {
    for(int i = 0; i <= MAX_VAL; i++) {
        table[i] = 1;
    }

    // 0 and 1 are neither prime nor composite, but to make the logic easier, they are still in the table. 
    // So, we must manually mark them off.
    table[0] = 0;
    table[1] = 0;
}


void print_top_primes() {

    int top_primes[10];

    for(int i = 0, count = 0; i <= MAX_VAL && count < 10; i++) { // scan through the table from back to front adding primes to the list until we have added 10
        
        if(table[MAX_VAL - i] == 1) {
            top_primes[9 - count] = MAX_VAL - i;
            count++;
        }
    }

    // formatting the output
    cout << "[";
    for(int i = 0; i < 9; i++) {
        cout << top_primes[i] << ", ";
    }
    cout << top_primes[9] << "]";

    return;
}


void print_time(int time) {
    
    if(time > 1000000) {
        cout << (float) time / 1000000.0 << "s ";
    }
    else if(time > 1000) {
        cout << (float) time / 1000.0 << "ms ";
    }
    else {
        cout << time << "μs ";
    }

    return;
}


void print_assignment_output(int duration) {

    print_time(duration);

    long long int sum = 0;
    int count = 0;

    for(int i = 2; i <= MAX_VAL; i++) {

        if(table[i] == 1) {
            count++;
            sum += i;
        }

    }

    cout << count << " ";
    cout << sum << " ";

    print_top_primes();

    return;
}


// -----> DEBUGGING FUNCTIONS <----- //
void print_primes() {

    for(int i = 2; i <= MAX_VAL; i++) {

        if(table[i] == 1) { // print if not marked off (prime)
            cout << i << " ";
        }

    }

    return;
}


void print_prime_stats(int duration) {

    cout << "\nAll done!\nElapsed Time: ";
    print_time(duration); 
    cout << "\n";

    cout << "\n10 largest primes: ";
    print_top_primes();

    long long int sum = 0;
    int count = 0;

    for(int i = 2; i <= MAX_VAL; i++) {

        if(table[i] == 1) {
            count++;
            sum += i;
        }

    }

    cout << "\nNumber of primes: " << count << "\n";
    cout << "Sum of primes: " << sum << "\n\n";

    return;
}