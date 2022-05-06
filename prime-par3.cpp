//----------------------------------------------------------------------------- 
// Program code for CS 415P/515 Parallel Programming, Portland State University
//----------------------------------------------------------------------------- 

// A prime-finding program (Naive C++ version).
//
// Usage: 
//   linux> ./prime N P
//
#define _GNU_SOURCE

#include <iostream>
#include <cmath> 
#include <sys/time.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <sched.h>
using namespace std; 

mutex mtx;
mutex sieve_mtx;
condition_variable cvar_sieve; 
bool *candidate;
int *sieve;
int sieve_length;
atomic<int> totalPrimes = {0};
int sqrtN = 0;
long N, P;
int sieve_count = -1;

void worker(int k) {
  int primes_checked = 0;
  int start = sqrtN+1;

  unsigned int cpu_id;
  getcpu(&cpu_id, NULL);
  
  //  cout << "Worker[CPU:"<< cpu_id <<" Thread:"<< k <<"] on range ["<< start << "..."<< N+1 << ")" << endl;

  int prime = 0;
  int prime_candidate = 0;

  while (primes_checked != sieve_count) {
    unique_lock<mutex> sieve_lck(sieve_mtx);  
    prime = sieve[primes_checked];
    if (prime == 0) {
      cvar_sieve.wait(sieve_lck);
      sieve_lck.unlock();
    }
    //    prime = sieve[primes_checked];
    else if (prime > 1) {
      sieve[primes_checked] = -1;
      sieve_lck.unlock();
      prime_candidate = start;
      while (prime_candidate % prime != 0) 
	prime_candidate++;
      //      cout << "Worker[CPU:"<< cpu_id <<" Thread:"<< k <<"] on Prime="<< prime <<"  Range=["<< start << ".."<< N << ")" << endl;
      for (int j = prime_candidate; j <= N; j += prime)
	candidate[j] = false;
      primes_checked++;
    }
    else if (prime == -1) {
      sieve_lck.unlock();
      primes_checked++;
    }
  }
}

int main(int argc, char **argv) {
  struct timeval start, end;

  if (argc < 3) {
    cout << "Usage: ./prime N P\n";
    exit(0);
  }
  if ((N = atoi(argv[1])) < 2) {
    cout << "N must be greater than 1\n";
    exit(0);
  }
  if ((P = atoi(argv[2])) < 1) {
    cout << "P must be greater than 0\n";
    exit(0);
  }
 
  sqrtN = sqrt(N);
  cout << "prime (par1) over [2.." << N << "] starting ...\n";

  gettimeofday(&start, NULL);
  // Initialize candidate boolean array and temp_sieve for storing primes.
  candidate = new bool[N+1];
  for (int i = 2; i <= N+1; i++)
    candidate[i] = true;

  sieve = new int[sqrtN];
  for (int i = 0; i < sqrtN; i++)
    sieve[i] = 0;
 
  thread workers[P];
  cout << endl << "Starting "<< P <<" workers..." << endl;
  for (int k = 0; k < P; k++)
    workers[k] = thread(worker, k);

  int prime_count = 0;
  unsigned int cpu_id;
  getcpu(&cpu_id, NULL);
  cout << "Master[CPU:"<< cpu_id <<"] on range [2..."<< sqrtN << "]" << endl;

  for (int prime = 2; prime <= sqrtN; prime++) {
    if (candidate[prime]) {
      for (int j = prime; j <= sqrtN; j += prime) 
	candidate[j] = false;
      mtx.lock();
      sieve[prime_count] = prime;
      mtx.unlock();
      cvar_sieve.notify_all();
      prime_count++;
    }
    
  }
  sieve_count = prime_count;
  totalPrimes = prime_count;
  cvar_sieve.notify_all(); 
  
  for (int j = 0; j < P; j++)
     workers[j].join();
  
  for (int i = sqrtN+1; i <= N; i++) {
    if (candidate[i] == true) {
      totalPrimes++;
    }
  }


  gettimeofday(&end, NULL);
  double msec = (end.tv_sec - start.tv_sec) * 1000.0;                                              
  msec += (end.tv_usec - start.tv_usec) / 1000.0; 
  cout << endl;
  cout << "prime (par2) found " << totalPrimes << " primes\n";
  cout << "prime-par2 (" << P << " threads) elapsed time:   "<< msec <<" ms" << endl;   
}
