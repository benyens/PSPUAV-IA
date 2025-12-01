import os
import json
import glob
import re
import pandas as pd

# --- CONFIGURACIÓN ---
# La carpeta donde el Makefile guarda los jsons
CARPETA_RESULTADOS = "resultados/defensa" # O "resultados/defensa" si usaste esa en el makefile
ARCHIVO_SALIDA = "resultados/resumen_experimentos_defensa.csv"

def parsear_nombre(filename):
    """
    Extrae la configuración del nombre del archivo.
    Espera formato: PSP-UAV_01_a_d1_T100_g500_p50_run1.json
    """
    # Regex explicada:
    # (.*)       -> Grupo 1: Nombre de la instancia (ej: PSP-UAV_01_a)
    # _d(\d+)    -> Grupo 2: Drones
    # _T(\d+)    -> Grupo 3: Ticks (T)
    # _g(\d+)    -> Grupo 4: Generaciones
    # _p(\d+)    -> Grupo 5: Población
    # _run(\d+)  -> Grupo 6: Número de corrida (Seed)
    patron = r"(.*)_d(\d+)_T(\d+)_g(\d+)_p(\d+)_run(\d+)\.json"
    
    match = re.search(patron, filename)
    if match:
        return {
            "Instancia": match.group(1),
            "Drones": int(match.group(2)),
            "T (Ticks)": int(match.group(3)),
            "Gen": int(match.group(4)),
            "Pop": int(match.group(5)),
            "Run": int(match.group(6))
        }
    else:
        # Si el archivo tiene nombre antiguo o raro
        return {"Instancia": filename, "Drones": -1}

def main():
    # 1. Verificar directorio
    if not os.path.exists(CARPETA_RESULTADOS):
        print(f"Error: La carpeta '{CARPETA_RESULTADOS}' no existe.")
        print("   Asegúrate de haber corrido 'make rundefensa_all' primero.")
        return

    # 2. Buscar archivos JSON
    patron_busqueda = os.path.join(CARPETA_RESULTADOS, "*.json")
    archivos = glob.glob(patron_busqueda)
    
    if not archivos:
        print(f"No se encontraron archivos .json en '{CARPETA_RESULTADOS}'")
        return

    print(f"Procesando {len(archivos)} archivos de experimentos...")
    
    filas = []

    # 3. Leer cada archivo
    for ruta in archivos:
        nombre_archivo = os.path.basename(ruta)
        
        # A. Extraer datos del NOMBRE (Configuración)
        datos = parsear_nombre(nombre_archivo)
        
        # B. Extraer datos del JSON (Resultados)
        try:
            with open(ruta, 'r') as f:
                contenido = json.load(f)
                
                # --- AQUÍ ESTÁ LA CLAVE: Usamos tus nombres exactos del JSON ---
                urgencia = contenido.get("best_urgency_total", 0)
                tiempo = contenido.get("total_time", 0.0)
                factible = contenido.get("feasible", False)
                razon = contenido.get("feasible_reason", "")
                
                # Agregamos a la fila
                datos["Best Urgency"] = urgencia
                datos["Tiempo (s)"] = round(tiempo, 4)
                datos["Factible"] = "SI" if factible else "NO"
                # Opcional: Si quieres saber por qué falló
                # datos["Razón"] = razon 
                
                filas.append(datos)

        except Exception as e:
            print(f"Error leyendo {nombre_archivo}: {e}")

    # 4. Crear Tabla (DataFrame)
    df = pd.DataFrame(filas)

    # 5. Ordenar columnas y filas para que se vea bonito
    columnas_deseadas = [
        "Instancia", "Drones", "T (Ticks)", "Gen", "Pop", "Run", 
        "Best Urgency", "Tiempo (s)", "Factible"
    ]
    # Filtramos por si alguna columna no salió (por errores de nombre)
    cols_finales = [c for c in columnas_deseadas if c in df.columns]
    df = df[cols_finales]

    # Ordenamos: Primero por instancia, luego drones, luego generaciones
    if not df.empty and "Instancia" in df.columns:
        df = df.sort_values(by=["Instancia", "Drones", "Gen", "Pop", "Run"])

    # 6. Mostrar y Guardar
    print("\n" + "="*60)
    print("RESUMEN (Primeras 10 filas)")
    print("="*60)
    print(df.head(10).to_string(index=False))
    
    df.to_csv(ARCHIVO_SALIDA, index=False)
    print("\n" + "="*60)
    print(f"¡Listo! Tabla completa guardada en: {ARCHIVO_SALIDA}")
    print("="*60)

if __name__ == "__main__":
    main()