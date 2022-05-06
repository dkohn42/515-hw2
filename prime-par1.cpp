//----------------------------------------------------------------------------- 
// Program code for CS 415P/515 Parallel Programming, Portland State University
//----------------------------------------------------------------------------- 

// A prime-finding program (Naive C++ version).
//
// Usage: 
//   linux> ./prime N P
//
#include <iostream>
#include <cmath> 
#include <sys/time.h>
#include <thread>
#include <mutex>
#include <atomic>
using namespace std; 

bool *candidate;
int *sieve;
int sieve_length;
atomic<int> totalPrimes = {0};
int sqrtN = 0;
int N, P;

void worker(int k) {                                                                             
  long split = (N - (int)sqrtN) / P;
  long start = k * split + sqrtN+1;
  long end = (k * split) + split + sqrtN;
  if (k == P-1)
    end = N;
  
  cout << "Worker["<< k <<"] on range ["<< start << "..."<< end << ")" << endl;
  for (int i = start; i <= end; i++)
    candidate[i] = true;

  for (int i = 0; i < sieve_length; i++) {
    int prime = sieve[i];
    //    cout << "Worker["<< k <<"] i:"<< i <<"  size:"<< sieve_length << "  checking prime: "<< prime << endl;
    int prime_candidate = start;
    while (prime_candidate % prime != 0)
      prime_candidate++;
    for (int j = prime_candidate; j <= end; j += prime)
      candidate[j] = false;
  }

  int prime_count = 0;
  for (int i = start; i <= end; i++) {
    if (candidate[i])
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
  // Initialize candidate boolean array and temp_sieve for storing primes.
  candidate = new bool[N+1];
  sieve = new int[sqrtN];
  //  cout << "Master: initializing candidate[2..."<< sqrtN <<"] and temp_sieve[0..."<< sqrtN-2 <<"]"<< endl;
  for (int i = 2; i <= sqrtN; i++) {
    candidate[i] = true;
    sieve[i-2] = 0;
  }
  
  cout << "Master: on range [2..."<< sqrtN << "]" << endl;
  for (int i = 2; i <= sqrtN; i++)
    if (candidate[i]) 
      for (int j = i+i; j <= sqrtN; j += i)
        candidate[j] = false;

  int prime_count = 0;
  //  cout << "Master: locating primes" << endl;
  for (int i = 2; i <= sqrtN; i++) {
    if (candidate[i] == true) {
      sieve[prime_count] = i;
      prime_count++;
    }
  }
  sieve_length = prime_count;
  totalPrimes = prime_count;
  
  //  cout << "Prime_count: "<< prime_count <<"  sieve:"<< sieve_length << endl;  
  //  cout << endl;
  
  thread workers[P];
  cout << "Starting "<< P <<" workers..." << endl;
  for (int g = 0; g < P; g++)
    workers[g] = thread(worker, g);
  for (int j = 0; j < P; j++)
    workers[j].join();
  
  gettimeofday(&end, NULL);
  double msec = (end.tv_sec - start.tv_sec) * 1000.0;                                              
  msec += (end.tv_usec - start.tv_usec) / 1000.0; 
  cout << endl;
  cout << "prime (par1) found " << totalPrimes << " primes\n";
  cout << "prime-par1 (" << P << " threads) elapsed time:   "<< msec <<" ms" << endl;   
}
