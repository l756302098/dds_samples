//writeShareMemory.cpp
#include "shmDatadef.hpp"

int main(int argc, char* argv[])
{
	void *shm = NULL;
	struct stuShareMemory *stu = NULL;
	int shmid = shmget((key_t)1234, sizeof(struct stuShareMemory), 0666|IPC_CREAT);
	if(shmid == -1)
	{
		printf("shmget err.\n");
		return 0;
	}

	shm = shmat(shmid, (void*)0, 0);
	if(shm == (void*)-1)
	{
		printf("shmat err.\n");
		return 0;
	}

	stu = (struct stuShareMemory*)shm;

	stu->iSignal = 0;

	//while(true) //如果需要多次 读取 可以启用 while
	{
		if(stu->iSignal != 1)
		{
			printf("write txt to shm.");
			memcpy(stu->chBuffer, "hello world 666 - 888.\n", 30);
			stu->iSignal = 1;
		}
		else
		{
			sleep(10);
		}
	}
	
	shmdt(shm);

	std::cout << "end progress." << endl;
	return 0;
}