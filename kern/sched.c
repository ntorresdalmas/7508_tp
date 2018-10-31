#include <inc/assert.h>
#include <inc/x86.h>
#include <kern/spinlock.h>
#include <kern/env.h>
#include <kern/pmap.h>
#include <kern/monitor.h>

void sched_halt(void);

// Choose a user environment to run and run it.
void
sched_yield(void)
{
	struct Env *idle;

	// Implement simple round-robin scheduling.
	//
	// Search through 'envs' for an ENV_RUNNABLE environment in
	// circular fashion starting just after the env this CPU was
	// last running.  Switch to the first such environment found.
	//
	// If no envs are runnable, but the environment previously
	// running on this CPU is still ENV_RUNNING, it's okay to
	// choose that environment.
	//
	// Never choose an environment that's currently running on
	// another CPU (env_status == ENV_RUNNING). If there are
	// no runnable environments, simply drop through to the code
	// below to halt the cpu.

	// LAB 4: Your code here.
	// TO DO: idea --> podriamos hacer una copia del struct envs y enlazar
	// envs[NENV-1] con envs[0] para que quede una lista circular
	// struct Env *envs_circular = envs;

	// Obtengo el curenv de la CPU actual
	idle = thiscpu->cpu_env;
	// Obtengo el index del curenv en el array envs
	int index_curenv = ENVX(idle->env_id);
	// Me muevo al proximo (si es el ultimo, paso al 0)
	bool last_env = index_curenv == NENV - 1;
	int index_nextenv = last_env ? 0 : index_curenv + 1;
	
	bool no_envs_runnable = true;
	// TO DO: ver cual es la condicion de corte
	while (1) {
		int i;
		// Recorro envs a partir del ultimo proceso que estaba corriendo + 1
		for (i=index_nextenv; i<NENV; i++) {
			if (envs[i].env_status == ENV_RUNNABLE) {
				// Si hay un proceso en espera, lo pongo a correr
				env_run(&envs[i]);
				// Actualizo el index (si es el ultimo, paso al 0)
				bool last_env = i == NENV - 1;
				index_nextenv = last_env ? 0 : i + 1;
				// Actualizo el booleano ya que aun quedan procesos por correr
				no_envs_runnable = false;
				break;
			}
		}
	}
	// Si recorri todos los procesos y no queda ninguno por correr,
	// pongo a correr el que ya estaba corriendo la CPU actual
	if (no_envs_runnable && thiscpu->cpu_env->env_status == ENV_RUNNING) {
		env_run(thiscpu->cpu_env);
	}			

	// TO DO: ver donde va esto
	// sched_halt never returns
	sched_halt();
}

// Halt this CPU when there is nothing to do. Wait until the
// timer interrupt wakes it up. This function never returns.
//
void
sched_halt(void)
{
	int i;

	// For debugging and testing purposes, if there are no runnable
	// environments in the system, then drop into the kernel monitor.
	for (i = 0; i < NENV; i++) {
		if ((envs[i].env_status == ENV_RUNNABLE ||
		     envs[i].env_status == ENV_RUNNING ||
		     envs[i].env_status == ENV_DYING))
			break;
	}
	if (i == NENV) {
		cprintf("No runnable environments in the system!\n");
		while (1)
			monitor(NULL);
	}

	// Mark that no environment is running on this CPU
	curenv = NULL;
	lcr3(PADDR(kern_pgdir));

	// Mark that this CPU is in the HALT state, so that when
	// timer interupts come in, we know we should re-acquire the
	// big kernel lock
	xchg(&thiscpu->cpu_status, CPU_HALTED);

	// Release the big kernel lock as if we were "leaving" the kernel
	unlock_kernel();

	// Reset stack pointer, enable interrupts and then halt.
	asm volatile("movl $0, %%ebp\n"
	             "movl %0, %%esp\n"
	             "pushl $0\n"
	             "pushl $0\n"
	             "sti\n"
	             "1:\n"
	             "hlt\n"
	             "jmp 1b\n"
	             :
	             : "a"(thiscpu->cpu_ts.ts_esp0));
}
