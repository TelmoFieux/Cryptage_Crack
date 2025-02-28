CC  = gcc
SYS = -std=gnu11 -Wall -Wextra -O2
PTHREAD_SYS = $(SYS) -lpthread

SRCDIR = src
OBJDIR = obj
BINDIR = bin
FILESDIR = fichiers

SYM_CRYPT = $(BINDIR)/sym_crypt
LOGICIEL = $(BINDIR)/logiciel
CRACK_MASK = $(BINDIR)/crack_mask
BREAK_CODE = $(BINDIR)/break_code
BREAK_CODE_PERFORMANCE = $(BINDIR)/break_code_performance
TESTS_BIN = $(BINDIR)/tests

OBJ_COMMON = $(OBJDIR)/chiffrement.o $(OBJDIR)/utils.o $(OBJDIR)/dh_prime.o $(OBJDIR)/stats.o
OBJ_COMMON2 = $(OBJDIR)/utils.o $(OBJDIR)/list.o $(OBJDIR)/dh_prime.o $(OBJDIR)/stats.o $(OBJDIR)/break_code_c1.o $(OBJDIR)/break_code_c2.o $(OBJDIR)/break_code_c3.o
OBJ_COMMON3 = $(OBJDIR)/utils.o $(OBJDIR)/dh_prime.o $(OBJDIR)/chiffrement.o
OBJ_COMMON4 = $(OBJDIR)/utils.o $(OBJDIR)/list.o $(OBJDIR)/dh_prime.o $(OBJDIR)/stats.o $(OBJDIR)/break_code_c1_multi_threaded.o $(OBJDIR)/break_code_c2_multi_threaded.o $(OBJDIR)/break_code_c3_multi_threaded.o
OBJ_COMMON5 = $(OBJDIR)/utils.o $(OBJDIR)/dh_prime.o

OBJ_CRYPT = $(OBJDIR)/sym_crypt.o
OBJ_CRACK = $(OBJDIR)/break_code_main.o
OBJ_CRACK_MASK = $(OBJDIR)/crack_mask.o
OBJ_CRACK_PERFORMANCE = $(OBJDIR)/break_code_main_multi_threaded.o
OBJ_LOGICIEL = $(OBJDIR)/Logiciel.o
OBJ_TESTS = $(OBJDIR)/tests.o

# ===========================================================
# Règles Principales
# ===========================================================
all: crypt crack crack_mask logiciel test crack_p

# Chiffrement
crypt: dirs $(OBJ_COMMON) $(OBJ_CRYPT)
	$(CC) -o $(SYM_CRYPT) $(OBJ_COMMON) $(OBJ_CRYPT) $(SYS)

# Crack
crack: dirs $(OBJ_COMMON2) $(OBJ_CRACK)
	$(CC) -o $(BREAK_CODE) $(OBJ_COMMON2) $(OBJ_CRACK) $(SYS)

# Crack Mask
crack_mask: dirs $(OBJ_COMMON3) $(OBJ_CRACK_MASK)
	$(CC) -o $(CRACK_MASK) $(OBJ_COMMON3) $(OBJ_CRACK_MASK) $(SYS)

# crack performance
crack_p: dirs $(OBJ_COMMON4) $(OBJ_CRACK_PERFORMANCE)
	$(CC) -o $(BREAK_CODE_PERFORMANCE) $(OBJ_COMMON4) $(OBJ_CRACK_PERFORMANCE) $(SYS)

# Logiciel
logiciel: dirs $(OBJ_COMMON5) $(OBJ_LOGICIEL)
	$(CC) -o $(LOGICIEL) $(OBJ_COMMON5) $(OBJ_LOGICIEL) $(SYS)

# Tests
test: dirs setup $(OBJ_COMMON2) $(OBJDIR)/chiffrement.o $(OBJ_TESTS)
	@echo "Compilation des tests..."
	$(CC) -o $(TESTS_BIN) $(OBJ_COMMON2) $(OBJDIR)/chiffrement.o $(OBJ_TESTS) $(SYS)
	@echo "Exécution des tests..."
	$(TESTS_BIN)

# Setup : Copie des fichiers de test nécessaires
setup:
	@echo "Copie des fichiers de test dans le répertoire courant..."
	cp $(SRCDIR)/message.txt ./
	cp $(SRCDIR)/empty.txt ./
	cp $(SRCDIR)/large_file.txt ./
	cp $(SRCDIR)/sample_encrypted.txt ./
	cp $(SRCDIR)/dictionary.txt ./

# ===========================================================
# Règles Génériques
# ===========================================================
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo "Compilation de $<..."
	$(CC) -o $@ -c $< $(SYS)

$(OBJDIR)/break_code_c2_multi_threaded.o: $(SRCDIR)/break_code_c2_multi_threaded.c
	$(CC) -o $@ -c $< $(PTHREAD_SYS)

$(OBJDIR)/break_code_c3_multi_threaded.o: $(SRCDIR)/break_code_c3_multi_threaded.c
	$(CC) -o $@ -c $< $(PTHREAD_SYS)

# ===========================================================
# Directoires
# ===========================================================
dirs:
	@mkdir -p $(OBJDIR)
	@mkdir -p $(BINDIR)

# ===========================================================
# Nettoyage
# ===========================================================
clean:
	rm -f $(OBJDIR)/*.o
	rm -f $(BINDIR)/*
	rm -f message.txt empty.txt large_file.txt sample_encrypted.txt dictionary.txt
	@echo "Nettoyage terminé."

# ===========================================================
# Aide
# ===========================================================
help:
	@echo "Cibles disponibles :"
	@echo "  make all        - Compiler tout"
	@echo "  make crypt      - Compiler le chiffrement"
	@echo "  make crack      - Compiler le crack"
	@echo "  make crack_mask - Compiler crack_mask"
	@echo "  make logiciel   - Compiler le logiciel principal"
	@echo "  make test       - Compiler et exécuter les tests"
	@echo "  make clean      - Nettoyer les fichiers objets et binaires"