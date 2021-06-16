# __IA para el videojuego Cynthia__
## *Proyecto final de la asignatura Inteligencia Artificial para Videojuegos*

### Desarrollado por __Jorge Moreno Martínez__

<br>
<br>



# __Índice__
- [Introducción](#Introducción)
- [Motivación del proyecto](#Motivación-del-proyecto)
- [Mecánicas de la demo](#Mecánicas-de-la-demo)
- [Diseño de la IA](#Diseño-de-la-IA)
    - [Cynthia](#Cynthia)
        - [Alternativas exploradas](#Alternativas-analizadas-del-comportamiento-de-Cynthia)


---

<br>
<br>



# __Introducción__

Este proyecto es una demo técnica desarrollada en Unity con el objetivo de servir como campo de pruebas para tener una Inteligencia Artificial funcional y divertida en otro proyecto: la segunda parte del videojuego [*Cynthia*](https://kryystinn.itch.io/cynthia).
<br>

La demo consta de un nivel concreto del juego, un hospital, en donde el jugador es víctima de una persecución a manos del enemigo principal del juego. Este enemigo es una mujer, de nombre __Cynthia__, la madre de un bebé que también está en el hospital.
<br>
<br>

<img src="./Images/CynthiaWalk.gif"/>
<br>
<br>

Dentro del juego, el objetivo es obtener un objeto coleccionable y después salir por el mismo lugar por el que has entrado. En realidad, en *Cynthia* este objeto es una parte de la historia, que no está incluida para ahorrar la implementación.
<br>

Cynthia irá merodeando por el mapa del hospital, buscándote continuamente. No obstante, si escucha al bebé lo priorizará incluso antes de ir a por ti. Esto genera una dinámica de provocar a Cynthia y distraerla de su patrulla para poder obtener el objeto coleccionable. Si Cynthia se acerca demasiado a ti, pierdes la partida.

---

<br>
<br>



# __Motivación del proyecto__

[*Cynthia*](https://kryystinn.itch.io/cynthia) es un videojuego creado para una Game Jam, el TechFest 2021. El juego fue ganador del concurso, pero no se quedó ahí: triunfó bastante entre fans del género de horror en Japón, por lo que los desarrolladores decidieron hacer una segunda parte.

Yo estoy participando en esta segunda entrega, en la que nos hemos encontrado con la necesidad de hacer una Inteligencia Artificial para la zona del hospital. El desarrollo está siendo en Unreal Engine 4.

Así pues, dado que durante todo el cuatrimestre hemos estado trabajando con Unity y que UE4 te da muchas cosas hechas, he decidido hacer la demo en Unity para demostrar lo aprendido en la asignatura. Más tarde, exportaré los comportamientos al proyecto de Unreal.

---

<br>
<br>



# __Mecánicas de la demo__

Dentro de la demo, el jugador puede hacer las siguientes acciones:

- Agacharse, caminar, correr y saltar (todos estos movimientos producen ruido, cada uno más que el anterior).
- Coger al bebé en brazos y soltarlo.
- Calmar al bebé cuando está llorando.
- Coger el objeto coleccionable que sirve para acabar la partida.
- Escapar del hospital por el mismo sitio por el que ha entrado.

Además, Cynthia puede:

- Andar por todo el escenario (incluyendo escaleras).
- Coger al bebé y soltarlo en una cuna.
- Calmar al bebé cuando está llorando.
- Perseguir al jugador.
- Asesinar al jugador.

Y por último, el bebé puede:

- Llorar.

(Como aclaración, esto es lo único que hacen los bebés humanos en general, por lo que se podría decir que el comportamiento del bebé es muy realista y podría merecer una matrícula de honor).

---

<br>
<br>


# __Diseño de la IA__
<br>

## __Cynthia__
<br>

Cynthia se dedica a priorizar al bebé antes que a ninguna otra cosa que haya en el mapa.

Si el bebé llora, irá a protegerlo: se desplazará hasta la posición en la que lo oiga y lo cogerá. A continuación, lo calmará y lo dejará en la __cuna más próxima__: para ello he investigado en profundidad acerca de la clase NavMesh y he llegado a un __algoritmo que encuentra la posición más cercana a un elemento que puede estar fuera de la NavMesh__ (porque Cynthia no puede recorrrer las cunas pero debería dejar al bebé en una para que este deje de llorar). 

Si el bebé no está llorando, su objetivo es encontrar al jugador: para ello, va recorriendo el mapa de manera aleatoria. En el momento en el que detecta al jugador (ya sea por el sonido que producen sus pasos o porque le ve directamente), le persigue (y chilla si lo ve, dado el odio visceral que siente hacia él).

<br>

### __Alternativas analizadas del comportamiento de Cynthia__

<br>

- __Prioridades__

    Inicialmente, Cynthia no priorizaba al bebé sobre el jugador, sino que se dedicaba a perseguir al jugador siempre que lo veía, incluso si su bebé estaba llorando. Esto fue descartado porque __podría provocar que el jugador se quedara estancado__, sin poder desviar la atención de Cynthia de ninguna manera (lo mismo ocurre con la situación en la que Cynthia se queda patruyando en la zona en la que está el bebé durmiendo).

