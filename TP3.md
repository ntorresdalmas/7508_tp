TP3: Multitarea con desalojo
========================

static_assert
---------

1. ¿Cómo y por qué funciona la macro static_assert que define JOS?
Para la evaluacion(comparacion) se le deben pasar consatntes (que no cambien a lo largo de la ejecucion del programa), por eso este assert hace la comparacion en tiempo de compilacion.


env_return
---------

1. al terminar un proceso su función umain() ¿dónde retoma la ejecución el kernel? Describir la secuencia de llamadas desde que termina umain() hasta que el kernel dispone del proceso.
TODO: no se si habla en general la funcion umain().

2. ¿en qué cambia la función env_destroy() en este TP, respecto al TP anterior?
Ahora env_destroy(e) primero detecta si el env a eliminar esta corriendo en otro CPU, en este caso le cambia el estado para que la proxima vez el Kernel lo detecte, lo libere. Sino lo destruye, se fija si esta el env actual corriendo y en este caso llama a sched_yield() para detectar el proximo env a ejecutar (usando round robin).


sys_yield
---------

1. TODO ni idea lo que hay que hacer.

2. 
" $ make qemu-nox
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
K> "
Como se llama 3 veces a ENV_CREATE(), tenemos los enviorments 1000, 1001 y 1002. Y notar que los va liberando ordenadamente.


envid2env
---------

1. en JOS, si un proceso llama a sys_env_destroy(0)
si el envid es cero, llama a env_destroy(curenv), es decir, libera el proceso que esta corriendo actualmente.

2. en Linux, si un proceso llama a kill(0, 9)
TO DO: deberia leer el manual kill?

3. JOS: sys_env_destroy(-1)
TO DO: estoy 93% seguro.
deberia indicar error, ya que los envid son todos positivos, excepto el 0(caso especial) que vimos en el punto anterior.
(si le paso a envid2env(-1, ...), la macro ENVX(-1)).

4. Linux: kill(-1, 9)
TO DO: deberia leer el manual kill?


dumbfork
---------

1. Si, antes de llamar a dumbfork(), el proceso se reserva a sí mismo una página con sys_page_alloc() ¿se propagará una copia al proceso hijo? ¿Por qué?

2. ¿Se preserva el estado de solo-lectura en las páginas copiadas? Mostrar, con código en espacio de usuario, cómo saber si una dirección de memoria es modificable por el proceso, o no. (Ayuda: usar las variables globales uvpd y/o uvpt.)

3. Describir el funcionamiento de la función duppage().

 Supongamos que se añade a duppage() un argumento booleano que indica si la página debe quedar como solo-lectura en el proceso hijo:
4. indicar qué llamada adicional se debería hacer si el booleano es true

5. describir un algoritmo alternativo que no aumente el número de llamadas al sistema, que debe quedar en 3 (1 × alloc, 1 × map, 1 × unmap).

6. ¿Por qué se usa ROUNDDOWN(&addr) para copiar el stack? ¿Qué es addr y por qué, si el stack crece hacia abajo, se usa ROUNDDOWN y no ROUNDUP?




---------
