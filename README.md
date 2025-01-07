# Ray-Tracing

### Breve introduzione 
Progetto del corso di Programmazione Avanzata e Parallela.
Il progetto consiste nel costruire un ray tracer che possa effetturare rendering di immagini composte da sfere di varia misura e colore.
L'idea del ray tracing è quella di avere una serie di "raggi" (almeno uno per pixel) emessi da una camera attraverso un viewport che vanno a intersecare diversi oggetti in una scena. Il colore dell'oggetto più vicino che
viene intersecato corrisponderà poi al colore del pixel a cui è associato il raggio.

### src
All'interno di src sono contenuti i file C.

- Il file ```vec3``` gestisce la creazione e le operazioni da svolgere sui raggi del ray tracer.

- Il file ```scene``` gestisce creazione, riempimento e cancellazione della scena.

- Il file ```render``` legge la rappresentazione della scena e colora i pixel.

- Il file ```ppm``` compone l'immagine in formato ppm.

- Il file ```test``` contiene le funzioni che misurano il tempo di esecuzione delle funzioni.

In ```render.c``` sono presenti due versioni della funzione ```render``` per calcolare il colore del raggio e per riempire l'array di dati di scena, una versione contiene i normali cicli for innestati, l'altra contiene la parallelizzazione di essi.
(La versione utilizzata )
### note particolari
In```render.c ``` l'unica funzione parallelizzata è ```omp_render_scene``` sebbene anche ```colore_raggio``` venga chiamata dentro una sezione parallela. Questo per evitare race conditions.

### build
Questa cartella contiene tutti i file '.o'

### prove_txt
In questa cartella sono presenti vari file di scena per sperimentare con varie disposizioni del viewport e delle sfere.

### renders
In questa cartella vengono salvati i file in formato ppm.

### sintassi
Per far funzionare correttamente il programma è necessario che l'input sia scritto in questa forma:

```bash
./main scene_file.txt image_file.ppm width height  
```

# Esempio di utilizzo
### Eseseguibile
Innanzitutto è necessario compilare i file necessari con
```bash
make
```
per ottenere

### Esecuzione

```bash
./main prove_txt/prova11.txt renders/immagine11.ppm 1920 1080
```



