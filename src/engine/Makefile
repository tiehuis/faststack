l := libfs.a
c := $(wildcard *.c)
o := $(patsubst %.c,%.o,$(c))

a: $(o)
	ar rcs $(l) $(o)
	strip -s $(l)
	rm -f $(o)

$(o): $(c)
	clang -Os -static -c $< -o $@
