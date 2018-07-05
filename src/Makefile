# Makefile for Solaris 2.1 on a Sparc 10 with gcc.
# Changes contributed by Trienne.

CC      = gcc
PROF    =
NOCRYPT =
#C_FLAGS = -g -ggdb -Wall $(PROF) $(NOCRYPT)
C_FLAGS = -g -ggdb -g3 -Wall $(PROF) $(NOCRYPT)
L_FLAGS = $(PROF)
#L_FLAGS = $(PROF)
#SYSTEM = -DSYSV

O_FILES = reset.o edit.o act_comm.o act_info.o act_move.o act_obj.o act_wiz.o comm.o const.o \
          db.o fight.o handler.o interp.o magic.o save.o special.o update.o \
	  mob_prog.o mob_commands.o castle.o clans.o hashstr.o

../bin/md: $(O_FILES)
	rm -f ../bin/md
	$(CC) $(L_FLAGS) -o ../bin/md $(O_FILES)
.c.o: merc.h
	$(CC) -c $(C_FLAGS) $(SYSTEM) $<

clean:
	rm -f $(O_FILES)

