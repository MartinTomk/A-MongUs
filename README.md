Trabajo practico de la Materia Sistemas Operativos de 3er Año de la carrera Ingenieria en Sistemas

Integrantes:

Ignacio Campaño (Modulo File System y Discordiador)
Martin Tomkinson (Modulo Mi-Ram y Discordiador)

En el presente trabajo busca simular las tareas realizadas por un sistema operativo, desde la creación de procesos (Patotas) y sus threads (Tripulantes, los cuales fueron modelados con 
hilos, utilizando la libreria pthread.h) continuando con la planificación elegida (Round Robin/FIFO) para que puedan realizar sus tareas respetando los correspondientes ciclos de CPU y el grado de multiprogramación utilizando semáforos.
A la hora de la creación de cada proceso es necesario almacenar los datos correspondientes al proceso e hilos en memoria, para esto tenemos 2 esquemas de memoria, segmentación con compactación para aprovechar al máximo la memoria evitando la fragmentación interna y compactando para solucionar la fragmentación externa y paginación con swap para utilizar espacio de disco como memoria virtual, este esquema posee una fragmentación interna considerable debido a que se puede desperdiciar toda una página - 1 byte. Es por estos incovenientes que poseen que se utiliza Segmentación paginada, Disminuir la fragmentacion interna, evitar la fragmentacion externa, facilitar mover datos desde y hacia la memoria virtual.
