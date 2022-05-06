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
atomic<int> totalPrimes = {0};
int sqrtN = 0;
unsigned int N;
int P;
int sieve_size = -1;

void worker(int k) {
  int primes_checked = 0;
  int split = (N - sqrtN) / P;
  int start = k * split + sqrtN+1;
  int end = (k * split) + split + sqrtN;
  if (k == P-1)
    end = N;

  int prime = 0;
  int prime_candidate = 0;
  
  unsigned int cpu_id;
  getcpu(&cpu_id, NULL);
  
  //  cout << "Worker[CPU:"<< cpu_id <<" Thread:"<< k <<"] on range ["<< start << "..."<< end << ")" << endl;
  for (int i = start; i <= end; i++)
    candidate[i] = true;

  while (primes_checked != sieve_size) {
    unique_lock<mutex> sieve_lck(sieve_mtx);  
    if (sieve[primes_checked] == 0) {
      cvar_sieve.wait(sieve_lck);
      //      sieve_lck.unlock();
    }
    else {
      prime = sieve[primes_checked];
      sieve_lck.unlock();
      prime_candidate = start;
      while (prime_candidate % prime != 0)
	prime_candidate++;
      for (int j = prime_candidate; j <= end; j += prime)
	candidate[j] = false;
      primes_checked++;
    }
    //    cout << "Thread: " << k << "  Primes Checked: " << primes_checked << endl;
    
  }
  
  int prime_count = 0;
  for (int q = start; q <= end; q++) {
    if (candidate[q])
      prime_count++;
  }
  totalPrimes += prime_count;
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

  candidate = new bool[N+1];
  sieve = new int[sqrtN+1];

  thread workers[P];
  cout << endl << "Starting "<< P <<" workers..." << endl;
  for (int k = 0; k < P; k++)
    workers[k] = thread(worker, k);

  for (int i = 0; i <= sqrtN; i++) {
    candidate[i] = true;
    sieve[i] = 0;
  }
  
  int prime_count = 0;
  unsigned int cpu_id;
  getcpu(&cpu_id, NULL);
  cout << "Master[CPU:"<< cpu_id <<"] on range [2..."<< sqrtN << "]" << endl;
  for (int prime = 2; prime <= sqrtN; prime++) {
    if (candidate[prime]) {
      for (int g = prime+prime; g <=sqrtN; g += prime)
	candidate[g] = false;
      sieve[prime_count] = prime;
      cvar_sieve.notify_all();
      prime_count++;
    }
  }
  sieve_size = prime_count;
  cvar_sieve.notify_all(); 
  
  totalPrimes += prime_count;
  
  for (int j = 0; j < P; j++)
    workers[j].join();
  
  gettimeofday(&end, NULL);
  double msec = (end.tv_sec - start.tv_sec) * 1000.0;                                              
  msec += (end.tv_usec - start.tv_usec) / 1000.0; 
  cout << endl;
  cout << "prime (par2) found " << totalPrimes << " primes\n";
  cout << "prime-par2 (" << P << " threads) elapsed time:   "<< msec <<" ms" << endl;   }
