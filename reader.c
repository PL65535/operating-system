#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include<sys/wait.h>
#include <sys/shm.h>
#include<semaphore.h>
#include<fcntl.h>
#include"shmem.h"

int main()
{
	char buffer[BUFSIZ];//缓冲区
	int running=1;
	void* shared_memory=(void*)0;   
	struct shared_block* shared_stuff;
	int shmid;
	sem_t* empty;
	sem_t* full;
	sem_t* sem;
	pid_t  pid;
	
	sem=sem_open("WAIT",O_CREAT,0666,1);   //初始化为1
	empty=sem_open("EMPTY",O_CREAT,0666,5);//表示空的位置，初始化为5
	full=sem_open("FULL",O_CREAT,0666,0); //表示满的位置，初始化为0
	
	shmid=shmget((key_t)1234,sizeof(struct shared_block),0666 | IPC_CREAT);//分配共享内存
	if(shmid==-1)
	{
		exit(EXIT_FAILURE);
	}
	
	shared_memory=shmat(shmid,(void*)0,0);  //获得共享内存
	if(shared_memory==(void*)-1)
	{
		fprintf(stderr,"shmmat failed\n");
		exit(EXIT_FAILURE);
	}
	
	printf("menory attached at %X\n",(int)shared_memory);
	shared_stuff=(struct shared_block*)shared_memory;//获得共享内存指针
	
	shared_stuff->count=5;//缓冲区大小为5
	shared_stuff->in=0;
	shared_stuff->out=0;
	
	//sleep(3);
	pid=fork();
	if(pid==0)//子进程
	{
		int tag=0;
		while(running)
		{
			sleep(0.1);
			sem_wait(full);
			sem_wait(sem);
		
			printf("Son reader get lock\n");
			if(shared_stuff->in==shared_stuff->out)//缓冲区为空
			{
				printf("The buffer is empty\n");
				sem_post(sem);
				sleep(2);
			}
			else
			{
				int pos=shared_stuff->out;
				strcpy(buffer,shared_stuff->data[pos]);
				printf("Reader %d.MSG: %s",getpid(),shared_stuff->data[pos]);
				
				if (strncmp(buffer, "end", 3) == 0) //结束
				{
					if(tag==0)//第一次读到
					{
						sem_post(full);
						tag=1;
						running=0;
					}
					else
					{
						running=0;
					}
					sem_post(sem);
				}
				else
				{
					shared_stuff->out=(shared_stuff->out+1)%5; //出队+1
					sem_post(sem);
					sem_post(empty);
				}
			}
		}
	}
	else if(pid!=0)
	{
		int tag=0;
		while(running)
		{
			sleep(0.1);
			sem_wait(full);
			sem_wait(sem);
		
			printf("Parent reader get lock\n");
			if(shared_stuff->in==shared_stuff->out)//缓冲区为空
			{
				printf("The buffer is empty\n");
				sem_post(sem);
				sleep(2);
			}
			else
			{
				int pos=shared_stuff->out;
				strcpy(buffer,shared_stuff->data[pos]);
				printf("Reader %d. MSG: %s",getpid(),shared_stuff->data[pos]);
				
				if (strncmp(buffer, "end", 3) == 0) //结束
				{
					if(tag==0)
					{
						sem_post(full);
						tag=1;
					}
					else
					{
						running=0;
					}
					sem_post(sem);
				}
				else
				{
					shared_stuff->out=(shared_stuff->out+1)%5; //出队+1
					sem_post(sem);
					sem_post(empty);
				}
			}
		
		
		}
		waitpid(pid,NULL,0); //等待子进程结束
		if(shmdt(shared_memory)==-1)//分离共享内存
		{
			fprintf(stderr, "detach memory failed\n");
			exit(EXIT_FAILURE);
		}

		if(shmctl(shmid,IPC_RMID,0)==-1)//删除共享内存
		{
			printf("ERROR in remove shared_memory\n");
			fprintf(stderr, "shmctl(IPC_RMID) failed\n");
			exit(EXIT_FAILURE);
		}
	}
	
	exit(EXIT_SUCCESS);
	
}
