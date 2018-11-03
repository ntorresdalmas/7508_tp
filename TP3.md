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