TP3: Multitarea con desalojo
========================

static_assert
---------

1. ¿Cómo y por qué funciona la macro static_assert que define JOS?
Para la evaluacion(comparacion) se le deben pasar consatntes (que no cambien a lo largo de la ejecucion del programa), por eso este assert hace la comparacion en tiempo de compilacion.


env_return
---------

1. al terminar un proceso su función umain() ¿dónde retoma la ejecución el kernel? Describir la secuencia de llamadas desde que termina umain() hasta que el kernel dispone del proceso.
TODO: no se donde garcha esta la funcion umain()

2. ¿en qué cambia la función env_destroy() en este TP, respecto al TP anterior?
Ahora env_destroy(e) primero detecta si el env a eliminar esta corriendo en otro CPU, en este caso le cambia el estado para que la proxima vez el Kernel lo detecte, lo libere. Sino lo destruye, se fija si esta el env actual corriendo y en este caso llama a sched_yield() para detectar el proximo env a ejecutar (usando round robin).
