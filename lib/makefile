TARGET	= libsiofs.lib

CFILES	= siofs.c comm.c

CFLAGS	= -O2

OFILES	= $(CFILES:.c=.obj)
CC		= ccpsx

all: $(OFILES)
	psylib2 /a $(TARGET) $(OFILES)

clean:
	rm -f $(OFILES) $(TARGET)

%.obj: %.c
	$(CC) $(CFLAGS) -c $< -o $@
