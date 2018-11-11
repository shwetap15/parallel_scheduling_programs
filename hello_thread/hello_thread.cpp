#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

struct threadData { 
	int thread_num; 
	int  total_threads; 
}; 


void *thread_call(void *thread_info) {
	int threadNum , totalThreads;
	threadData* print_data = (threadData *) thread_info;  
	
	threadNum = print_data->thread_num;
	totalThreads = print_data->total_threads;
	
	//std::cout << "I am thread "<< threadNum<<" in "<< totalThreads << "\r\n"<<std::endl;
	
	printf("I am thread %d in %d \r\n",threadNum, totalThreads);
	
	pthread_exit(NULL);
}

int main (int argc, char* argv[]) {

	if (argc < 2) {
		std::cerr<<"usage: "<<argv[0]<<" <nbthreads>"<<std::endl;
		return -1;
	}

	int nbthreads = atoi(argv[1]);
	pthread_t threads[nbthreads];
	int rc;
	int i;
	
	struct threadData *thread_data = NULL;
	
	for( i = 0; i < nbthreads; i++ ) {
		thread_data = (struct threadData*)malloc(sizeof *thread_data);
        
		//threadData *thread_data;
		
		thread_data->thread_num = i;
		thread_data->total_threads = nbthreads;
		
		rc = pthread_create(&threads[i], NULL, thread_call, (void *) thread_data);
		
		if (rc) {
		   std::cout << "Error:unable to create thread," << rc << std::endl;
		}
	}
	pthread_exit(NULL);

	return 0;
}
