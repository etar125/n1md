VERSION = 0.2.2

PREFIX = $(HOME)/.local

ECFLAGS = -Iinclude -std=c89 -pedantic -Wall -le1l -DVERSION=\"$(VERSION)\" $(CFLAGS)

CC = cc
AR = ar
