#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>

#define MAX 1024
#define Q_SIZE 6 

typedef struct queue
{
	char buf[Q_SIZE];
	int front;
	int rear;
} queue;

// create queue
queue q;
	
// mutext & condition variables
pthread_mutex_t mutex;
pthread_cond_t empty;
pthread_cond_t full;

// QUEUE FUNCTIONS
int is_empty(queue q)
{
	if (q.front == q.rear)
		return 1;
	return 0;
}

int is_full(queue q)
{ 
	if (q.rear == q.front - 1 || (q.front == 0 && q.rear == Q_SIZE - 1))
		return 1;
	return 0;
}

int enqueue(queue* q, char c)
{
	if (!is_full(*q))
	{
		q->buf[q->rear] = c;
		if (++q->rear > Q_SIZE - 1)
			q->rear = 0;
		return 1;
	}
	return 0;
}

int dequeue(queue* q, char* c)
{
	if (!is_empty(*q))
	{
		*c = q->buf[q->front];
		if (++q->front > Q_SIZE - 1)
			q->front = 0;
		return 1;
	}
	return 0;
}

// producer function
void* produce(void* arg)
{
	char* msg = (char*)arg;

	while (*msg++ != '\0')
	{
		pthread_mutex_lock(&mutex);
		while (is_full(q))
			pthread_cond_wait(&empty, &mutex);

		enqueue(&q, *msg);

		pthread_cond_signal(&full);
		pthread_mutex_unlock(&mutex);
	}
	pthread_exit(NULL);
}

// consumer function
void* consume(void* arg)
{
	char* msg = (char*)arg;
	char c;

	while (*msg++ != '\0')
	{
		pthread_mutex_lock(&mutex);

		while (is_empty(q))
			pthread_cond_wait(&full, &mutex);

		dequeue(&q, &c);
		printf("%c", c);

		pthread_cond_signal(&empty);
		pthread_mutex_unlock(&mutex);
	}
	pthread_exit(NULL);
}

// get message from file into a string
void get_msg(FILE* fp, char msg[])
{
	while((*msg++ = fgetc(fp)) != EOF);
	*msg = '\0';
}

int main(void)
{
	FILE* fp = fopen("message.txt", "r");
	if (!fp)
	{
		printf("Error file \"message.txt\" not found\n");
		return 1;
	}

	// set q attributes
	q.rear  = 0;
	q.front = 0;

	// get message
	char msg[MAX];
	get_msg(fp, msg);


	// create threads and attributes
	pthread_t producer;
	pthread_t consumer;
	pthread_attr_t attr;

	// Initialize mutex and condition variable objects
	pthread_mutex_init(&mutex, NULL);
	pthread_cond_init (&full, NULL);
	pthread_cond_init (&empty, NULL);

	// For portability, explicitly create threads in a joinable state
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	// start threads
	pthread_create(&consumer, &attr, consume, (void*)msg);
	pthread_create(&producer, &attr, produce, (void*)msg);

	// join threads
	pthread_join(consumer, NULL);
	pthread_join(producer, NULL);

	// clean up and exit
	pthread_attr_destroy(&attr);
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&empty);
	pthread_cond_destroy(&full);
	
	fclose(fp);
	return 0;
}
