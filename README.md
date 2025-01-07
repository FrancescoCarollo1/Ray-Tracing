# Ray-Tracing

### Breve introduzione 
Progetto in Linguaggio C del corso di Programmazione Avanzata e Parallela.  
Il progetto consiste nel costruire un ray tracer che possa effetturare rendering di immagini composte da sfere di varie dimensioni e colori.
L'idea del ray tracing è quella di emettere una serie di "raggi" (almeno uno per pixel) da una camera attraverso un viewport. Questi raggi intersecano gli oggetti nella scena, e il colore del pixel corrisponderà a quello dell'oggetto più vicino intersecato.

---

### Struttura dei file sorgenti (src)
Nella cartella src sono contenuti i file sorgenti C.

- Il file ```vec3``` gestisce la creazione e le operazioni sui vettori tridimensionali.

- Il file ```scene``` si occupa di creazione, gestione e cancellazione della scena.

- Il file ```render``` legge la rappresentazione della scena e calcola il colore dei pixel.

- Il file ```ppm``` compone l'immagine finale in formato ppm.

---

### Note particolari
In```render.c ``` l'unica funzione parallelizzata è ```omp_render_scene``` sebbene anche ```colore_raggio``` venga chiamata dentro una sezione parallela. Questo per evitare race conditions.

---

## Struttura delle cartelle
- *build*:  contiene tutti i file oggetto ('.o') generati durante la compilazione.
- *prove_txt*: contiene i vari file di scena per sperimentare con varie configurazioni di viewport e sfere.

- *renders*: In questa cartella vengono salvati i file in formato PPM.

---

### Sintassi di Esecuzione
Per far funzionare correttamente il programma è necessario che l'input sia scritto in questa forma:

```bash
./main scene_file.txt image_file.ppm width height  
```
Assicurarsi che il percorso del file di scena e del file immagine siano specificati correttamente.

# Esempio di utilizzo
### Compilazione
Innanzitutto è necessario compilare i file necessari con:
```bash
make
```

### Esecuzione
Esempio di esecuzione del comando
```bash
./main prove_txt/prova11.txt renders/immagine11.ppm 1920 1080
```



