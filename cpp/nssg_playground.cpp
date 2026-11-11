#include <usearch/index_dense.hpp>
#include <iostream>
#include <vector>
#include <random>
#include <cassert>

/*
Prueba#1 del issue2: Que ejecute aplanando HNSW
Siguiente paso: Optimizar memoria

Código para correr: g++ -std=c++11 -O2 -I./include -pthread cpp/nssg_playground.cpp -o playground && ./playground
*/

int main() {
    // Empaquetar: dimensiones (4), métrica (l2sq) y tipo escalar (f32)
    unum::usearch::metric_punned_t metric(
        4, 
        unum::usearch::metric_kind_t::l2sq_k, 
        unum::usearch::scalar_kind_t::f32_k
    );

    // make() devuelve un resultado; con o sin argumento config toma los valores por defecto
    auto index_result = unum::usearch::index_dense_t::make(metric);
    if (!index_result) {
        std::cerr << "Error al inicializar el índice: " << index_result.error.what() << std::endl;
        return 1;
    }
    // Extraer el índice del resultado
    auto index = std::move(index_result.index);

    index.reserve(1000);

    std::mt19937 rng(42);
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for (int i = 0; i < 1000; ++i) {
        std::vector<float> vector_data = { dist(rng), dist(rng), dist(rng), dist(rng) };
        index.add(i, vector_data.data());
    }

    std::vector<float> query = { 0.5f, 0.5f, 0.5f, 0.5f };
    auto results = index.search(query.data(), 8);

    for (std::size_t j = 0; j < results.size(); ++j) {
        std::cout << "Vecino ID: " << results[j].member.key 
                  << " - Distancia: " << results[j].distance << std::endl;
    }

    assert(index.max_level() == 0);

    std::cout << "\nIndex.max_level(): " << index.max_level() << "\n";

    return 0;
}