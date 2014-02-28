all: data


.PHONY: data
data: \
bin/data.exe


bin/data.exe: \
obj/data/casing.o \
obj/data/codeoutput.o \
obj/data/main.o \
obj/data/parser.o \
obj/data/tointeger.o | \
bin/unicodedata.dll
	$(GPP) -o $@ $^ bin/unicodedata.dll