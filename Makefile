CC=gcc-4.6

CCFLAGS=-Wall -Wextra -O2 -I include/
CCPLUGFLAGS=-O2 -I include/ -I `gcc -print-file-name=plugin`/include -fPIC
CCTESTFLAGS=-O3 -fplugin=./$(EXEC) -Wl,-rpath,./
CCLIBFLAGS=-O2 -I include/ -fPIC -shared

SRC=$(wildcard src/*.c)
BUILDDIR=build
OBJ=$(addprefix $(BUILDDIR)/, $(patsubst %.c, %.o, $(SRC)))
EXEC=plug_instrument.so

SRCLIB=$(wildcard lib/*.c)
LIB=libruntime.so

LDFLAGS=-L./ -lruntime

SRCTEST=$(wildcard test/*.c)
TEST=$(patsubst %.c, %, $(SRCTEST))

.PHONY: all update plugin lib test clean mrproper cleantest cleanlib debug instrument

all: lib $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -shared $^ -o $@
	yacc -o lexer/y.tab.c -d -v lexer/parser.yy
	lex -o lexer/lex.yy.c lexer/parser.lex
	$(CC) -I include/ lexer/y.tab.c lexer/lex.yy.c -lfl -o instrument

$(OBJ): $(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CCPLUGFLAGS) -c $< -o $@

$(TEST): $(SRCTEST)
	$(CC) $(CCTESTFLAGS) $^ -o $@ $(LDFLAGS)

$(LIB): $(SRCLIB)
	$(CC) -shared $(CCLIBFLAGS) $^ -o $@

update: $(EXEC)

debug: $(SRCTEST)
	gdb --command=r --args $(CC) $(CCTESTFLAGS) $^ -o $@ $(LDFLAGS)

lib: $(LIB)

# plugin: $(PLUG)

test: $(TEST)

instrument: 
	cat dtrack.inst strack.inst | ./instrument

clean:
	rm -f $(OBJ)

cleantest:
	rm -f $(TEST)

cleanlib:
	rm -f $(LIB)

mrproper: clean cleantest cleanlib
	rm -f $(EXEC)
