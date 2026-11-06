# USearch-NSSG

Fork en C++ de [USearch](https://github.com/unum-cloud/USearch) modificado para reemplazar el motor de indexación jerárquica HNSW (*Hierarchical Navigable Small World*) por NSSG (*Navigating Satellite System Graph*).

Esta versión conserva únicamente el núcleo en C++ (cabeceras, pruebas y benchmarks nativos), eliminando los bindings de otros lenguajes del repositorio original para aislar el desarrollo y la evaluación del índice en memoria.

## Descripción técnica

USearch implementa originalmente una variante de HNSW: una estructura multicapa construida de forma incremental donde los niveles superiores actúan como listas de salto de largo alcance y el nivel base (`level 0`) aloja el grafo denso completo.

Este repositorio adapta la estructura interna (`unum::usearch::index_gt`) para operar bajo el modelo de NSSG, el cual prescinde de la jerarquía multicapa y construye un único grafo plano navegable sobre el nivel base:

1. **Almacenamiento plano (`level 0`):** Todos los nodos se alojan sin niveles superiores, reutilizando las cintas de memoria contiguas (`tape_allocator`) y los identificadores comprimidos (`compressed_slot_t`) de USearch.
2. **Grafo k-NN base y expansión a 2 saltos:** A partir de un grafo $k$-NN aproximado inicial (mediante NN-Descent o búsqueda exacta en lotes pequeños), se recolectan para cada nodo sus vecinos directos y los vecinos de sus vecinos hasta un límite de $l$ candidatos ordenados por distancia.
3. **Poda angular (criterio SSG):** Se seleccionan hasta $r$ aristas salientes evaluando la oclusión geométrica entre candidatos mediante la ley de cosenos con un ángulo umbral $\alpha$, distribuyendo las conexiones de forma omnidireccional alrededor de cada vértice.
4. **Aristas inversas y puntos de navegación:** Se insertan enlaces inversos sujetos al mismo criterio de poda angular y se eligen $s$ puntos de navegación (*navigating points*), desde los cuales se construyen árboles de expansión mediante DFS para garantizar la alcanzabilidad global durante la búsqueda.

## Estructura del repositorio

```text
.
├── CMakeLists.txt                  Configuración principal de compilación
├── cmake/                          Módulos auxiliares de CMake
├── include/usearch/
│   ├── index.hpp                   Motor del grafo (index_gt), memoria de vecinos y búsqueda
│   ├── index_dense.hpp             Índice para vectores densos (index_dense_gt) y serialización
│   └── index_plugins.hpp           Métricas SIMD (NumKong), ejecutores de hilos y utilidades de E/S
├── cpp/
│   ├── CMakeLists.txt              Definición de ejecutables de prueba y benchmark
│   ├── test.cpp                    Suite de pruebas unitarias en C++
│   ├── bench.cpp                   Benchmark nativo por línea de comandos
│   └── nssg_playground.cpp         Entorno mínimo de pruebas para NSSG
├── numkong/                        Submódulo de primitivas SIMD para cálculo de distancias
└── stringzilla/                    Submódulo utilizado por la suite de pruebas
```

## Requisitos

- Compilador compatible con C++11 o superior (GCC, Clang o MSVC).
- CMake 3.11 o superior.
- Opcional: OpenMP (para construcción y evaluación multihilo) y jemalloc.

Si se clona el repositorio por primera vez, es necesario inicializar los submódulos si se habilita NumKong o las pruebas unitarias:

```bash
git submodule update --init --recursive
```

## Compilación y ejecución

Configurar y compilar el proyecto en modo `Release`:

```bash
cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSEARCH_BUILD_TEST_CPP=ON \
    -DUSEARCH_BUILD_BENCH_CPP=ON \
    -DUSEARCH_USE_OPENMP=ON

cmake --build build -j
```

Los binarios se generan directamente en el directorio `build/`:

```bash
# Ejecutar pruebas unitarias
./build/test_cpp

# Ejecutar benchmark nativo
./build/bench_cpp --help
```

### Opciones de CMake

| Opción | Valor por defecto | Descripción |
| :--- | :--- | :--- |
| `USEARCH_BUILD_TEST_CPP` | `ON` | Compila el ejecutable de pruebas `test_cpp`. |
| `USEARCH_BUILD_BENCH_CPP` | `ON` | Compila el ejecutable de benchmarks `bench_cpp`. |
| `USEARCH_USE_OPENMP` | `OFF` | Habilita paralelización con OpenMP. |
| `USEARCH_USE_NUMKONG` | `OFF` | Habilita aceleración SIMD mediante NumKong. |
| `USEARCH_USE_JEMALLOC` | `OFF` | Utiliza jemalloc para la reserva de memoria dinámica. |

## Uso como biblioteca (Header-Only)

El motor se distribuye únicamente en archivos de cabecera. Para integrarlo en un ejecutable propio, basta con incluir `include/` en las rutas del compilador:

```cpp
#include <usearch/index.hpp>
#include <usearch/index_dense.hpp>

using namespace unum::usearch;

int main() {
    metric_punned_t metric(128, metric_kind_t::l2sq_k, scalar_kind_t::f32_k);
    index_dense_t index = index_dense_t::make(metric);

    index.reserve(1000);

    float vec[128] = {};
    index.add(1, &vec[0]);

    auto results = index.search(&vec[0], 10);
    for (std::size_t i = 0; i < results.size(); ++i) {
        vector_key_t key = results[i].member.key;
        distance_t dist = results[i].distance;
    }
    return 0;
}
```

## Referencias

- C. Fu, C. Wang, D. Cai, *"High Dimensional Similarity Search With Satellite System Graph: Efficiency, Scalability, and Unindexed Query Compatibility,"* IEEE Transactions on Pattern Analysis and Machine Intelligence (TPAMI), vol. 44, no. 8, pp. 4139-4150, 2022.
- Y. A. Malkov, D. A. Yashunin, *"Efficient and robust approximate nearest neighbor search using Hierarchical Navigable Small World graphs,"* IEEE Transactions on Pattern Analysis and Machine Intelligence (TPAMI), vol. 42, no. 4, pp. 824-836, 2020.
- A. Vardanian, *"USearch: Smaller & Faster Single-File Vector Search Engine,"* Unum Cloud, [github.com/unum-cloud/USearch](https://github.com/unum-cloud/USearch).

## Licencia

Distribuido bajo la licencia Apache 2.0. Consulta el archivo [LICENSE](LICENSE) para más detalles.
