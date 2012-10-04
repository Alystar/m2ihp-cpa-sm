CC=gcc

CCFLAGS=-Wall -Wextra -O2 -I include/
CCPLUGFLAGS=-O2 -I include/ -I `gcc -print-file-name=plugin`/include -fPIC
CCTESTFLAGS=-fplugin=./$(EXEC) -fdump-tree-ssa
CCLIBFLAGS=-O2 -fPIC -shared

SRC=$(wildcard src/*.c)
BUILDDIR=build
OBJ=$(addprefix $(BUILDDIR)/, $(patsubst %.c, %.o, $(SRC)))
EXEC=plug_instrument.so

SRCLIB=$(wildcard lib/*.c)
LIB=runtime.so

LCTESTFLAGS=-L ./
LDFLAGS=-lruntime -lm

SRCTEST=$(wildcard test/*.c)
TEST=$(patsubst %.c, %, $(SRCTEST))

.PHONY: all update plugin lib test clean mrproper cleantest cleanlib

all: lib $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -shared $^ -o $@

$(OBJ): $(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CCPLUGFLAGS) -c $< -o $@

$(TEST): $(SRCTEST)
	$(CC) $(CCTESTFLAGS) $(LCTESTFLAGS) $^ -o $@ $(LDFLAGS)

$(LIB): $(SRCLIB)
	$(CC) -shared $(CCLIBFLAGS) $^ -o $@

update: $(EXEC)

lib: $(LIB)

# plugin: $(PLUG)

test: $(TEST)

clean:
	rm -f $(OBJ)

cleantest:
	rm -f $(TEST)

cleanlib:
	rm -f (LIB)

mrproper: clean cleantest cleanlib
	rm -f $(EXEC)
