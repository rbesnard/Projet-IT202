.PHONY : default clean clean-all

CC=gcc

CFLAGS = -W -Wall -g `pkg-config --cflags glib-2.0`
LDFLAGS = `pkg-config --libs glib-2.0`

VPATH=./
SRC=$(wildcard $(patsubst %/,%/*.c,$(subst :, ,$(VPATH))))
DEP=$(patsubst %.c, %.d, $(SRC))
OBJ=$(patsubst %.c, %.o, $(SRC))

TARGET=example

BOLD="\033[01m"
NORMAL="\033[0m"
RED="\033[01m\033[31m"
GREEN="\033[01m\033[32m"
YELLOW="\033[01m\033[33m"
OK=echo $(GREEN)"OK"$(NORMAL)
FAILED=echo $(RED)"FAILED"$(NORMAL)
ERROR=if [ -s .tmp ]; then echo -n $(YELLOW); cat .tmp | sed 's|^|\t|g'; echo $(NORMAL); fi


default : example

clean :
	@echo $(BOLD)"Nettoyage..."$(NORMAL)
	@rm -vf $(OBJ) *~ *.d | sed 's|^|\t|g'
	@rm -f .tmp


clean-all :
	@echo $(BOLD)"Nettoyage..."$(NORMAL)
	@rm -vf $(OBJ) *~ *.d | sed 's|^|\t|g'
	@rm -f .tmp
	@rm -f $(TARGET)

all : $(TARGET)

example : example.o thread.o
	@echo -n $(BOLD)"$(CC) -o $@ $^ $(LDFLAGS) : "$(NORMAL)
	@($(CC) -o $@ $^ $(LDFLAGS) 2> .tmp && $(OK) && $(ERROR)) || ($(FAILED) && $(ERROR) && false)

%.d : %.c
	@$(CC) $(CFLAGS) -M -o $@ $<

-include $(DEP)

%.o : %.c
	@echo -n $(BOLD)"$(CC)-o $@ -c $< : "$(NORMAL)
	@($(CC) $(CFLAGS) -o $@ -c $< 2> .tmp && $(OK) && $(ERROR)) || ($(FAILED) && $(ERROR) && false)