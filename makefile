CC      = mipsel-linux-gnu-gcc

INCLUDES = -I/home/devcontainers/net-yaroze/build-tools/INCLUDE -I/home/devcontainers/net-yaroze/elden_clone

CFLAGS  = \
    -O1 \
    -mips1 \
    -mabi=32 \
    -EL \
    -msoft-float \
    -ffreestanding \
    -fno-builtin \
	-mno-abicalls \
    -fno-pic \
    -G0 \
    -mno-gpopt \
    $(INCLUDES)

LDSCRIPT = /home/devcontainers/net-yaroze/build-tools/lib/LDSCRIPT/MIPSPSX.X

LDFLAGS = \
    -nostdlib \
    -L/home/devcontainers/net-yaroze/build-tools/lib \
    -Wl,-T,$(LDSCRIPT) \
    -Wl,-G,0

LIBS = -lps

PROG = main.elf

OBJS = main.o

$(PROG): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -f $(PROG) $(OBJS)