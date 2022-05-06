//----------------------------------------------------------------------------- 
// Program code for CS 415P/515 Parallel Programming, Portland State University
//----------------------------------------------------------------------------- 

// A prime-finding program (Sequential version).
//
// Usage: 
//   linux> ./prime N
//
#include <iostream>
#include <cmath> 
#include <sys/time.h>
#include <omp.h>
using namespace std; 

int main(int argc, char **argv) {
  int N,P;
  struct timeval start, end;
  bool sieve_finished = false;
  
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

  cout << "prime (omp2) over [2.." << N << "] starting ...\n";

  gettimeofday(&start, NULL);
  int sqrtN = (int)sqrt(N);

  bool candidate[N+1];
  for (int i = 2; i <= N+1; i++)
    {
      candidate[i] = true;
    }
  cout << endl;
    
  int sieve[sqrtN];
  for (int c = 0; c < sqrtN; c++) {
    sieve[c] = 0;
  }

  int prime_count = 0;
#pragma omp parallel num_threads(P) shared(sieve, prime_count)
  {
#pragma omp single
    {
      for (int i = 2; i <= sqrtN; i++)
	{
	  if (candidate[i])
	    {
	      for (int p = i+i; p <= sqrtN; p += i)
		candidate[p] = false;
	      sieve[prime_count] = i;
	      //	      cout << sieve[prime_count] << "  " << i << "  "<< prime_count << endl;
	      prime_count++; 
	    }
	}
    }
#pragma omp for
    for (int q = 0; q < prime_count; q++) {
      {
	int prime = sieve[q];
	int prime_candidate = sqrtN+1;
	while (prime_candidate % prime != 0)
	  prime_candidate++;
	//	cout << "Thread " << omp_get_thread_num() << ": Prime=" << prime <<"  Prime_candidate=" << prime_candidate << endl;
	for (int p = prime_candidate; p <= N; p += prime)
	  candidate[p] = false;
	
      }
    }
  }

  int totalPrimes = 0;
  for (int i = 2; i <= N; i++)
    if (candidate[i]) 
      totalPrimes++;
  
  gettimeofday(&end, NULL);
  double msec = (end.tv_sec - start.tv_sec) * 1000.0;                                              
  msec += (end.tv_usec - start.tv_usec) / 1000.0; 
  cout << "prime (omp2) found " << totalPrimes << " primes\n";
  cout << "Prime-omp2 elapsed time:   "<< msec <<" ms" << endl;   
}
