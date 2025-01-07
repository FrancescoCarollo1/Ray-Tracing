# Ray-Tracing


Progetto del corso di Programmazione Avanzata e Parallela.
Il progetto consiste nel costruire un ray tracer che possa effetturare rendering di immagini composte da sfere di varia misura e colore.
L'idea del ray tracing è quella di avere una serie di "raggi" (almeno uno per pixel) che
sono emessi da una camera attraverso un "viewport" (pensatelo come l'obiettivo della
camera) e che vanno a intersecare diversi oggetti in una scena. Il colore dell'oggetto che
viene intersecato corrisponderà poi al colore del pixel a cui è associato il raggio.
Il progrramma è stato suddiviso in diversi file ai quali è associata una funzione specifica:
- Il file ```vec3``` gestisce la creazione e le operazioni da svolgere sui raggi del ray tracer.

- Il file ```scene``` gestisce creazione, riempimento e cancellazione della scena.

- Il file ```render``` legge la rappresentazione della scena e colora i pixel.

- Il file ```ppm``` compone l'immagine in formato ppm.

- Il file ```test``` contiene le funzioni che misurano il tempo di esecuzione delle funzioni.

All'interno del file ```render.c``` sono presenti due versioni della funzione ```render``` per calcolare il colore del raggio e per riempire l'array di dati di scena, una versione contiene i normali cicli for innestati, l'altra contiene la parallelizzazione di essi.


