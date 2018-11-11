#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>
#include <cmath>
#include <pthread.h>
#include <unistd.h>

#ifdef __cplusplus
extern "C" {
#endif

float f1(float x, int intensity);
float f2(float x, int intensity);
float f3(float x, int intensity);
float f4(float x, int intensity);

#ifdef __cplusplus
}
#endif

float output = 0;

struct threadData { 
	int function_id;
	float a;
	float b;
	long n;
	long n_start;
	long n_end;
	int intensity;
	float ba_by_n;
	float local_output;
	float x=0;
	float x_int;
}; 

pthread_mutex_t mut; 

//iteration level mutex functionality	
void *iteration_level_thread_call(void *thread_info) {
	int function_id , intensity;
	float ba_by_n, a, b;
	long n, n_start, n_end;
	threadData* info_data = (threadData *) thread_info;  
	
	function_id = info_data->function_id;
	a = info_data->a;
	b = info_data->b;
	n = info_data->n;
	n_start = info_data->n_start;
	n_end = info_data->n_end;
	intensity = info_data->intensity;
	ba_by_n = info_data->ba_by_n;
	
	for(long i = n_start;i<n_end;i++)
	{
		float x = 0;
		float j = 0;
		j = (i+0.5)*ba_by_n;
		x = a+j;
		
		pthread_mutex_lock (&mut);
		
		switch(function_id)
		{
			case 1 : {
					output += f1(x,intensity);
					break;
				}
			case 2 : {
					output += f2(x,intensity);
					break;
				}
			case 3 : {
					output += f3(x,intensity);
					break;
				}
			case 4 : {
					output += f4(x,intensity);
					break;
				}
			default:break;		
		}	
		
		pthread_mutex_unlock (&mut);
		
	}
		
	pthread_exit(NULL);
}


//thread level mutex functionality
void *thread_level_thread_call(void *thread_info) {
	int function_id , intensity;
	float ba_by_n, a, b;
	long n, n_start, n_end;
	
	struct threadData* info_data = (struct threadData* )thread_info;
	
	function_id = info_data->function_id;
	a = info_data->a;
	b = info_data->b;
	n = info_data->n;
	n_start = info_data->n_start;
	n_end = info_data->n_end;
	intensity = info_data->intensity;
	ba_by_n = info_data->ba_by_n;
	
	for(long i = n_start;i<n_end;i++)
	{
		float x = 0;
		float j = 0;
		j = (i+0.5)*ba_by_n;
		x = a+j;
		
		switch(function_id)
		{
			case 1 : {
					info_data->local_output += f1(x,intensity);
					break;
				}
			case 2 : {
					info_data->local_output += f2(x,intensity);
					break;
				}
			case 3 : {
					info_data->local_output += f3(x,intensity);
					break;
				}
			case 4 : {
					info_data->local_output += f4(x,intensity);
					break;
				}
			default:break;		
		}
	}
	
	pthread_exit(NULL);
}


int main (int argc, char* argv[]) {

	auto start = std::chrono::high_resolution_clock::now();
	
	if (argc < 8) {
		std::cerr<<"usage: "<<argv[0]<<" <functionid> <a> <b> <n> <intensity> <nbthreads> <sync>"<<std::endl;
		return -1;
	}

	int function_id = atoi(argv[1]);
	float a = atof(argv[2]);
	float b = atof(argv[3]);
	int n = atoi(argv[4]);
	int intensity = atoi(argv[5]);
	int nbthreads = atoi(argv[6]);
	std::string sync = argv[7];
	float ba_by_n = ((b-a)/n);
	
	pthread_t threads[nbthreads];
	int rc;
	
	struct threadData *thread_data = NULL;
	
	if(sync.compare("iteration") == 0)
	{
		//call iteration_level_mutex function in pthread_create
		for( int i = 0; i < nbthreads; i++ ) {
			
			thread_data = (struct threadData*)malloc(sizeof *thread_data);
			
			long n_start = (i*n)/nbthreads;
			long n_end = ((i+1)*n)/nbthreads;
			
			thread_data->function_id = function_id;
			thread_data->a = a;
			thread_data->b = b;
			thread_data->n = n;
			thread_data->n_start = n_start;
			thread_data->n_end = n_end;
			thread_data->intensity = intensity;
			thread_data->ba_by_n = ba_by_n;
			
			rc = pthread_create(&threads[i], NULL, iteration_level_thread_call, (void *) thread_data);
			
			if (rc) {
			   std::cout << "Error:unable to create thread," << rc << std::endl;
			}
			
		}	
		for (int i=0; i < nbthreads; i++)
			pthread_join(threads[i], NULL);
		
	}
	else
	{
		//call thread_level_mutex function in pthread_create
		
		pthread_mutex_init(&mut, NULL);
		
		thread_data = (struct threadData*)malloc(nbthreads * sizeof(struct threadData));
		
		for( int i = 0; i < nbthreads; i++ ) 
		{
			long n_start = (i*n)/nbthreads;
			long n_end = ((i+1)*n)/nbthreads;
			
			thread_data[i].function_id = function_id;
			thread_data[i].a = a;
			thread_data[i].b = b;
			thread_data[i].n = n;
			thread_data[i].n_start = n_start;
			thread_data[i].n_end = n_end;
			thread_data[i].intensity = intensity;
			thread_data[i].ba_by_n = ba_by_n;
			
			rc = pthread_create(&threads[i], NULL, thread_level_thread_call, (void *)&thread_data[i]);
			
			if (rc) {
			   std::cout << "Error:unable to create thread," << rc << std::endl;
			}
			
		}	
		for ( int i = 0; i < nbthreads; i++)
		{
			pthread_join(threads[i], NULL);
		}
		for( int k = 0; k < nbthreads; k++)
		{
			output += thread_data[k].local_output;
		}
		
	}
		
	
	output = output*ba_by_n;
	std::cout<<output;
		
	auto stop = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> exe_time = stop - start;

	std::cerr<<exe_time.count() ;
	
	return 0;
}