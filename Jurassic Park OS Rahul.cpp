
#include <iostream>
#include <queue>
#include <cstdlib>
#include <fstream>
#include <thread>
#include <chrono>
#include <ctime>
#include <random>
#include <semaphore.h>

// Library effective with Windows
//#include <windows.h>
 
// Library effective with Linux
#include <unistd.h>

using namespace std;
using namespace std::chrono;

int P,C,lambda_P,lambda_C,k;

int *P_state, *C_state; //stores the state of passengers and cars

sem_t smphC;
sem_t smphP;
sem_t mutex; //to lock the shared data before using it

int p_left; //no. of rides
queue<int> p_thread_id; //for storing thread ids of waiting passenger threads..a waiting queue

void p_ride(int id){

	fstream output;
  	
  	output.open("output.txt",ios::app);
  	if(!output){
   		 cout << "File creation failed \n";
 	 }
	sem_wait(&mutex);
	P_state[id]=0; //in the museum
	sem_post(&mutex);
	
 	 auto givemetime_entry = chrono::system_clock::to_time_t(chrono::system_clock::now());
 	 output << "Passenger " << id+1 << " enters the museum at " << ctime(&givemetime_entry);
 	 

	//wait time for passenger t_p
	default_random_engine generator;
    
    	exponential_distribution<double> d1((1.0/lambda_P));
    	
	double t_p = d1(generator)*1e3;
	
 	 for (int i=0; i<k; i++){
 	 
 	 	auto givemetime_request = chrono::system_clock::to_time_t(chrono::system_clock::now());
 	 	output << "Passenger " << id+1 << " made a ride request at " << ctime(&givemetime_request);
 	 	
 	 	sem_wait(&mutex); //acquire the mutex before accessing the shared data
 	 	sem_post(&smphP);
 	 	p_thread_id.push(id);
 	 	sem_post(&mutex); //release the mutex
 	 	
 	 	//wait for the car to accept the ride
 	 	sem_wait(&mutex);

 	 	while(true){
	 	 	int j;
	 	 	for(j=0; j<C; j++){
	 	 		//checking if any car is available
		 	 	if(C_state[j]==0){
		 	 		C_state[j]=1;
		 	 		P_state[id]=2;
		 	 		sem_post(&mutex);
		 	 		break;
		 	 	}
	 	 	}
	 	 	if(P_state[id]==2){
	 	 		auto givemetime_riding = chrono::system_clock::to_time_t(chrono::system_clock::now());
	 	 		output << "Passenger " << id+1 << " started riding at " << ctime(&givemetime_riding);
	 	 		
	 	 		sem_post(&mutex);
	 	 		break;
	 	 	}
	 	 	
 	 	}
 	 	
	 	sem_wait(&smphC); //because passenger found an available car and is using it
	 	
	 	 	sem_wait(&mutex);
	 	 	p_left--; //passenger completed a ride
	 	 	sem_post(&mutex);
	 	 	
 	 	auto givemetime_ridecomplete = chrono::system_clock::to_time_t(chrono::system_clock::now());
 	 	output << "Passenger " << id+1 << " finished riding at " << ctime(&givemetime_ridecomplete);
		
		sem_wait(&mutex);
	 	P_state[id]=0; //passenger is back for more rides
	 	sem_post(&mutex);
	 	
	 	this_thread::sleep_for(chrono::duration<double>(t_p)); //wait before making another request
	 	
 	 }
 	 
	output.close();
	return;
}

void c_ride(int id){
	 
	fstream output;
  	
  	output.open("output.txt",ios::app);
  	if(!output){
   		 cout << "File creation failed \n";
 	 }
 	 
	 sem_wait(&mutex);
	 C_state[id]=0; //car is available
	 sem_post(&mutex);
 
	//wait time for car t_c 
	default_random_engine generator;
 
    	exponential_distribution<double> d2((1.0/lambda_C));

	double t_c = d2(generator)*1e3;
	
 	 while(p_left!=0){
 	
 	 	sem_wait(&smphP);
 	 	
 		sem_wait(&mutex);
 		
 	 	int pid = p_thread_id.front();
 	 	p_thread_id.pop();
 	 	
 	 	output << "Car " << id+1 << " accepts Passenger " << pid+1 << "'s request" << endl;
 	 	
 	 	C_state[id]=2;
 	 	P_state[pid]=2;
 	 	output << "Car " << id+1 << " is ridding Passenger " << pid+1 << endl; 
 	 	
 	 	sem_post(&smphC);
 	 	sem_post(&mutex);
 	 	output << "Car " << id+1 << " finished Passenger " << pid+1 << "'s tour" << endl;
 	 	
 	 	sem_wait(&mutex);
 	 	C_state[id]=0; //car is available again
 	 	sem_post(&mutex);
 		
 		this_thread::sleep_for(chrono::duration<double>(t_c));
 	 	 //wait before accepting another ride
 	 }
	
	output.close();
	return;
}

int main(){
	fstream input;
        input.open("inp-params.txt", ios::in);

  	if (!input) {
    		cout << "File couldn't be opened\n";
 	 }
  	else{
  		//take input from input file n, k, λ1, λ2.
    		input >> P >> C >> lambda_P >> lambda_C >> k;
 	 }

	input.close();

	P_state=(int *)malloc(sizeof(int)*P);
	C_state=(int *)malloc(sizeof(int)*C);
	
	sem_init(&smphC,0,C); 
	sem_init(&smphP,0,0);
	sem_init(&mutex,0,1);
	
	p_left=k*P;
	
	vector<thread> my_threads;
	
	double complete_time_p=0;
	double complete_time_c=0;
	
	for (int i=0; i<P ; i++){
		auto start = high_resolution_clock::now();
		thread thd(p_ride,i);
		auto stop = high_resolution_clock::now();
		auto duration = duration_cast<microseconds>(stop - start);
		complete_time_p=complete_time_p+duration.count();
		my_threads.push_back(move(thd));
	}
	
	for(int i=0; i<C; i++){
		auto start = high_resolution_clock::now();
		thread thd(c_ride,i);
		auto stop = high_resolution_clock::now();
		auto duration = duration_cast<microseconds>(stop - start);
		complete_time_c=complete_time_c+duration.count();
		my_threads.push_back(move(thd));
	}
	
	for(thread & th : my_threads){
		if(th.joinable()) th.join();
	}
	
	free(P_state);
	free(C_state);
	cout << "Average time for passenger threads to complete their tour = "<< complete_time_p/P << "microseconds\n";
	cout << "Average time for cars to complete their tour = "<< complete_time_c/C << "microseconds\n";
return 0;
}
