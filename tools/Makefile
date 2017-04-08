CC ?= "gcc"
OBJS = gr-power-burst-recorder.o
CFLAGS = -Wall

all: gr-power-burst-recorder

gr-power-burst-recorder: $(OBJS)
	@$(CC) $(OBJS) -o gr-power-burst-recorder

%.o: %.c
	@$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f gr-power-burst-recorder *.o
