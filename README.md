# USearch (Fork NSSG — Estructuras de Datos Avanzadas)

Proyecto enfocado en la investigación, modificación y evaluación de algoritmos de búsqueda aproximada de vecinos más cercanos (ANN - Approximate Nearest Neighbors).

---

## 🎯 Objetivo del Proyecto

El objetivo principal es **reemplazar el algoritmo de indexación por grafos [HNSW](https://arxiv.org/abs/1603.09320) (Hierarchical Navigable Small World) por [NSSG](https://arxiv.org/abs/1907.06146) (Navigating Spreading-out Graph / Navigating Spatial Small World Graph)**.

### HNSW vs. NSSG:
- **HNSW actual:** Estructura jerárquica multicapa donde las capas superiores tienen aristas largas para saltos rápidos y la capa base (`layer 0`) contiene el grafo denso.
- **NSSG objetivo:** Grafo plano navegable (monocapa) construido a partir de un grafo kNN aproximado, optimizado mediante poda angular y aristas de navegación espacial para lograr menor uso de memoria y alta velocidad de convergencia en consultas.

---

## 📂 Lenguajes y Componentes Activos

El repositorio ha sido depurado para mantener únicamente los entornos relevantes para el proyecto:

- **C++20 / C++17:** Núcleo algorítmico y estructuras de datos (`include/usearch/`).
- **Python:** Interfaz de scripting, notebooks de evaluación, benchmarks y pruebas (`python/`).
- **Java:** Bindings JNI para aplicaciones Java (`java/`).
- **C:** Capa de interoperabilidad C99 estándar (`c/`).

---

## 🗺️ Dónde Modificar el Código

| Archivo / Carpeta | Propósito |
| :--- | :--- |
| [include/usearch/index.hpp](file:///home/mariab/Documents/EDA/Usearch/Usearch/USearch/include/usearch/index.hpp) | **Núcleo algorítmico principal.** Aquí reside la estructura del grafo HNSW (`index_gt`), la lógica de niveles, selección de vecinos y búsqueda greedily/beam search. **Aquí se realizarán los cambios para NSSG.** |
| [include/usearch/index_dense.hpp](file:///home/mariab/Documents/EDA/Usearch/Usearch/USearch/include/usearch/index_dense.hpp) | Envoltorio para vectores densos, casting de tipos (f32, f16, i8) y métricas SIMD. |
| [cpp/test.cpp](file:///home/mariab/Documents/EDA/Usearch/Usearch/USearch/cpp/test.cpp) | Pruebas unitarias de C++ para validar inserción, búsqueda y persistencia en disco. |
| [cpp/bench.cpp](file:///home/mariab/Documents/EDA/Usearch/Usearch/USearch/cpp/bench.cpp) | Benchmark nativo en C++ para medir latencia y QPS (Queries Per Second). |
| [python/lib.cpp](file:///home/mariab/Documents/EDA/Usearch/Usearch/USearch/python/lib.cpp) | Enlace de PyBind11 entre el motor C++ y Python. |
| [java/cloud/unum/usearch/](file:///home/mariab/Documents/EDA/Usearch/Usearch/USearch/java/cloud/unum/usearch/) | Implementación JNI para interacción con Java. |

---

## 🚀 Cómo Ejecutar y Probar

### 1. Pruebas Rápidas en Python

El entorno virtual `.venv` ya está configurado con las dependencias necesarias (`usearch`, `numpy`).

#### Ejecutar script de prueba:
```bash
# Opción 1: Directo con el binario del entorno
.venv/bin/python test_search.py

# Opción 2: Activando el entorno
source .venv/bin/activate
python test_search.py
```

#### Recompilar la extensión local tras hacer cambios en C++:
Si modificas el código C++ en `include/` y deseas probar los cambios desde Python:
```bash
source .venv/bin/activate
pip install -e . --no-build-isolation
```

---

### 2. Compilación y Pruebas en C++

Para compilar las pruebas y benchmarks nativos de C++:

```bash
# 1. Configurar el build con CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release -DUSEARCH_BUILD_TEST_CPP=ON -DUSEARCH_BUILD_BENCH_CPP=ON

# 2. Compilar
cmake --build build -j

# 3. Ejecutar los tests unitarios
./build/cpp/test_cpp

# 4. Ejecutar benchmarks
./build/cpp/bench_cpp
```

---

### 3. Pruebas en Java (JNI)

Si trabajas con Java:

```bash
# 1. Compilar la biblioteca JNI nativa
cmake -B build -DUSEARCH_BUILD_JNI=ON
cmake --build build --target usearch_jni -j

# 2. Ejecutar los tests de Java con Gradle
./gradlew test
```

---

## 📋 Hoja de Ruta: De HNSW a NSSG

1. **Análisis de la estructura actual:**
   - Inspeccionar en [include/usearch/index.hpp](file:///home/mariab/Documents/EDA/Usearch/Usearch/USearch/include/usearch/index.hpp) cómo se gestionan los niveles (`level_t`), la tabla de vecinos y el punto de entrada (`entry_slot_`).
2. **Construcción del Grafo Base:**
   - Implementar o reutilizar el mecanismo para generar el grafo k-NN inicial aproximado.
3. **Poda de Aristas de NSSG:**
   - Implementar el algoritmo de selección/poda de aristas basado en visibilidad espacial y ángulos (Monotonic Relative Neighborhood Graph) para garantizar transitividad y navegabilidad.
4. **Punto de Navegación Central (Navigating Node):**
   - Determinar y almacenar el punto de entrada óptimo para la búsqueda en el grafo monocapa.
5. **Evaluación de Rendimiento:**
   - Comparar Recall vs. QPS y consumo de memoria entre el HNSW original y la nueva versión con NSSG usando [cpp/bench.cpp](file:///home/mariab/Documents/EDA/Usearch/Usearch/USearch/cpp/bench.cpp) o scripts en Python.
