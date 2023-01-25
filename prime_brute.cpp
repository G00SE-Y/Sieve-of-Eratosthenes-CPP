/*
 *  Multithreaded Prime Enumerator
 *
 *   Author: Ethan Woollet
 *   Date: Jan 2023 
 * 
 *  Professor: Juan Parra
 *  Class: Parallel and Distributed Processing (COP4520)
 * 
 */


#include <iostream>
#include <cmath>
#include <chrono>
#include <mutex>
#include <vector>
#include <thread>
#include <bits/stdc++.h>

using namespace std;
using namespace std::chrono;

// function prototypes
bool is_prime(int x);
vector<int> get_primes(int max);
void print(vector<int> arr);
void thread_function();
void try_push(int val);
void update_sum_total(int val);
void try_add_if_prime(int val);

// mutex locks
mutex mut_count;
mutex mut_primes;
mutex mut_sum;
mutex mut_total;
mutex mut_jobs;
mutex mut_are_jobs;
mutex mut_all_jobs_done;

// thread shared values
int COUNTER = 3;
vector<int> PRIMES(10);
long long int SUM = 2;
int TOTAL = 1;
vector<int> jobs(0);
bool all_jobs_done = false; 
bool are_jobs = false;

// other
int THREAD_COUNT = 8;
int MAX_VAL = 1000000; // 100000000


int main(void) {

    auto start = high_resolution_clock::now();
    auto primes = get_primes(MAX_VAL);
    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(stop - start);
    cout << "\nAll done!\nElapsed Time: " << duration.count() << "ms\n\n";

    sort(PRIMES.begin(), PRIMES.end());
    cout << "\n10 largest primes: ";
    print(PRIMES);
    cout << "\nNumber of primes: " << TOTAL << "\n";
    cout << "Sum of primes: " << SUM << "\n";


    return 0;
}


vector<int> get_primes(int max) {
    
    vector<thread> pool;
    vector<int> primes(10);
    PRIMES[0] = 2;


    int iter_count = (max - 3) / 2;
    if (max % 2 == 1) {
        iter_count += 1;
    }
    
    for(int i = 0; i < THREAD_COUNT; i++) {
        thread t(thread_function);
        pool.push_back(move(t));
    }

    for(int i = 3; i <= max; i+= 2 ) {
        mut_jobs.lock();
        jobs.push_back(i);
        mut_jobs.unlock();
        are_jobs = true;
    }

    while(true) {
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
            std::this_thread::sleep_for (std::chrono::nanoseconds(100));
        }
    }

    while(!pool.empty()) { // wait for all threads to finish

        pool.back().join();
        pool.pop_back();
    }

    return primes;
}


void thread_function() {

    mut_all_jobs_done.lock();
    while(!all_jobs_done) {
        mut_all_jobs_done.unlock();

        mut_are_jobs.lock();
        if(are_jobs) {

            mut_jobs.lock();
            int val = jobs.back();
            jobs.pop_back();
            mut_jobs.unlock();

            if(jobs.empty()) are_jobs = false;

            mut_are_jobs.unlock();

            try_add_if_prime(val);
        }
        else {
            mut_are_jobs.unlock();
            std::this_thread::sleep_for (std::chrono::nanoseconds(100));
        }

        mut_all_jobs_done.lock();
    }

    mut_all_jobs_done.unlock();
}


void try_add_if_prime(int val) {
    
    if (is_prime(val)) {
        update_sum_total(val);
        try_push(val);
    }
}


void try_push(int val) {

    mut_primes.lock();

    int low_idx = 0;
    for(int i = 0; i < 10; i++) {
        if (PRIMES[i] < PRIMES[low_idx]) {
            low_idx = i;
        }
    }

    if (PRIMES[low_idx] < val) PRIMES[low_idx] = val;

    mut_primes.unlock();
}


void update_sum_total(int val) {

    mut_sum.lock();
    SUM += val;
    mut_sum.unlock();

    mut_total.lock();
    TOTAL += 1;
    mut_total.unlock();
}


bool is_prime(int x) {

    if (x <= 3) {
        return true;
    }
    // else if (x % 2 == 0) {
    //     return false;
    // }

    int root = sqrt(x) + 1; 

    for(int i = 3; i < root; i += 2) {

        if (x % i == 0) {
            return false;
        }
    }

    return true;
}


void print(vector<int> arr) {

    for(auto v: arr) {
        cout << v << ", ";
    }
}

