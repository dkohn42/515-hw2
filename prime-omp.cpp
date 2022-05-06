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
  int N, P;
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


  cout << "prime (seq) over [2.." << N << "] starting ...\n";

  gettimeofday(&start, NULL);
  bool candidate[N+1];
  for (int i = 2; i <= N; i++)
    candidate[i] = true;
    
  int sqrtN = sqrt(N);
  #pragma omp parallel num_threads(P) 
  #pragma omp single
  for (int i = 2; i <= sqrt(N); i++)
    if (candidate[i]) 
#pragma omp parallel for firstprivate(i)
      for (int j = i+i; j <= N; j += i)
        candidate[j] = false;

  int totalPrimes = 0;
  #pragma omp parallel for shared(totalPrimes)
  for (int i = 2; i <= N; i++)
    if (candidate[i]) {
      #pragma omp critical
      {
      totalPrimes++;
      }
    }
  
  gettimeofday(&end, NULL);
  double msec = (end.tv_sec - start.tv_sec) * 1000.0;                                              
  msec += (end.tv_usec - start.tv_usec) / 1000.0; 
  cout << "prime (seq) found " << totalPrimes << " primes\n";
  cout << "prime-omp (" << P << " threads) elapsed time:   "<< msec <<" ms" << endl;   
}
