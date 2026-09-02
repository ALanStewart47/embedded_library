#include "rtos.h"

TASK_Parameter task_para[TASK_SUM];

void init_task(void)
{
	unsigned char i = 0;

	for(i = 0; i < TASK_SUM; i++)
	{
		task_para[i].state = TASK_FREE;
		
	}
}

void Create_task(TASK_ID task_id, task_fun func, unsigned char task_time)
{
	unsigned char i = 0;

	for(i = 0; i < TASK_SUM; i++)
	{
		if(task_para[i].state == TASK_FREE)
		{
			task_para[i].state = TASK_CREATE;
			task_para[i].task_id = task_id;
			task_para[i].delay_count.task_flag = 0;
			task_para[i].delay_count.task_ms = 0;
			task_para[i].task_SumDelay_time = task_time;
			task_para[i].func = func;
			break;
		}
	}
}

void Start_TargetTask(TASK_ID task_id)
{
	unsigned char i = 0;

	for(i = 0; i < TASK_SUM; i++)
	{
		if(task_para[i].task_id == task_id)
			task_para[i].state = TASK_START;
	}
}

void Stop_TargetTask(TASK_ID task_id)
{
	unsigned char i = 0;

	for(i = 0; i < TASK_SUM; i++)
	{
		if(task_para[i].task_id == task_id)
			task_para[i].state = TASK_STOP;
	}
}


void Start_AllTask(void)
{
	unsigned char i = 0;

	for(i = 0; i < TASK_SUM; i++)
	{
		if((task_para[i].state == TASK_CREATE)||(task_para[i].state == TASK_STOP))
		{
			task_para[i].state = TASK_START;
		}
	}
}

void Stop_AllTask(void)
{
	unsigned char i = 0;

	for(i = 0; i < TASK_SUM; i++)
	{
		if(task_para[i].state == TASK_START)
		{
			task_para[i].state = TASK_STOP;
		}
	}
}

void Delete_TargetTask(TASK_ID task_id)
{
	unsigned char i = 0;

	for(i = 0; i < TASK_SUM; i++)
	{
		if(task_para[i].task_id == task_id)
		{
			task_para[i].state = TASK_FREE;
			task_para[i].task_id = FREE_TASK_ID;
			task_para[i].delay_count.task_flag = 0;
			task_para[i].delay_count.task_ms = 0;
			task_para[i].task_SumDelay_time = 0;
		}
	}
}

void Task_time(void)
{
	unsigned char i = 0;

	for(i = 0; i < TASK_SUM; i++)
	{
		if(task_para[i].state == TASK_START)
		{
			task_para[i].delay_count.task_ms++;
			if(task_para[i].delay_count.task_ms > task_para[i].task_SumDelay_time)
			{
				task_para[i].delay_count.task_ms = 0;
				task_para[i].delay_count.task_flag = 1;
			}
		}
	}
}

 
void Task_Run(void)
{
	unsigned char i = 0;

	for(i = 0; i < TASK_SUM; i++)
	{
		if((task_para[i].state == TASK_START)&&(task_para[i].delay_count.task_flag == 1))
		{
			task_para[i].delay_count.task_flag = 0;
			task_para[i].func();
		}
	}
}

void systick_TaskHandler(void)
{

	Task_time();   
	
	Task_Run();   
}


