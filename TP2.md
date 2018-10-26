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

Generation queda [0x00001000], eso si el generation de la linea 1 dio <=0.
Finalmente en la 4ta linea | (or, pone 1 en las posiciones donde e-envs sean 1)
(e-envs) que es una aritmetica de punteros que da la distancia entre esos 2 lugares de memoria, es decir, la distancia entre
e(el env actual) y el arreglo de envs, osea la posicion de e en ese envs.
entonces el env_id del primer proceso qeuda [0x1000], el segundo en [0x1001], el tercero en [0x1002], el cuarto en [0x1003] 
y el quinto en [0x1004]

2. Supongamos que al arrancar el kernel se lanzan NENV proceso a ejecución. A continuación se destruye
   el proceso asociado a envs[630] y se lanza un proceso que cada segundo muere y se vuelve a lanzar.
   ¿Qué identificadores tendrá este proceso en sus sus primeras cinco ejecuciones?
Siguiendo la logica de la pregunta 1, el primero quedaria en [0x1276], el segundo en [0x1277], el tercero en [0x1278],
el cuarto en [0x1279] y el quinto en [0x127A]. ya que (e - envs) da 630 [0x276]


env_init_percpu
---------------
1. ¿Cuántos bytes escribe la función lgdt, y dónde?
La función 'ldgt' escribe sizeof(gdt) = 48 bits = 6 bytes.
Los escribe en la dirección de memoria donde se encuentra Global Descriptor Table.

2. ¿Qué representan esos bytes?
Esos bytes representan la Base Address, el Segment Registers y la GDT Entry.


env_pop_tf
----------
1. ¿Qué hay en (%esp) tras el primer movl de la función?
Tras el primer 'movl', el stack pointer (%esp) queda en 0.

2. ¿Qué hay en (%esp) justo antes de la instrucción iret? ¿Y en 8(%esp)?
En $esp se encuentra el $eip. En 8($esp) se encuentra el trapno.

3. ¿Cómo puede determinar la CPU si hay un cambio de ring (nivel de privilegio)?
La CPU conoce el nivel de privilegio gracias al Descriptor Privilege Level (DPL).
En caso de estar en 0, se trata del ring 0 (kernel mode).
En caso de estar en 3, se trata del ring 3 (user mode).


gdb_hello
---------

1.
$ make gdb
gdb -q -s obj/kern/kernel -ex 'target remote 127.0.0.1:26000' -n -x .gdbinit
Reading symbols from obj/kern/kernel...done.
Remote debugging using 127.0.0.1:26000
warning: No executable has been specified and target does not support
determining executable automatically.  Try using the "file" command.
0x0000fff0 in ?? ()
(gdb) b env_pop_tf
Breakpoint 1 at 0xf0102e92: file kern/env.c, line 502.
(gdb) c
Continuing.
The target architecture is assumed to be i386
=> 0xf0102e92 <env_pop_tf>:	push   %ebp

Breakpoint 1, env_pop_tf (tf=0xf01c0000) at kern/env.c:502
502	{

2. 
$ make run-hello-nox-gdb
make[1]: Entering directory '/home/ntorresdalmas/Desktop/sisop/tp/7508_tp'
+ cc kern/env.c
+ cc kern/trap.c
+ ld obj/kern/kernel
+ mk obj/kern/kernel.img
make[1]: Leaving directory '/home/ntorresdalmas/Desktop/sisop/tp/7508_tp'
qemu-system-i386 -nographic -drive file=obj/kern/kernel.img,index=0,media=disk,format=raw -serial mon:stdio -gdb tcp:127.0.0.1:26000 -D qemu.log  -d guest_errors -S
6828 decimal is 15254 octal!
Physical memory: 131072K available, base = 640K, extended = 130432K
check_page_alloc() succeeded!
check_page() succeeded!
check_kern_pgdir() succeeded!
check_page_installed_pgdir() succeeded!
[00000000] new env 00001000
QEMU 2.11.1 monitor - type 'help' for more information
(qemu) info registers
EAX=003bc000 EBX=f01c0000 ECX=f03bc000 EDX=0000022a
ESI=00010094 EDI=00000000 EBP=f0118fd8 ESP=f0118fbc
EIP=f0102e92 EFL=00000092 [--S-A--] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0010 00000000 ffffffff 00cf9300 DPL=0 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]

3.
(gdb) p tf
$1 = (struct Trapframe *) 0xf01c0000

4.
(gdb) p sizeof(struct Trapframe) / sizeof(int)
$4 = 17
(gdb) x/17x tf
0xf01c1000:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01c1010:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01c1020:	0x00000023	0x00000023	0x00000000	0x00000000
0xf01c1030:	0x00800020	0x0000001b	0x00000000	0xeebfe000
0xf01c1040:	0x00000023


5.
(gdb) disas
Dump of assembler code for function env_pop_tf:
   0xf0102f0e <+0>:	push   %ebp
   0xf0102f0f <+1>:	mov    %esp,%ebp
   0xf0102f11 <+3>:	sub    $0xc,%esp
   0xf0102f14 <+6>:	mov    0x8(%ebp),%esp
=> 0xf0102f17 <+9>:	popa   
   0xf0102f18 <+10>:	pop    %es
   0xf0102f19 <+11>:	pop    %ds
   0xf0102f1a <+12>:	add    $0x8,%esp
   0xf0102f1d <+15>:	iret   
   0xf0102f1e <+16>:	push   $0xf0105915
   0xf0102f23 <+21>:	push   $0x1ff
   0xf0102f28 <+26>:	push   $0xf01058ba
   0xf0102f2d <+31>:	call   0xf01000a9 <_panic>
End of assembler dump.

"o usar (gdb) si 4" (M=4)""

6.
(gdb) x/17x $sp
0xf01c1000:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01c1010:	0x00000000	0x00000000	0x00000000	0x00000000
0xf01c1020:	0x00000023	0x00000023	0x00000000	0x00000000
0xf01c1030:	0x00800020	0x0000001b	0x00000000	0xeebfe000
0xf01c1040:	0x00000023

7.
Afirmamos el funcionamiento de env_pop_tf viendo que restaura a cero todos los registros que se usaron en el proceso.
vemos que los primeros 8 estan en cero ya que son los registros que se 'usan' en el trapframe.

8. 
(qemu) info registers
EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000000
ESI=00000000 EDI=00000000 EBP=00000000 ESP=f01c0030
EIP=f0102ea1 EFL=00000096 [--S-AP-] CPL=0 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =0008 00000000 ffffffff 00cf9a00 DPL=0 CS32 [-R-]

Vemos que todos los registros que usa Trapframe se setean en cero, luego claramente cambia el esp, eip, ya que tienen que ver
con el flujo de la ejecucion, el que direccion de memoria saltar(eip) y el estado del stack(esp)

9.
(gdb) si
=> 0x800020:	cmp    $0xeebfe000,%esp
0x00800020 in ?? ()
(gdb) disas
No function contains program counter for selected frame.
(gdb) p $pc
$3 = (void (*)()) 0x800020
(gdb) p $eip
$4 = (void (*)()) 0x800020
(gdb) symbol-file obj/user/hello
Load new symbol table from "obj/user/hello"? (y or n) y
Reading symbols from obj/user/hello...done.
Error in re-setting breakpoint 1: Function "env_pop_tf" not defined.
(gdb) p $pc
$5 = (void (*)()) 0x800020 <_start>

(qemu) info registers
EAX=00000000 EBX=00000000 ECX=00000000 EDX=00000000
ESI=00000000 EDI=00000000 EBP=00000000 ESP=eebfe000
EIP=00800020 EFL=00000002 [-------] CPL=3 II=0 A20=1 SMM=0 HLT=0
ES =0023 00000000 ffffffff 00cff300 DPL=3 DS   [-WA]
CS =001b 00000000 ffffffff 00cffa00 DPL=3 CS32 [-R-]

Vemos que tampoco se notan muchos cambios, los 'obvios' mencionados en el punto anterior y ahora tambien el code segment.

10.
(gdb) tbreak syscall
Temporary breakpoint 2 at 0xf01033a6: file {standard input}, line 73.

TO DO: 
despues de la ejecucion de la instruccion int 0x30 (48, numero elegido para T_SYSCALL) ...



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
No cambia el comportamiento, ya que la salida es la misma, 