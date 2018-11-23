:octocat: TP3: Multitarea con desalojo :octocat:
========================
 
-------------
:clubs: static_assert
 
1. ¿Cómo y por qué funciona la macro static_assert que define JOS?

Para la evaluación (comparación) se le deben pasar constantes (que no cambien a lo largo de la ejecución del programa). Por ello, este assert hace la comparación en tiempo de compilación.
Esto es así ya que esta definida la macro con un switch(x) case 0: case(x)
Si x es 0 (False), siempre cae en case 0 y produce un error en tiempo de compilación.
 
 
----------
:clubs: env_return
 
1. Al terminar un proceso su función umain() ¿dónde retoma la ejecución el kernel? Describir la secuencia de llamadas desde que termina umain() hasta que el kernel dispone del proceso.

:construction:
 
2. ¿En qué cambia la función env_destroy() en este TP, respecto al TP anterior?
La nueva versión de env_destroy(e) primero detecta si el env a eliminar está corriendo en otro CPU, en cuyo caso le cambia el estado para que la próxima vez el Kernel lo detecte y lo libere.
Caso contrario lo destruye, se fija si está el env actual corriendo y llama a sched_yield() para detectar el próximo env a ejecutar (mediante Round Robin).

 
---------
:clubs: sys_yield
 
2. Leer y estudiar el código del programa user/yield.c. Cambiar la función i386_init() para lanzar tres instancias de dicho programa, y mostrar y explicar la salida de make qemu-nox.

"" 
$ make qemu-nox
+ cc kern/init.c
+ ld obj/kern/kernel
+ mk obj/kern/kernel.img
***
*** Use Ctrl-a x to exit qemu
***
qemu-system-i386 -nographic -drive file=obj/kern/kernel.img,index=0,media=disk,format=raw -serial mon:stdio -gdb tcp:127.0.0.1:26000 -D qemu.log -smp 1  -d guest_errors
6828 decimal is 15254 octal!
Physical memory: 131072K available, base = 640K, extended = 130432K
check_page_free_list() succeeded!
check_page_alloc() succeeded!
check_page() succeeded!
check_kern_pgdir() succeeded!
check_page_free_list() succeeded!
check_page_installed_pgdir() succeeded!
SMP: CPU 0 found 1 CPU(s)
enabled interrupts: 1 2
[00000000] new env 00001000
[00000000] new env 00001001
[00000000] new env 00001002
hello, world
i am environment 00001000
[00001000] exiting gracefully
[00001000] free env 00001000
hello, world
i am environment 00001001
[00001001] exiting gracefully
[00001001] free env 00001001
hello, world
i am environment 00001002
[00001002] exiting gracefully
[00001002] free env 00001002
No runnable environments in the system!
Welcome to the JOS kernel monitor!
Type 'help' for a list of commands.
K>
""

Considerando que se llama 3 veces a ENV_CREATE(), tenemos los environments 1000, 1001 y 1002.
Notar que los va liberando de manera ordenada.
 
 
---------
:clubs: envid2env
 
1. En JOS, si un proceso llama a sys_env_destroy(0)

Si el envid es cero, llama a env_destroy(curenv). Es decir, libera el proceso que está corriendo actualmente.
 
2. En Linux, si un proceso llama a kill(0, 9)

Si el pid es cero, envía la señal (9) a todo proceso dentro del grupo de procesos que se encuentra el actual. La señal 9 indica claramente que debe quitarse.
 
3. JOS: sys_env_destroy(-1)

Indica error, ya que los envid son todos positivos, excepto el 0 (caso especial) que se mencionó en el punto anterior.
De hecho, si se hace envid2env(-1, ...), la macro ENVX(-1) indica error con parámetros negativos.
 
4. Linux: kill(-1, 9)

Si el pid es -1, envia la señal (9) a todo proceso tal que el actual tenga permiso de enviarle señales.


--------
:clubs: dumbfork

1. Si, antes de llamar a dumbfork(), el proceso se reserva a sí mismo una página con sys_page_alloc() ¿se propagará una copia al proceso hijo? ¿Por qué?

Sí, ya que la nueva página reservada es parte del address space del padre, por lo cual se copiará
al hijo como cualquier otra dirección que esté mapeada.

2. ¿Se preserva el estado de solo-lectura en las páginas copiadas? Mostrar, con código en espacio de usuario, cómo saber si una dirección de memoria es modificable por el proceso, o no. (Ayuda: usar las variables globales uvpd y/o uvpt.)

No, no se preserva el estado de solo-lectura en las páginas copiadas, ya que la función duppage
no recibe permisos como parámetros, sino que le pasa siempre tres flags fijos tanto a sys_page_alloc como a sys_page_map. En particular, estos flags son PTE_P | PTE_U | PTE_W. Éste último es, justamente, el que marca como writeable a todas las páginas copiadas.

Código en user-space para saber si una dirección de memoria es modificable por el proceso, o no:

for (va=0; va<UTOP; va+=PGSIZE){
		// Obtengo la direccion del page directory entry
		pde_t actual_pde = uvpd[PDX(va)];
		// Si tiene el bit de presencia --> hay una pagina mapeada
		is_maped = (actual_pde == (actual_pde | PTE_P));

		if (is_maped) {
			// Obtengo la direccion del page table entry
			pte_t actual_pte = uvpt[PGNUM(va)];
			// Si tiene el bit de escritura --> es modificable
			is_writeable = (actual_pte == (actual_pte | PTE_W));
		}
}

3. Describir el funcionamiento de la función duppage().

Primero, aloca una página en la dirección recibida con los permisos PTE_U | PTE_P | PTE_W
Segundo, comparte la página alocada con una dirección temporal (UTEMP)
Tercero, mueve el contenido de la dirección temporal a la dirección recibida
Cuarto, libera la dirección temporal alocada.


4. Supongamos que se añade a duppage() un argumento booleano que indica si la página debe quedar como solo-lectura en el proceso hijo. Indicar qué llamada adicional se debería hacer si el booleano es true.

Supongamos que la firma de duppage ahora es:

duppage(envid_t dstenv, void *addr, bool read_only);

En este caso, bastaría con chequear el parámetro booleano para saber si los permisos deben modificarse o no. Por ejemplo:

duppage(envid_t dstenv, void *addr, bool read_only) {
	int perm;
	if (read_only) {
		perm = PTE_P | PTE_U;
	} else {
		perm = PTE_P | PTE_U | PTE_W;
	}
}

De esta forma, se puede ver que, si el booleano es true, la página se copiará sin permisos de escritura (read-only).

5. Describir un algoritmo alternativo que no aumente el número de llamadas al sistema, que debe quedar en 3 (1 × alloc, 1 × map, 1 × unmap).

El algoritmo descripto en el punto 4 cumple con este requisito.

6. ¿Por qué se usa ROUNDDOWN(&addr) para copiar el stack? ¿Qué es addr y por qué, si el stack crece hacia abajo, se usa ROUNDDOWN y no ROUNDUP?

Se utiliza &addr porque es una variable local y, por lo tanto, vive en el stack.
Por otro lado, se utiliza ROUNDDOWN porque justamente nos interesa mapear el principio de la página.


--------------
:clubs: multicore_init

1. ¿Qué código copia, y a dónde, la siguiente línea de la función boot_aps()?
memmove(code, mpentry_start, mpentry_end - mpentry_start);
Copia la direccion virtual correspondiente a la fisica en donde empiezan los non-boot CPUs (APs) en mpentry_start(variable global).

2. ¿Para qué se usa la variable global mpentry_kstack? ¿Qué ocurriría si el espacio para este stack se reservara en el archivo kern/mpentry.S, de manera similar a bootstack en el archivo kern/entry.S?
TODO: leer parte A del lab4 del MIT


3. Cuando QEMU corre con múltiples CPUs, éstas se muestran en GDB como hilos de ejecución separados. Mostrar una sesión de GDB en la que se muestre cómo va cambiando el valor de la variable global mpentry_kstack


4. En el archivo kern/mpentry.S se puede leer:
"# We cannot use kern_pgdir yet because we are still
# running at a low EIP.
movl $(RELOC(entry_pgdir)), %eax"
	¿Qué valor tiene el registro %eip cuando se ejecuta esa línea?

	Responder con redondeo a 12 bits, justificando desde qué región de memoria se está ejecutando este código.

	¿Se detiene en algún momento la ejecución si se pone un breakpoint en mpentry_start? ¿Por qué?	


5. Con GDB, mostrar el valor exacto de %eip y mpentry_kstack cuando se ejecuta la instrucción anterior en el último AP.


-------
:clubs: ipc_rev

1. Un proceso podría intentar enviar el valor númerico -E_INVAL vía ipc_send(). ¿Cómo es posible distinguir si es un error, o no? En estos casos:

    // Versión A
    envid_t src = -1;
    int r = ipc_recv(&src, 0, NULL);
    if (r < 0)
      if (/* ??? */)
        puts("Hubo error.");
      else
        puts("Valor negativo correcto.")

    // Versión B
    int r = ipc_recv(NULL, 0, NULL);
    if (r < 0)
      if (/* ??? */)
        puts("Hubo error.");
      else
        puts("Valor negativo correcto.")

Para la versión A puedo detectar si se trata de un error o no si en la variable &src (en nuestra función es from_env_store) quedó almacenado un 0 o el envid del emisor. Es decir, la condición podría ser:

if (from_env_store == 0) {
	puts("Hubo error.");
} else {
	puts("Valor negativo correcto.");
}

En cambio, para la versión B no hay forma de detectar si se trata de un valor o un código de error, ya que la función ipc_recv recibe NULL como primer parámetro. Ergo, no hay estructura en donde almacenar lo explicado para la versión anterior.


----------------
:clubs: sys_ipc_try_send

1. ¿Cómo se podría hacer bloqueante esta llamada? Esto es: qué estrategia de implementación se podría usar para que, si un proceso A intenta enviar a B, pero B no está esperando un mensaje, el proceso A sea puesto en estado ENV_NOT_RUNNABLE, y sea despertado una vez B llame a ipc_recv().

Podría modificarse la condición que comprueba si el proceso destino está esperando o no un mensaje:

if (!e->env_ipc_recving) {
	curenv->env_status = ENV_NOT_RUNNABLE;
	sys_yield();
}