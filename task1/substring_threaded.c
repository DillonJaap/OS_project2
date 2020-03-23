#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define MAX 1024
#define MAX_THREADS 4

int total = 0;
int n1,n2;
char *s1,*s2;
FILE *fp;


int readf(FILE *fp)
{
	if ((fp=fopen("strings.txt", "r")) == NULL){
		printf("ERROR: can't open string.txt!\n");
		return 0;
	}

	s1 = (char*)malloc(sizeof(char) * MAX);
	if (s1 == NULL){
		printf("ERROR: Out of memory!\n");
		return -1;
	}

	s2 = (char*)malloc(sizeof(char) * MAX);
	if(s2 == NULL){
		printf("ERROR: Out of memory\n");
		return -1;
	}

	/*read s1 s2 from the file*/
	s1 = fgets(s1, MAX, fp);
	s2 = fgets(s2, MAX, fp);
	n1 = strlen(s1)-1; /*length of s1*/
	n2 = strlen(s2)-1; /*length of s2*/
	
	if (s1 == NULL || s2 == NULL || n1 < n2)  /*when error exit*/
		return -1;
}

void* num_substring(void* arg)
{
	long index = (long)arg;
//	printf("index: %2ld | %c%c\n", index, s1[index], s1[index+1]);

	int i;
	for(i = 0; i < n2; i++){  // search for the next string of size of n2
		if (*(s1 + index + i) != *(s2 + i))
			break;
	}

	if (i == n2)    
		total++;		// find a substring in this step
	
	pthread_exit(NULL);
}


int main(int argc, char* argv[])
{
	readf(fp);

	pthread_t threads[MAX_THREADS];
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	int rc = 0;
	int thread_num = 0;

	// create indices list
	long indices[MAX] = {0};
	int end = 0;

	for (int i = 0; i <= (n1 - n2); i++){   
		if (*(s1 + i) == *s2) 
			indices[end++] = i;
	}

	// loop and create threads for every index
	int i = 1;
	for (;;) {
		thread_num = MAX_THREADS - (i % MAX_THREADS) - 1;

		rc = pthread_create(&threads[thread_num], &attr,
			       	num_substring, (void*)indices[i-1]);  
		if (rc) {
			printf("Heh, nice try bro, but you couldn't CREATE a thread\n");
			return 1;
		}

		// if we used all the threads join them all
		if (i % MAX_THREADS == 0)
		{
			for (int j = 0; j < MAX_THREADS; j++) {
				pthread_join(threads[j], NULL);
			}
		}

		if (i >= end)
			break;
		i++;
	}

	// clean up remaining threads
	for (int j = 0; j < i % MAX_THREADS; j++)
		pthread_join(threads[j], NULL);

 	printf("The number of substrings is: %d\n", total);
	return 0;
}
