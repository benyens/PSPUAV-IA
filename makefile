# ===========================================
#  PSP-UAV  | Proyecto IA-PSP 2025
# ===========================================
.PHONY: all clean deepclean runs sweep viz resultsclean

# ---------- COMPILADOR ----------
CXX := g++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra
TARGET := PSP-UAV
SRCS := PSP-UAV.cpp instance.cpp
HDRS := instance.hpp sites.hpp pvp_ga.hpp pvp_fitness.hpp pvp_chromosome.hpp simulate.hpp utils.hpp dump_json.hpp

# ---------- PYTHON ----------
PYTHON := python
SWEEP := sweep.py
VIZ := viz_snapshot.py

# ---------- DIRECTORIOS ----------
INSTDIR := instancias
RESDIR := resultados
PDFDIR := $(RESDIR)/pdfs
SNAPDIR := $(RESDIR)/snaps

# ---------- COMPILAR ----------
all: $(TARGET)

$(TARGET): $(SRCS) $(HDRS)
	@echo "Compilando $(TARGET)..."
	$(CXX) $(CXXFLAGS) -o $@ $(SRCS)
	@echo "Binario listo: ./$(TARGET)"

# ---------- LIMPIEZAS ----------
clean:
	@echo "Limpiando binarios y objetos..."
	@rm -f $(TARGET)
	@echo "Limpieza básica lista."

resultsclean: 
	@echo "Limpiando solo resultados experimentales..."
	@rm -rf $(RESDIR)
	@rm -f *.csv *.json *.png *.pdf
	@echo "Resultados experimentales limpios."

deepclean: clean
	@echo "Limpiando resultados experimentales..."
	@rm -rf $(RESDIR)
	@rm -f *.csv *.json *.png *.pdf
	@echo "Todo limpio (binarios + resultados + gráficos)."


# ---------- CORRIDAS RÁPIDAS ----------
run1a: $(TARGET)
	./$(TARGET) $(INSTDIR)/PSP-UAV_01_a.txt 5 1000 50
run1b: $(TARGET)
	./$(TARGET) $(INSTDIR)/PSP-UAV_01_b.txt 5 1000 50
run2a: $(TARGET)
	./$(TARGET) $(INSTDIR)/PSP-UAV_02_a.txt 10 1000 50
run2b: $(TARGET)
	./$(TARGET) $(INSTDIR)/PSP-UAV_02_b.txt 10 1000 50
run3a: $(TARGET)
	./$(TARGET) $(INSTDIR)/PSP-UAV_03_a.txt 15 1000 50
run3b: $(TARGET)
	./$(TARGET) $(INSTDIR)/PSP-UAV_03_b.txt 15 1000 50

# ---------- BARRIDO Y VISUALIZACIÓN ----------
sweep: $(TARGET) $(SWEEP)
	@echo "Corriento todo para experimentar..."
	$(PYTHON) $(SWEEP)

viz: $(VIZ)
	@echo "Visualizando snapshot más reciente..."
	@latest=$$(ls -t $(SNAPDIR)/*.json 2>/dev/null | head -n1); \
	if [ -z "$$latest" ]; then \
	    echo "No hay JSONs en $(SNAPDIR)/. Ejecuta primero un --dump o make sweep."; \
	else \
	    echo "Mostrando $$latest"; \
	    $(PYTHON) $(VIZ) "$$latest"; \
	fi
