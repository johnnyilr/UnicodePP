all: test


.PHONY: test
test: \
bin/regex_tests.exe \
bin/tests.exe


bin/regex_tests.exe: \
obj/test/catch.o \
obj/test/main.o \
obj/test/regex/anchor.o \
obj/test/regex/misc.o \
obj/test/regex/replace.o \
obj/test/regex/split.o | \
bin/unicode.dll
	$(GPP) -o $@ $^ bin/unicode.dll
	bin/regex_tests.exe


bin/tests.exe: \
obj/test/catch.o \
obj/test/main_old.o | \
bin/unicode.dll
	$(GPP) -o $@ $^ bin/unicode.dll
	bin/tests.exe
