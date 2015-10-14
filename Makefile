OBJS=src/mathexpr.o src/mt19937ar.o
LIBFILE=libmathexpr.a
CFLAGS=

.PHONY: all test clean

all: $(LIBFILE)

test: all
	$(CC) -o test test.c -L. -lmathexpr -lm
	./test

.c.o:
	$(CC) -c -o $@ $< $(CFLAGS)

$(LIBFILE): $(OBJS)
	$(AR) rc $(LIBFILE) $(OBJS)

clean:
	-rm $(OBJS) $(LIBFILE) test

