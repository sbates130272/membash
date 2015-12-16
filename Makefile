EXE=membash
CFLAGS += -std=gnu99 -O2 -g -Wall -Werror
SRC = ./src

default: $(EXE)

$(EXE): membash.c argconfig.o suffix.o report.o
	$(CC) $(CFLAGS) membash.c $(LDFLAGS) -o $(EXE) argconfig.o \
		suffix.o report.o

argconfig.o: $(SRC)/argconfig.c $(SRC)/argconfig.h $(SRC)/suffix.h
	$(CC) $(CFLAGS) -c $(SRC)/argconfig.c

report.o: $(SRC)/report.c $(SRC)/report.h $(SRC)/suffix.h
	$(CC) $(CFLAGS) -c $(SRC)/report.c

suffix.o: $(SRC)/suffix.c $(SRC)/suffix.h
	$(CC) $(CFLAGS) -c $(SRC)/suffix.c

clean:
	rm -f *~ *.o $(EXE)
