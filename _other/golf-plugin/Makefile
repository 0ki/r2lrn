LIBEXT=$(shell r2 -H R2_LIBEXT)
CFLAGS=$(shell pkg-config --cflags r_core)
LDFLAGS=$(shell pkg-config --libs r_core)
R2_USER_PLUGINS=$(shell r2 -H R2_USER_PLUGINS)

all:
	$(CC) -g $(CFLAGS) $(LDFLAGS) -fPIC -shared -o r2golf.$(LIBEXT) plugin.c

install:
	mkdir -p "$(R2_USER_PLUGINS)"
	cp -f r2golf.$(LIBEXT) "$(R2_USER_PLUGINS)"

run:
	r2 -c 'golf open' --

uninstall:
	rm -f "$(R2_USER_PLUGINS)/r2golf.$(LIBEXT)"

clean:
	rm -f "r2golf.$(LIBEXT)"
	rm -rf "r2golf.$(LIBEXT).dSYM"
