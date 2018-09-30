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
a. 

b. 


page_alloc
----------
La función page2kva() utiliza la ya mencionada función page2pa() pero, en vez de devolver la dirección física
del struct PageInfo recibido, devuelve la dirección virtual a través de la macro KADDR.


