CC = gcc

CFLAGS = -c -Wall 

LDFLAGS = 

LIBS    = -lpthread

SOURCES = init.c \
	  session.c \
	  transport_l.c \
	  neighbor.c \


OBJECTS=$(SOURCES:.c=.o)

EXECUTABLE = receiver


	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@


all: $(SOURCES) $(EXECUTABLE)

install: all
	./receiver
clean: 
	rm -rf receiver *.o
