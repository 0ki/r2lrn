LIBEXT=$(shell r2 -H R2_LIBEXT)
CFLAGS=$(shell pkg-config --cflags r_core)
LDFLAGS=$(shell pkg-config --libs r_core)
R2_USER_PLUGINS=$(shell r2 -H R2_USER_PLUGINS)

all:
	$(CC) -g $(CFLAGS) $(LDFLAGS) -shared -o r2golf.$(LIBEXT) plugin.c
	mkdir -p "$(R2_USER_PLUGINS)"
	cp -f r2golf.$(LIBEXT) "$(R2_USER_PLUGINS)"
	r2 -c 'golf open' --

clean:
	rm -f "r2golf.$(LIBEXT)"
	rm -rf "r2golf.$(LIBEXT).dSYM"
