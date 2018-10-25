TP2: Procesos de usuario
========================

env_alloc
---------
1. ¿Qué identificadores se asignan a los primeros 5 procesos creados? (Usar base hexadecimal.)
codigo: 
" generation = (e->env_id + (1 << ENVGENSHIFT)) & ~(NENV - 1);
	if (generation <= 0)  // Don't create a negative env_id.
		generation = 1 << ENVGENSHIFT;
	e->env_id = generation | (e - envs); "

En la 1era linea, generation queda []
En la 3era linea, se corren 12bits(ENVGENSHIFT) a derecha el 1. 
entonces generation queda [0x00001000], eso si el gen anterior dio <=0.
Finalmente en la 4ta linea el env_id va a ser igual al gen qeu tuvo antes, |(or, pone 1 en las posiciones donde e-envs sean 1.)
(e-envs) que es una aritmetica de punteros que da la distancia entre esos 2 lugares de memoria, es decir, la distancia entre
e(el env actual) y el arreglo de envs, osea la posicion de e en ese envs.
#TO DO faltaria ver que hace la primera linea, para ver cuando da un negative env_id. y asi sacar los primeros 5.


2. Supongamos que al arrancar el kernel se lanzan NENV proceso a ejecución. A continuación se destruye
   el proceso asociado a envs[630] y se lanza un proceso que cada segundo muere y se vuelve a lanzar.
   ¿Qué identificadores tendrá este proceso en sus sus primeras cinco ejecuciones?
#TO DO


env_init_percpu
---------------
1. ¿Cuántos bytes escribe la función lgdt, y dónde?
La función 'ldgt' escribe sizeof(gdt) = 48 bits = 6 bytes.
Los escribe en la dirección de memoria donde se encuentra Global Descriptor Table.


2. ¿Qué representan esos bytes?
Esos bytes representan la Base Address, el Segmen Registers y la GDT Entry.



env_pop_tf
----------
1. ¿Qué hay en (%esp) tras el primer movl de la función?
Tras el primer 'movl', el stack pointer (%esp) queda en 0.


2. ¿Qué hay en (%esp) justo antes de la instrucción iret? ¿Y en 8(%esp)?
Justo antes de la instrucción 'iret', el stack pointer (%esp) apunta a 0x8.
#TO DO: que hay en 8(%esp)?


3. ¿Cómo puede determinar la CPU si hay un cambio de ring (nivel de privilegio)?
La CPU conoce el nivel de privilegio gracias al Descriptor Privilege Level (DPL).
En caso de estar en 0, se trata del ring 0 (kernel mode).
En caso de estar en 3, se trata del ring 3 (user mode).


gdb_hello
---------
#TO DO



kern_idt
---------
1. ¿Cómo decidir si usar TRAPHANDLER o TRAPHANDLER_NOEC? ¿Qué pasaría si se usara solamente la primera?
TRAPHANDLER debe utilizarse para aquellas interrupciones/ excepciones que el CPU devuelve un código de error.
En cambio, TRAPHANDLER_NOEC para aquellas que el CPU no devuelve un código de error, sino un 0.

Si se utilizara solamente TRAPHANDLER, el trap frame perdería el formato en aquellos handlers que el CPU
no pushea el código de error. Es decir, el 0 que pushea TRAPHANDLER_NOEC para suplantar el código de error
no estaría, por lo cual se pushearía el número de trapframe en un lugar incorrecto.


2. ¿Qué cambia, en la invocación de handlers, el segundo parámetro (istrap) de la macro SETGATE?
   ¿Por qué se elegiría un comportamiento u otro durante un syscall?
En la invocación de handlers, el parámetro 'istrap' se comporta de la siguiente forma:
	- Si es 0, no permite anidar interrupciones.
	- Si es 1, permite anidar interrupciones.
	
Si se quiere poder pausar el manejo de una interrupción para atender otra, debe setearse en 1.


3. Leer user/softint.c y ejecutarlo con make run-softint-nox. ¿Qué excepción se genera?
   Si hay diferencias con la que invoca el programa... ¿por qué mecanismo ocurre eso, y por qué razones?
Lo que ocurre en 'softint.c' es que se invoca a una interrupción con un nivel de privilegio (DPL) que no corresponde.
En particular, se está llamado a la interrupción 14 (Page Fault) con un DPL=3 (User Mode).
Pero, como SETGATE la definió con un DPL=0 (Kernel Mode), se genera la excepción 'General Protection',
la cual actúa justamente para prevenir que se hagan llamadas con niveles de privilegio incorrectos.


user_evil_hello
---------------
1. ¿En qué se diferencia el código de la versión en evilhello.c mostrada arriba?
Por un lado, 'evilhello.c' le pasa a la syscall un puntero a char.
Por el otro, 'user_evil_hello.c' le pasa a la syscall la dirección de memoria en la cual se encuentra el puntero a char.


2. ¿En qué cambia el comportamiento durante la ejecución? ¿Por qué? ¿Cuál es el mecanismo?
#TO DO
