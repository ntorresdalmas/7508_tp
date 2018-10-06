TP1: Memoria virtual en JOS
===========================

page2pa
-------
La función page2pa() mapea un struct PageInfo con la dirección física de la página a la que apunta.
Esto lo logra mediante una aritmética de punteros y un corrimiento de bits.

En primer lugar, al PageInfo recibido por parámetro se le resta la estructura Pages, obteniendo así el índice
del PageInfo en cuestión dentro de la estructura Pages.

Luego, con el corrimiento de bits, se localiza la dirección física de la página identificada.
Considerando que la dirección física de una página se encuentra a partir del bit 12
y que estamos tratando con una arquitectura Little Endian (el bit más representativo a derecha),
tiene sentido que realicemos un desplazamiento a izquierda de PGSHIFT=12 bits.


boot_alloc_pos
--------------
a. En la primera llamada a boot_alloc(), cuando se crea el page directory, nextfree esta como NULL, entonces
hace una llamada a ROUNDUP de la ultima direccion de donde se cargo el kernel. Entonces en la terminal tiro un /obj/kern$nm kernel me tira la memoria " f011794c B pages " ya que las variables globales son lo ultimo que se cargan cuando se carga en kernel, a esta direccion (0xf0117950)[4027677008] se le hace un ROUNDUP, que alinea la direccion a PGSIZE(0x00001000)[4096], es decir, llega (0xf0119000)[4027682816].


b. " $ make gdb
gdb -q -s obj/kern/kernel -ex 'target remote 127.0.0.1:26000' -n -x .gdbinit
Reading symbols from obj/kern/kernel...done.
Remote debugging using 127.0.0.1:26000
warning: No executable has been specified and target does not support
determining executable automatically.  Try using the "file" command.
0x0000fff0 in ?? ()
(gdb) b boot_alloc
Breakpoint 1 at 0xf0100b27: file kern/pmap.c, line 89.
(gdb) c
Continuing.
The target architecture is assumed to be i386
=> 0xf0100b27 <boot_alloc>:	push   %ebp

Breakpoint 1, boot_alloc (n=0) at kern/pmap.c:89
89	{
(gdb) p nextfree
$1 = 0x0
(gdb) p (char*)end
$2 = 0xf01006f6 <cons_init> "U\211\345\203\354\b\350\266\373\377\377\350\277\372\377\377\200=4u", <incomplete sequence \360>
(gdb) display/i nextfree
1: x/i nextfree
   0x0:	push   %ebx
(gdb) display/i (char*)end
2: x/i (char*)end
   0xf01006f6 <cons_init>:	push   %ebp
(gdb) c
Continuing.
=> 0xf0100b27 <boot_alloc>:	push   %ebp

Breakpoint 1, boot_alloc (n=0) at kern/pmap.c:89
89	{
1: x/i nextfree
   0xf0119000:	add    %al,(%eax)
2: x/i (char*)end
   0xf01006f6 <cons_init>:	push   %ebp
(gdb) p nextfree
$5 = 0xf0119000 ""
(gdb) p (char*) end
$6 = 0xf01006f6 <cons_init> "U\211\345\203\354\b\350\266\373\377\377\350\277\372\377\377\200=4u", <incomplete sequence \360>
(gdb) display/i nextfree
3: x/i nextfree
   0xf0119000:	add    %al,(%eax)
(gdb) display/i (char*)end
4: x/i (char*)end
   0xf01006f6 <cons_init>:	push   %ebp
(gdb) display/i nextfree
5: x/i nextfree
   0xf0119000:	add    %al,(%eax) "

page_alloc
----------
La función page2kva() utiliza la ya mencionada función page2pa() pero, en vez de devolver la dirección física
del struct PageInfo recibido, devuelve la dirección virtual a través de la macro KADDR.


