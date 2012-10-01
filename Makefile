CC=gcc

CCFLAGS=-Wall -Wextra -O2 -I include/
CCPLUGFLAGS=-O2 -I `gcc -print-file-name=plugin`/include -fPIC
CCTESTFLAGS=-fplugin=./$(EXEC) -Wall -Wextra -O2
LDFLAGS=

SRC=$(wildcard src/*.c)
BUILDDIR=build
OBJ=$(addprefix $(BUILDDIR)/, $(patsubst %.c, %.o, $(SRC)))
EXEC=plug_instrument.so

SRCTEST=$(wildcard test/*.c)
TEST=$(patsubst %.c, %, $(SRCTEST))

.PHONY: all update plugin test clean mrproper cleantest

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -shared $^ -o $@

$(OBJ): $(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CCPLUGFLAGS) -c $< -o $@

$(TEST): $(SRCTEST)
	$(CC) $(CCTESTFLAGS) $^ -o $@ $(LDFLAGS)

update: $(EXEC)

# plugin: $(PLUG)

test: $(TEST)

clean:
	rm -f $(OBJ)

cleantest:
	rm -f $(TEST)

mrproper: clean cleantest
	rm -f $(EXEC)
