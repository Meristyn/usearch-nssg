# Plan de Implementación: Reemplazo de HNSW por NSSG en USearch

> **Contexto del Proyecto:** Reemplazar el motor HNSW dentro de USearch (`include/usearch/index.hpp`) por NSSG (*Navigating Satellite System Graph*, Fu et al., IEEE TPAMI 2021) en C++11, sin usar cuantizadores ni árboles espaciales (KD-Tree/R-Tree).

---

## 📅 Resumen de Milestones y Etiquetas

| Milestone | Etiqueta | Descripción |
| :--- | :--- | :--- |
| **Semana 1** | `semana-1` | Aplanar USearch y dominar el entorno |
| **Semana 2** | `semana-2` | Grafo k-NN inicial y Candidatos a 2 Saltos |
| **Semana 3** | `semana-3` | Poda Angular, Aristas Inversas y Árbol DFS |
| **Semana 4** | `semana-4` | Paralelización, Datasets Reales y Gráficas |

---

## Semana 1: Aplanar USearch y dominar el entorno (`semana-1`)
*Milestone: Semana 1 - Aplanar USearch y dominar el entorno*

- [ ] **#01 Crear cpp/nssg_playground.cpp de prueba mínima**
  - **Objetivo:** Crear un archivo C++ mínimo en `cpp/` que incluya `<usearch/index_dense.hpp>`, inserte 100 vectores aleatorios de dimensión 4 (`f32_k`, métrica `l2sq_k`) y ejecute `search()`.
  - **Criterio de aceptación:** Compila con C++11 y muestra 5 vecinos en consola sin errores.

- [ ] **#02 Forzar nivel 0 en choose_random_level()**
  - **Objetivo:** Inspeccionar `include/usearch/index.hpp`, localizar la generación de nivel aleatorio al insertar nodos y forzar que devuelva siempre `0` para aplanar la jerarquía.
  - **Criterio de aceptación:** Al insertar 1,000 vectores e imprimir `index.max_level()`, el resultado en consola es `0`.

- [ ] **#03 Localizar e imprimir listas de vecinos del Nivel 0**
  - **Objetivo:** Ubicar en `index.hpp` la función que accede al `span` de vecinos en la capa base (`level = 0`) de un `slot` dado.
  - **Criterio de aceptación:** Función auxiliar `print_neighbors(slot)` funcionando e imprimiendo en consola los IDs conectados al nodo 0.

- [ ] **#04 Implementar setter manual set_neighbors_level0(slot, lista)**
  - **Objetivo:** Escribir un método en `index_gt` que permita sobrescribir la lista de `compressed_slot_t` en la cinta de memoria del nivel 0 de cualquier nodo.
  - **Criterio de aceptación:** Conectar manualmente el nodo 0 con el nodo 5, llamar a `print_neighbors(0)` y verificar que devuelva `[5]`.

- [ ] **#05 Implementar setter manual para entry_slot_**
  - **Objetivo:** Ubicar `entry_slot_` en `index.hpp` y exponer un método para definir manualmente el nodo de entrada de las búsquedas.
  - **Criterio de aceptación:** Cambiar `entry_slot_ = 42`, ejecutar `index.search()` y confirmar que recorre el grafo sin fallos de segmentación.

- [ ] **#06 Implementar inserción cruda en lote (add_raw_batch)**
  - **Objetivo:** Crear una rutina que almacene los vectores en los slots de USearch y reserve su memoria para `level = 0` con grado máximo `connectivity = r`, omitiendo la búsqueda e inserción incremental de HNSW.
  - **Criterio de aceptación:** 10,000 vectores se alojan en memoria en milisegundos con 0 aristas conectadas.

---

## Semana 2: Grafo k-NN inicial y Candidatos a 2 Saltos (`semana-2`)
*Milestone: Semana 2 - Grafo k-NN inicial y Candidatos a 2 Saltos*

- [ ] **#07 Atajo seguro: Generar grafo k-NN base con exact_search_t**
  - **Objetivo:** Usar `exact_search_t` (NumKong SIMD nativo de USearch) para obtener los `k = 20` vecinos más cercanos de un dataset pequeño (1,000 a 10,000 puntos) y guardarlos en `std::vector<std::vector<uint32_t>> knn_graph`.
  - **Criterio de aceptación:** Matriz `knn_graph` generada correctamente para poder probar el resto del pipeline de NSSG sin bloqueos.

- [ ] **#08 Paso 01 (NN-Descent): Inicialización aleatoria sin duplicados**
  - **Objetivo:** Crear la función `build_nndescent(n, k)` que inicialice para cada nodo `k` vecinos aleatorios distintos y calcule sus distancias usando `index.metric()`.
  - **Criterio de aceptación:** Todos los `n` nodos tienen exactamente `k` vecinos iniciales válidos con sus distancias.

- [ ] **#09 Paso 01 (NN-Descent): Generación de listas inversas**
  - **Objetivo:** Construir las listas de vecinos inversos (`reverse_lists`) en cada iteración para conocer qué nodos apuntan al nodo `i` y formar su vecindario general.
  - **Criterio de aceptación:** Unión de vecinos directos e inversos generada sin fugas de memoria.

- [ ] **#10 Paso 01 (NN-Descent): Bucle de refinamiento local ("vecino de mi vecino")**
  - **Objetivo:** Programar el bucle iterativo que evalúa pares de vecinos de cada nodo, actualiza los heaps/listas si halla una distancia menor y se detiene tras 5-8 iteraciones o baja tasa de cambios.
  - **Criterio de aceptación:** En 1,000 vectores, el número de actualizaciones disminuye por iteración y alcanza >90% de recall respecto al grafo exacto del Issue #07.

- [ ] **#11 Paso 02: Expansión de candidatos a 2 saltos (get_2hop_candidates)**
  - **Objetivo:** Implementar las líneas 4-14 del Algoritmo 3 de NSSG: para un nodo `i`, recolectar en un conjunto `P` sus vecinos en `knn_graph` y los vecinos de sus vecinos, eliminando duplicados y cortando al llegar a `l` candidatos.
  - **Criterio de aceptación:** Devuelve un vector `P` sin IDs repetidos ni el propio nodo `i`, con tamaño máximo `l`.

- [ ] **#12 Ordenar conjunto de candidatos P por distancia ascendente**
  - **Objetivo:** Calcular las distancias desde el nodo `i` a cada candidato en `P` (si no están en caché) y ordenarlos de menor a mayor distancia con `std::sort`.
  - **Criterio de aceptación:** `P[0]` es siempre el candidato más cercano al nodo `i`.

---

## Semana 3: Poda Angular, Aristas Inversas y Árbol DFS (`semana-3`)
*Milestone: Semana 3 - Poda Angular, Aristas Inversas y Árbol DFS*

- [ ] **#13 Implementar función matemática is_occluded_by_angle()**
  - **Objetivo:** Codificar el criterio geométrico de cono de SSG usando la Ley de Cosenos sobre distancias: devolver `true` (conflicto/ocluido) si `(d_ij^2 + d_ik^2 - d_jk^2) / (2 * d_ij * d_ik) > cos_alpha`.
  - **Criterio de aceptación:** Pruebas unitarias pasando para ángulos de 30°, 60° (`cos = 0.5`) y 90° (`cos = 0.0`).

- [ ] **#14 Paso 03: Bucle de Poda Angular SSG sobre candidatos P**
  - **Objetivo:** Implementar el Algoritmo 2 sobre `P`: insertar `P[0]` en la lista de aceptados y aceptar `d_j` solo si no tiene conflicto angular con ningún `d_k` ya aceptado, hasta un grado máximo `r`.
  - **Criterio de aceptación:** De `l = 100` candidatos con `alpha = 60°`, el nodo conserva entre 15 y 40 aristas omnidireccionales.

- [ ] **#15 Paso 04: Inserción de aristas inversas con poda angular**
  - **Objetivo:** Implementar las líneas 18-24 del Algoritmo 3: para cada arista `(i -> j)`, intentar agregar `i` a la lista de vecinos de `j` usando la misma regla `is_occluded_by_angle()` y truncar al grado máximo `r` si se excede.
  - **Criterio de aceptación:** Las listas de adyacencia incorporan enlaces bidireccionales válidos sin superar jamás el límite `r`.

- [ ] **#16 Paso 05a: Selección de s = 10 Nodos de Navegación (Navigating Points)**
  - **Objetivo:** Seleccionar aleatoriamente `s = 10` nodos del dataset y almacenar sus slots en un arreglo `navigating_nodes_` dentro del índice.
  - **Criterio de aceptación:** Vector `navigating_nodes_` poblado con 10 IDs únicos al construir el índice.

- [ ] **#17 Paso 05b: Árboles de expansión DFS para reforzar conectividad**
  - **Objetivo:** Implementar las líneas 26-29 del Algoritmo 3: ejecutar un recorrido DFS desde cada uno de los `s` nodos de navegación y añadir aristas hacia los nodos no visitados más cercanos hasta cubrir el 100% del grafo.
  - **Criterio de aceptación:** El conteo de nodos alcanzables desde cualquiera de los 10 puntos de navegación es exactamente `N`.

- [ ] **#18 Volcado del grafo NSSG a las cintas de USearch y ajuste de search()**
  - **Objetivo:** Copiar las listas de adyacencia finales de NSSG al nivel 0 de USearch (usando el método del Issue #04) y hacer que `search()` inicie su cola de prioridad desde el nodo en `navigating_nodes_` más cercano a la consulta `q`.
  - **Criterio de aceptación:** `index.search()` nativo de USearch funciona sobre el grafo NSSG y logra >95% de Recall@10 en pruebas locales.

---

## Semana 4: Paralelización, Datasets Reales y Gráficas (`semana-4`)
*Milestone: Semana 4 - Paralelización, Datasets Reales y Gráficas*

- [ ] **#19 Paralelizar la construcción de NSSG con OpenMP / executor_default_t**
  - **Objetivo:** Paralelizar el bucle de los Pasos 02 y 03 (recolección de 2 saltos y poda angular por nodo) usando `#pragma omp parallel for` o el `executor_default_t` de USearch.
  - **Criterio de aceptación:** La etapa de selección de aristas utiliza todos los hilos del CPU sin condiciones de carrera y reduce el tiempo de construcción varias veces.

- [ ] **#20 Implementar lector de datasets estándar (SIFT-128 y GloVe-100)**
  - **Objetivo:** Cargar en C++ los vectores base, las queries y el ground truth (top-100) desde formato `.fvecs`/`.ivecs` o binario extraído de `ann-benchmarks`.
  - **Criterio de aceptación:** Lectura verificada de dimensiones (`d = 128` para SIFT, `d = 100` para GloVe) y conteo de vectores correcto en consola.

- [ ] **#21 Ejecutar benchmark comparativo: USearch-HNSW vs USearch-NSSG**
  - **Objetivo:** Medir en condiciones idénticas (1 hilo para búsqueda, sin cuantización `f32_k`) ambos motores variando `expansion_search` entre 16 y 256.
  - **Criterio de aceptación:** Archivo `results_comparison.csv` exportado con columnas: `Algorithm, Dataset, efSearch, Recall@10, Recall@100, QPS, IndexSize_MB, BuildTime_s`.

- [ ] **#22 Experimento de sensibilidad del ángulo alpha (50°, 55°, 60°, 65°, 70°)**
  - **Objetivo:** Construir el índice NSSG variando el hiperparámetro `alpha` para evaluar cómo cambia el grado promedio de salida (AOD) y el QPS (replicando la Fig. 5 del paper de TPAMI).
  - **Criterio de aceptación:** Archivo `results_alpha.csv` generado mostrando la variación de densidad y velocidad por ángulo.

- [ ] **#23 Generar gráficas finales en Python (matplotlib)**
  - **Objetivo:** Escribir un script en Python que lea `results_comparison.csv` y `results_alpha.csv` y exporte las curvas `Recall vs QPS` (escala logarítmica en Y) y `Tamaño de índice`.
  - **Criterio de aceptación:** Gráficas exportadas en `.png` y `.pdf` listas para insertar en las diapositivas finales.

- [ ] **#24 Limpieza de código, comentarios y conclusiones del experimento**
  - **Objetivo:** Limpiar warnings de compilación, documentar las funciones añadidas en `index.hpp` y redactar el análisis comparativo contrastando los resultados obtenidos con la hipótesis planteada en la exposición inicial.
  - **Criterio de aceptación:** Repositorio limpio, compilando en modo `Release` (`-O3`) y listo para la entrega final.

