# Copyright (c) 2026 etar125 Admanse
# Licensed under ISC (see LICENSE)

include config.mk

SRC = n1md.c
OBJ = $(SRC:%.c=%.o)

all: n1md

%.o: %.c config.mk
	$(CC) $(ECFLAGS) -c $< -o $@

n1md: $(OBJ)
	$(CC) $(ECFLAGS) $< -o $@

clean:
	rm -rf n1md
	rm -rf $(OBJ)

install: n1md
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp n1md $(DESTDIR)$(PREFIX)/bin/
	chmod +x $(DESTDIR)$(PREFIX)/bin/n1md

uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/n1md

.PHONY: all clean install uninstall
