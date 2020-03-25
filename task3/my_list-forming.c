/*
list-forming.c: 
Each thread generates a data node, attaches it to a global list. This is reapeated for K times.
There are num_threads threads. The value of "num_threads" is input by the student.  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sched.h>

int K = 200; // generate a data node for K times in each thread

struct Node
{
	int data;
	struct Node* next;
};

struct list
{
	struct Node* header;
	struct Node* tail;
};

pthread_mutex_t mutex_lock;

struct list *List;

void bind_thread_to_cpu(int cpuid) 
{
	cpu_set_t mask;
	CPU_ZERO(&mask);

	CPU_SET(cpuid, &mask);
	if (sched_setaffinity(0, sizeof(cpu_set_t), &mask))
       	{
		fprintf(stderr, "sched_setaffinity");
		exit(EXIT_FAILURE);
	}
}

struct Node* generate_data_node()
{
	struct Node *ptr;
	ptr = (struct Node*)malloc(sizeof(struct Node));    

	if (NULL != ptr)
		ptr->next = NULL;
	else 
		printf("Node allocation failed!\n");
	return ptr;
}

void* producer_thread(void *arg)
{
	int tid = *((int*)arg);
	bind_thread_to_cpu(tid); // bind this thread to a CPU

	struct Node* ptr, tmp;

	// create sublist
	struct list* SubList = (struct list*)malloc(sizeof(struct list));
	SubList->header = SubList->tail = NULL;

	// generate and attach K nodes to the global list
	for (int i = 0; i < K; i++)
	{
		ptr = generate_data_node();

		if (NULL != ptr)
		{
			// lock mutex

			ptr->data  = i; //generate data
			// attach the generated node to the global list
			if (SubList->header == NULL)
			{
				SubList->header = SubList->tail = ptr;
			}
			else
			{
				SubList->tail->next = ptr;
				SubList->tail = ptr;
			}                    
		}
	}

	// attach sublist to global list
	pthread_mutex_lock(&mutex_lock);
	if (List->header == NULL)
	{
		List->header = SubList->header;
		List->tail = SubList->tail;
	}
	else
	{
		List->tail->next = SubList->header;
		List->tail = SubList->tail;
	}
	pthread_mutex_unlock(&mutex_lock);
}


int main(int argc, char* argv[])
{
	int i, num_threads;

	int NUM_PROCS;//number of CPU
	int* cpu_array = NULL;

	struct Node  *tmp,*next;
	struct timeval starttime, endtime;

	if(argc == 1)
	{
		printf("ERROR: please provide an input arg (the number of threads)\n");
		exit(1);
	}

	num_threads = atoi(argv[1]); //read num_threads from user
	pthread_t producer[num_threads];

	K = atoi(argv[2]); // read K from user

	NUM_PROCS = sysconf(_SC_NPROCESSORS_CONF); //get number of CPU


	if (NUM_PROCS > 0)
	{
		cpu_array = (int*)malloc(NUM_PROCS*sizeof(int));
		if (cpu_array == NULL)
		{
			printf("Allocation failed!\n");
			exit(0);
		}
		else
		{
			for(i = 0; i < NUM_PROCS; i++)
				cpu_array[i] = i;
		}

	}

	pthread_mutex_init(&mutex_lock, NULL);

	List = (struct list*)malloc(sizeof(struct list));

	if( NULL == List )
	{
		printf("Allocation of list failed!\n");
		exit(0);	
	}
	List->header = List->tail = NULL;


	gettimeofday(&starttime, NULL); //get program start time

	for (i = 0; i < num_threads; i++)
	{
		pthread_create(&(producer[i]), NULL, (void*)producer_thread,
			       	&cpu_array[i % NUM_PROCS]);
	}

	for (i = 0; i < num_threads; i++)
	{
		if (producer[i] != 0)
			pthread_join(producer[i], NULL);
	}


	gettimeofday(&endtime, NULL); //get the finish time

	/* // print list to make sure it was created properly
	for (; List->header != NULL; List->header = List->header->next)
		printf("%d\n", List->header->data);
	*/

	// clean up / free memory
	if (List->header != NULL)
	{
		next = tmp = List->header;
		while(tmp != NULL)
		{  
			next = tmp->next;
			free(tmp);
			tmp = next;
		}            
	}
	if (cpu_array!= NULL)
		free(cpu_array);

	long time = (endtime.tv_sec-starttime.tv_sec) * 1000000+(endtime.tv_usec-starttime.tv_usec);
	// calculate program runtime
	printf("Total run time is %ld microseconds.\n", time);

	// print to file
	FILE* fp = fopen("my_list-out.txt", "a");
	fprintf(fp, "%d,%d,%ld\n", K, num_threads, time);
	fclose(fp);
	return 0; 
}
