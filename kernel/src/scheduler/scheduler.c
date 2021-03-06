/*
 * scheduler.c
 *
 *  Created on: 26.02.2014
 *      Author: Michael
 */


#include <stdio.h>
#include <string.h>

#include "scheduler.h"
#include "../common/common.h"
#include "../timer/gptimer.h"
#include "../task/task.h"
#include "../task/taskTable.h"

extern void _schedule_asm( uint32_t* pc, uint32_t cpsr, uint32_t* userReg );

uint32_t getNextReady();
int32_t createTask( task_func entryPoint );

static uint32_t runningPID = 0;

int32_t
idleTask( void* args )
{
	volatile uint32_t counter = 0;

	uint32_t x1 = 42;
	uint32_t x2 = 43;
	uint32_t x3 = 44;
	uint32_t x4 = 45;

	while( 1 )
	{
		volatile uint32_t z = x1 + x2 + x3 + x4;
		counter += z;
	}
}

uint32_t
initScheduler() {
	reg32w(INTCPS_MIR_CLEAR1, 0, (1 << 6));
	reg32w(GPTIMER2_BASE, GPTIMER_TCRR, 0x00);
	reg32w(GPTIMER2_BASE, GPTIMER_TIER, GPTIMER_MATCH);
	reg32w(GPTIMER2_BASE, GPTIMER_TMAR, (1 << 31));
	reg32w(GPTIMER2_BASE, GPTIMER_TLDR, 0x00);
	reg32w(GPTIMER2_BASE, GPTIMER_TWER, 0x01);
	reg32w(GPTIMER2_BASE, GPTIMER_TISR, 0x03);
	reg32w(GPTIMER2_BASE, GPTIMER_TTGR, 0x00);
	reg32w(GPTIMER2_BASE, GPTIMER_TCLR, (1 << 6) | 0x03);

	createTask( idleTask );

	return 0;
}

void
schedule( uint32_t* userPC, uint32_t userCpsr, uint32_t* userRegs )
{
	// only one task in system, it must be running
	if(getNumOfTasks() == 1)
	{
		return;
	}

	//reg32w(GPTIMER2_BASE, GPTIMER_TCLR, reg32r(GPTIMER2_BASE, GPTIMER_TCLR) & ~0x01);

	Task* runningTask = getTask( runningPID );
	runningTask->pc = userPC;
	runningTask->cpsr = userCpsr;
	memcpy( runningTask->reg, userRegs, sizeof( runningTask->reg ) );

	uint32_t nextPID = getNextReady();

	scheduleTask( nextPID );

	// INFO: this point will never be reached

	//reg32w(GPTIMER2_BASE, GPTIMER_TTGR, 0x00);
	//reg32w(GPTIMER2_BASE, GPTIMER_TCLR, reg32r(GPTIMER2_BASE, GPTIMER_TCLR) | 0x01);
}

void
scheduleTask( uint32_t pid )
{
	Task* task = getTask( pid );
	task->state = RUNNING;

	_schedule_asm( task->pc, task->cpsr, task->reg );
}

uint32_t
getNextReady() {

	uint32_t i;
	uint32_t pid = runningPID % MAX_TASKS;

	for(i = 0; i < MAX_TASKS; i++) {
		if(getTask(pid)->state == READY) {
			return pid;
		} else {
			pid = (++pid) % MAX_TASKS;
		}
	}

	return MAX_TASKS;

}

int32_t
createTask( task_func entryPoint )
{
	Task newTask;
	newTask.state = READY;
	newTask.pid = getNextFreePID();
	newTask.pc = ( uint32_t* ) entryPoint;
	newTask.cpsr = 0x60000110; // user-mode and IRQs enabled

	// TODO: need a valid stack-pointer
	// TODO: check to what we need to set the LR (newTask.reg[ 14 ]
	void* stackPtr = ( void*) malloc( 1024 );
	memset( stackPtr, 'a', 1024 );

	newTask.reg[ 13 ] = ( uint32_t ) stackPtr;

	addTask( &newTask );

	return 0;
}

int32_t
fork()
{
	return 0;
}

int32_t
sleep( uint32_t millis )
{
	return 0;
}
