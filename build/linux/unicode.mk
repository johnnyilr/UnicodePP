all: unicode


.PHONY: unicode
unicode: bin/unicode.so


bin/unicode.so: \
obj/boundaries.o \
obj/byteordermark.o \
obj/caseconverter.o \
obj/codepoint.o \
obj/comparer.o \
obj/condition.o \
obj/conditions.o \
obj/data/getdata.o \
obj/defaultlocale.o \
obj/encoding.o \
obj/encodingaction.o \
obj/encodingerror.o \
obj/endianencoding.o \
obj/error.o \
obj/locale.o \
obj/normalizer.o \
obj/string.o \
obj/utf8.o \
obj/utf16.o \
obj/utf32.o | \
bin
	$(GPP) -shared -o $@ $^