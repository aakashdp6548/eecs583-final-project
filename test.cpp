#include <stdio.h>
#include <omp.h>

int get_num_threads(void) {
    int num_threads = 1;
    #pragma omp parallel
    {
        #pragma omp single
        num_threads = omp_get_num_threads();
    }
    return num_threads;
}

int main() 
{
  int num_devices = omp_get_num_devices();
  printf("Number of available devices %d\n", num_devices);

  #pragma omp target 
  {
      if (omp_is_initial_device()) {
        printf("Running on CPU\n");    
      } else {
        int nthreads= get_num_threads();
        printf("Running on GPU with %d threads in each team\n", nthreads);
      }
  }
  
}