import numpy as np
from usearch.index import search, MetricKind, Matches, BatchMatches

print("1. Generando vectores sintéticos...")
vectors = np.random.rand(10_000, 1024).astype(np.float32)
vector = np.random.rand(1024).astype(np.float32)

print("2. Ejecutando búsqueda exacta (1 contra muchos)...")
one_in_many: Matches = search(vectors, vector, 50, MetricKind.L2sq, exact=True)
print(f"   ✓ Encontradas {len(one_in_many)} coincidencias.")
print(f"   ✓ Índice del más cercano: {one_in_many.keys[0]}, distancia L2sq: {one_in_many.distances[0]:.4f}")

print("3. Ejecutando búsqueda por lotes (muchos contra muchos)...")
# Reducimos a 100 vectores de consulta para una prueba rápida
query_vectors = vectors[:100]
many_in_many: BatchMatches = search(vectors, query_vectors, 10, MetricKind.L2sq, exact=True)
print(f"   ✓ Búsqueda batch finalizada para {len(many_in_many)} vectores de consulta.")
print("¡Prueba completada exitosamente!")
