# Particle Simulator – Prestazioni e Ottimizzazioni

## 📊 TESTS INIZIALI

| Numero di Particelle | FPS Stimato     |
|----------------------|-----------------|
| 5.000                | 35–40 fps       |
| 10.000               | 30 fps          |
| 50.000               | 5–6 fps         |

---

## ⚙️ OTTIMIZZAZIONI IMPLEMENTATE

1. `cmake -DCMAKE_BUILD_TYPE=Release .`  
   ➤ Molto importante, migliora notevolmente le prestazioni.
2. Disegno separato delle particelle verdi e rosse  
   ➤ Poco impattante, migliora leggermente gli fps evitando cambi colore frequenti.
3. Uso di `SDL_RenderDrawPoints` per disegnare tutti i punti in un’unica chiamata  
   ➤ Significativo aumento di prestazioni rispetto al disegno singolo.
4. Programmazione **multithread**  
   ➤ Migliora notevolmente la simulazione a carichi alti.
5. **Frame skipping** con molte particelle  
   ➤ Non porta benefici, a meno di abbassare drasticamente il target FPS.  
   ➤ A FPS bassi la simulazione risulta più realistica.
6. Funzione di gestione collisioni ottimizzata  
   ➤ Riduce il tempo di calcolo delle interazioni.
7. tramite `ulimit -s` aumento la dimensione massima dello stack (`ulimit -a` per vedere i limiti correnti), questo funziona solo per la shell corrente

---

## ❌ FALLIMENTI

1. Uso dell’**insertion sort**  
   ➤ Non efficiente per il numero di particelle.
2. Multithreading con assegnazione dei chunk una sola volta  
   ➤ Inefficace: i carichi non erano bilanciati tra i thread.
3. Spatial Grid
   ➤ Non sono capace, ho fatto un implementazione inefficente

---

## ✅ FUNZIONALITÀ AGGIUNTE

1. **Resize** dinamico della finestra.
2. **Controllo della velocità del tempo** con le frecce:
    - Freccia destra = velocizza
    - Freccia sinistra = rallenta
      ➤ Il `dt` (delta time) viene modificato dinamicamente.
3. **Pausa** con la **barra spaziatrice**.
4. **Zoom** con + e -

---