

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <pthread.h>
#include <fstream>

typedef struct thread_info{
 int circle;//count of points inside the circle
 double *inside_circle;
 double *inside_square;
}thread_info;


using namespace std;

long N;
int K, thread_number=0;
int circle_count=0;//to keep track of total no. of points inside the circle
 
thread_info *threads;

double circle(double x, double y){
    double result=(x * x) + (y * y) - 1;
    return result; 
} 

void *runner(void *param);

int main() {

  fstream input;
  input.open("inp.txt", ios::in);

  if (!input) {
    cout << "File couldn't be opened\n";
  }
  else{
    input >> N >> K;
  }
  // K no. of threads
  // N no. of random numbers
  input.close();
  
  threads=(thread_info *)malloc(sizeof(thread_info)*K);
  
  double time_spent = 0.0;

  clock_t begin = clock();

  pthread_t thread[K];
  srand(time(0));
  for (int i = 0; i < K; i++) {
    pthread_create(&thread[i], NULL, runner, NULL);
  }

  for (int i = 0; i < K; i++) {
    pthread_join(thread[i], NULL);
  }
  
  double pi=4*(double)(circle_count)/(double)(N);
   
   clock_t end = clock();
  time_spent += (double)(end - begin) / CLOCKS_PER_SEC;
  //calculating time spent in microseconds
  time_spent=time_spent*1000000;
  fstream output;
  output.open("output.txt",ios::out);
  if(!output){
    cout << "File creation failed in Main\n";
  }
  // create a final output file from thread outputs
    output << "Time: " << time_spent << " microseconds\n";
    output << "Value Computed: " << pi << "\n" <<"Log:\n";
    
    //writing log into output file
    for(int i=0; i<K; i++){
     output << "Thread" <<i+1<<":"<<N/K<<", "<<threads[i].circle<<"\n";
     output <<"Points inside the square: ";
     for(int j=0; j<N/K; j++){
     output << "("<<threads[i].inside_square[2*j]<<", "<<threads[i].inside_square[2*j+1]<<"),";
     }
     output<<"\n";
     output <<"Points inside the circle: ";
     for(int j=0; j<threads[i].circle; j++){
     output << "("<<threads[i].inside_circle[2*j]<<", "<<threads[i].inside_circle[2*j+1]<<"),";
     }
     output<<"\n";
    }
  output.close();
  
  for(int i=0; i<K; i++){
   free(threads[i].inside_square);
   free(threads[i].inside_circle);
  }
  free(threads);
  
  return 0;
}

void *runner(void *param) {
  int t=thread_number+1;
  thread_number++;

  int count_circle = 0;//keeps track of no.of points inside the circle for this thread
  
  threads[t-1].inside_square=(double *)malloc(sizeof(double)*(2*N/K));
  threads[t-1].inside_circle=(double *)malloc(sizeof(double)*(2*N/K));
  

  for (long i = 0; i < N / K; i++) {
    double x =((double)(rand())/(double)RAND_MAX)*(2)-1;
    double y = ((double)(rand())/(double)RAND_MAX)*(2)-1; 
    //above x and y are points inside the square
     
      threads[t-1].inside_square[2*i]=x;
      threads[t-1].inside_square[2*i+1]=y;
      
      if (circle(x, y) < 0) {
      threads[t-1].inside_circle[2*count_circle]=x;
      threads[t-1].inside_circle[2*count_circle+1]=y;
      count_circle++;
    }
  }
  threads[t-1].circle=count_circle;
  circle_count += count_circle;
  
  pthread_exit(0);
}
