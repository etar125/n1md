VERSION = "0.1.0"

PREFIX = /usr

ECFLAGS = -Iinclude -std=c99 -pedantic -Wall -le1l -DVERSION=\"$(VERSION)\" $(CFLAGS)

CC = cc
AR = ar
