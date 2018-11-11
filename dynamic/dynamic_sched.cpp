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

float output = 0,a,b;
float ba_by_n;
long start=0, end,n, intensity ;
int function_id, granularity,loop_done = 0;

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
	int granularity;
}; 

pthread_mutex_t mut; 

int fetch_beginning()
{
	int temp;
	pthread_mutex_lock(&mut);
	temp = start;
	start = start + granularity;
	
	if (start > n)
		loop_done = 1;
	pthread_mutex_unlock(&mut);
	return temp;
}

int fetch_ending(int start)
{	
	int end = (start + granularity);
	if( end >= n)
		return n;
	
	return end;
}

//iteration level mutex functionality	
void *iteration_level_thread_call(void *thread_info) {
	
	long n_start, n_end;
	threadData* info_data = (threadData *) thread_info;  
	
	while (loop_done!= 1)
	{
		n_start = fetch_beginning();
		if (n_start >= n)
			break;
		n_end = fetch_ending(n_start);
		
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
			
			if (i == n_end-1 && end>=n-1)
				loop_done = 1;
			
			pthread_mutex_unlock (&mut);
			
		}
		
	}		
	
	pthread_exit(NULL);
	
}



//thread level mutex functionality
void *thread_level_thread_call(void *thread_info) {
	long n_start, n_end;
	threadData* info_data = (threadData *) thread_info;  
		
	while(loop_done!=1)
	{
		n_start = fetch_beginning();
		if (n_start >= n)
			break;
		n_end = fetch_ending(n_start);

		for(int i = n_start; i < n_end; i++)
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
			
			pthread_mutex_lock(&mut);
			if (i == n_end-1 && end>=n-1)
					loop_done = 1;
				
			pthread_mutex_unlock(&mut);
		}
		
	}
	
	pthread_exit(NULL);
	
}

//chunk level mutex functionality
void *chunk_level_thread_call(void *thread_info) {
	threadData* info_data = (threadData *) thread_info;  
	
	//float local_result = 0;
	long n_start, n_end;
	
	while(loop_done != 1)
	{
		float local_result = 0;
		//pthread_mutex_lock(&mut);
		n_start = fetch_beginning();
		if (n_start >= n)
			break;
		n_end = fetch_ending(n_start);
		
		for(int i = n_start; i < n_end; i++)
		{
			float x=0, j = 0;
			j = (i+0.5)*ba_by_n;
			x = a+j;
			
			//pthread_mutex_lock(&mut);
			//std::cout<<"x is-- "<<local_x_val<<std::endl;
			
			switch(function_id)
			{
				case 1: local_result += f1(x, intensity);
						break;
				case 2: local_result += f2(x, intensity);
						break;
				case 3: local_result += f3(x, intensity);
						break;
				case 4: local_result += f4(x, intensity);
						break;
				default: break;
			}
			
			//pthread_mutex_unlock(&mut);

		}	
		
		
		pthread_mutex_lock(&mut);
		output +=  local_result;
		pthread_mutex_unlock(&mut);
		
		//std::cout<<"local_result -- "<<local_result <<"output is-" << output<<std::endl;
		
		if ( end>=n-1)
			loop_done = 1;
		
		//pthread_mutex_unlock(&mut);
		
		
	}
	
	/*pthread_mutex_lock(&mut);
	output +=  local_result;
	pthread_mutex_unlock(&mut);*/
	
	pthread_exit(NULL);
} 


int main (int argc, char* argv[]) {
	auto start = std::chrono::high_resolution_clock::now();
	
	if (argc < 9) {
		std::cerr<<"usage: "<<argv[0]<<" <functionid> <a> <b> <n> <intensity> <nbthreads> <sync> <granularity>"<<std::endl;
		return -1;
	}

	function_id = atoi(argv[1]);
	a = atof(argv[2]);
	b = atof(argv[3]);
	n = atoi(argv[4]);
	intensity = atoi(argv[5]);
	int nbthreads = atoi(argv[6]);
	std::string sync = argv[7];
	granularity = atoi(argv[8]);
	ba_by_n = (float)((b-a)/n);
	pthread_t threads[nbthreads];
	int rc;
	
	struct threadData *thread_data = NULL;
	
	if(sync.compare("iteration") == 0)
	{
		//call iteration_level_mutex function in pthread_create
		for( int i = 0; i < nbthreads; i++ ) {
			
			thread_data = (struct threadData*)malloc(sizeof *thread_data);
			
			thread_data->function_id = function_id;
			thread_data->a = a;
			thread_data->b = b;
			thread_data->n = n;
			thread_data->intensity = intensity;
			thread_data->granularity = granularity;
			thread_data->ba_by_n = ba_by_n;
			
			rc = pthread_create(&threads[i], NULL, iteration_level_thread_call, (void *) thread_data);
			
			if (rc) {
				std::cout << "Error:unable to create thread," << rc << std::endl;
			}
			
		}	
		for (int i=0; i < nbthreads; i++)
			pthread_join(threads[i], NULL);
		
	}
	else if(sync.compare("thread") == 0)
	{
		//call thread_level_mutex function in pthread_create
		
		pthread_mutex_init(&mut, NULL);
		
		thread_data = (struct threadData*)malloc(nbthreads * sizeof(struct threadData));
		
		for( int i = 0; i < nbthreads; i++ ) 
		{
			
			thread_data[i].function_id = function_id;
			thread_data[i].a = a;
			thread_data[i].b = b;
			thread_data[i].n = n;
			thread_data[i].intensity = intensity;
			thread_data[i].ba_by_n = ba_by_n;
			thread_data[i].granularity = granularity;
			
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
	else if(sync.compare("chunk") == 0)
	{
		//call chunk_level_mutex function in pthread_create
		for(int i = 0; i < nbthreads; i++)
 		{
			thread_data = (struct threadData*)malloc(sizeof *thread_data);
			
			thread_data->function_id = function_id;
			thread_data->a = a;
			thread_data->b = b;
			thread_data->n = n;
			thread_data->intensity = intensity;
			thread_data->granularity = granularity;
			thread_data->ba_by_n = ba_by_n;
			
 			rc = pthread_create(&threads[i], NULL, chunk_level_thread_call, (void *) thread_data);
			
			if (rc) {
				std::cout << "Error:unable to create thread," << rc << std::endl;
			}
 		}
 	
 	  	for(int i = 0; i < nbthreads; i++)
 		{
 			pthread_join(threads[i], NULL);
 		}
	}
	
	output = output*ba_by_n;
	std::cout<<output;
		
	auto stop = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double> exe_time = stop - start;

	std::cerr<<exe_time.count() ;
	
	return 0;
}