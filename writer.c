#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/shm.h>
#include<semaphore.h>
#include<fcntl.h>
#include"shmem.h"

int main()
{
	char buffer[BUFSIZ];
	int running=1;
	void* shared_memory=(void*)0;
	struct shared_block* shared_stuff;
	int shmid;
	sem_t* empty;
	sem_t* full;
	sem_t* sem;
	
	sem=sem_open("WAIT",O_CREAT,0666,1);
	empty=sem_open("EMPTY",O_CREAT,0666,5);
	full=sem_open("FULL",O_CREAT,0666,0);
	
	shmid=shmget((key_t)1234,sizeof(struct shared_block),0666 | IPC_CREAT);
	if(shmid==-1)
	{
		exit(EXIT_FAILURE);
	}
	
	shared_memory=shmat(shmid,(void*)0,0);
	if(shared_memory==(void*)-1)
	{
		fprintf(stderr,"shmmat failed\n");
		exit(EXIT_FAILURE);
	}
	
	printf("menory attached at %X\n",(int)shared_memory);
	shared_stuff=(struct shared_block*)shared_memory;
	
	
	
	while(running)
	{
		sleep(0.2);
		sem_wait(empty);
		sem_wait(sem);
		
		printf("writer get lock\n");
		if((shared_stuff->in+1)%5==shared_stuff->out)//缓冲区满了
		{
			fgets(buffer,sizeof(buffer),stdin);
			printf("Sorry,buffer is full,you cannot write\n");
			sem_post(sem);
			sleep(2);
		}
		else
		{
			int inqueue=shared_stuff->in;
			fgets(buffer,sizeof(buffer),stdin); //等待输入
			
			strncpy(shared_stuff->data[inqueue],buffer,TEXT_SZ);
			shared_stuff->in=(shared_stuff->in+1)%5;			//入队+1
			
			printf("This is writer process %d,input pos is %d\n",getpid(),inqueue);
			
			if (strncmp(buffer, "end", 3) == 0) //结束
			{
                running = 0;
			}
			sem_post(sem);
			sem_post(full);
		}
		
	}
	
	if(shmdt(shared_memory)==-1)
	{
		fprintf(stderr, "detach memory failed\n");
		exit(EXIT_FAILURE);
	}

    exit(EXIT_SUCCESS);
}