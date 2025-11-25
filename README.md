# PSP-UAV – Optimización de Patrullaje mediante Algoritmo Genético

**Autor:** Benjamín Ferrada Larach  
**Curso:** INF-285 – Inteligencia Artificial  
**Institución:** Universidad Técnica Federico Santa María  
**Semestre:** 2025-2  

---

## 1. Descripción general

Este proyecto implementa una solución al **problema PSP-UAV (Patrolling Scheduling Problem for Unmanned Aerial Vehicles)**, cuyo objetivo es **maximizar la cobertura de celdas con distintos niveles de urgencia** utilizando un número limitado de drones en un entorno discretizado con obstáculos.

El enfoque implementado corresponde a un **algoritmo evolutivo (genético)** basado en la metodología descrita en el paper *"Multi-Mission Learning Workflow with Genetic Algorithms (MMLW-GA)"*.  
Este algoritmo busca mejorar rutas de patrullaje mediante procesos iterativos de selección, cruce y mutación de soluciones candidatas, balanceando exploración y explotación en el espacio de búsqueda.

---

## 2. Estructura del proyecto

```
PSP-UAV/
│
├── PSP-UAV.cpp             # Programa principal (ejecución completa)
├── instance.cpp / .hpp      # Lectura de instancias desde archivo
├── pvp_ga.hpp               # Implementación del algoritmo genético
├── pvp_chromosome.hpp       # Representación de individuos
├── pvp_fitness.hpp          # Cálculo de aptitud (fitness)
├── simulate.hpp             # Simulación del comportamiento de drones
├── dump_json.hpp            # Exportación de resultados a JSON
├── sweep.py                 # Automatización de experimentos
├── showcase.py              # Visualización animada (GIF)
├── vis_snapshot.py          # Visualización estática
├── instancias/              # Instancias de prueba (.txt)
├── resultados/
│   ├── showcase/            # Salidas y JSON del showcase
│   ├── gifs/                # Animaciones generadas
│   ├── pdfs/                # Reportes experimentales
│   └── snapshots/           # Snapshots individuales
└── Makefile                 # Compilación y automatización
```

---

## 3. Compilación

El proyecto se compila utilizando **Makefile**.  
Para compilar el ejecutable principal:

```bash
make
```

Esto generará el binario:

```
./PSP-UAV
```

---

## 4. Ejecución básica

Ejemplo de ejecución manual:

```bash
./PSP-UAV instancias/PSP-UAV_01_a.txt 5 1000 50
```

Donde:
- `instancias/PSP-UAV_01_a.txt`: ruta del archivo de instancia.
- `5`: número máximo de drones.
- `1000`: parámetro K, usado para determinar generaciones.
- `50`: tamaño de la ventana temporal T (ticks de simulación).

Opcionalmente, pueden usarse los siguientes parámetros adicionales:

| Parámetro | Descripción | Valor por defecto |
|------------|--------------|-------------------|
| `--seed`   | Semilla del generador aleatorio | 12345 |
| `--gen`    | Número de generaciones | Depende de K |
| `--pop`    | Tamaño de población | 100 |
| `--Nmult`  | Factor de largo del recorrido | 3 |
| `--csv`    | Archivo de salida de resultados | ninguno |
| `--tag`    | Etiqueta del experimento | vacío |
| `--dump`   | Ruta para exportar snapshot JSON | ninguno |
| `--outdir` | Carpeta de salida (para showcase) | resultados/showcase |

---

## 5. Ejemplos de uso con Makefile

Ejecutar una instancia estándar:

```bash
make run1a
```

Ejecutar todas las instancias:

```bash
make all
```

Creación de csv's y pdf:

```bash
make sweep
```

Creación de gráficos y visualizaciones:

```bash
make vis
```

Generar showcase completo (corrida + animación de la primera instancia):

```bash
make showcase
```

Generar showcase de otra instancia definida: 

```bash
make showcase SHOWCASE_RUN={run} #Ej: run3a
```

Limpiar binarios:

```bash
make clean
```

Limpiar todo:

```bash
make deepclean
```

Eliminar resultados experimentales:

```bash
make resultsclean
```

---

## 6. Visualización de resultados

El sistema genera automáticamente visualizaciones a partir de los archivos JSON (`snapshots`):

- `vis_snapshot.py`: genera figuras estáticas (`grid`, `heatmap`, `routes`, `kernel`).
- `showcase.py`: genera animaciones en formato GIF dentro de `resultados/gifs/`.

Ejemplo de uso directo:

```bash
python showcase.py --animate --fps 60
```

---

## 7. Metodología del algoritmo genético

1. **Inicialización:**  
   Se generan rutas iniciales (cromosomas) de longitud N, representando secuencias de celdas a visitar.

2. **Evaluación de aptitud (fitness):**  
   Cada individuo es evaluado según la urgencia acumulada ponderada por la distancia y penalizaciones por celdas inaccesibles.

3. **Selección y reproducción:**  
   Se aplica selección por torneo, cruzamiento de segmentos y mutación aleatoria.

4. **Reemplazo:**  
   Las nuevas generaciones sustituyen parcialmente a las anteriores (elitismo).

5. **Terminación:**  
   El proceso continúa hasta alcanzar el número máximo de generaciones o convergencia.

6. **Simulación final:**  
   La mejor ruta (`best_walk`) se distribuye entre los drones disponibles y se simula la cobertura sobre la grilla.

---

## 8. Referencia bibliográfica

Basado en la metodología presentada en:

> **Multi-Mission Learning Workflow with Genetic Algorithms (MMLW-GA)**,  
> Applied Intelligence Journal, 2023.

Adaptado para el problema PSP-UAV dentro del curso INF-285 (UTFSM, 2025-2).

---

## 9. Observaciones

- El código fue diseñado para ser modular y reproducible.
- Los scripts de Python requieren **matplotlib**, **numpy** y **Pillow**.
- Las animaciones son puramente visuales; no afectan el resultado numérico.
- El parámetro `--outdir` permite separar los resultados de experimentos y visualizaciones.

---
