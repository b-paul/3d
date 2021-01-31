LIBS = -lxcb -lm

SOURCES = *.c

prog: ${SOURCES}
	${CC} ${SOURCES} -o $@ ${LIBS}
