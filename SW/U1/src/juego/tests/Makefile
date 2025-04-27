SRCDIR  := ../../src
OBJDIR  := ../../obj
TESTDIR := ${SRCDIR}/tests
SOURCES := ${OBJDIR}/interfaz.o \
		   ${OBJDIR}/menu.o     \
		   ${OBJDIR}/juego.o    \
		   ${OBJDIR}/movimiento.o \
		   ${OBJDIR}/utils.o
INCDIRS := ${SRCDIR}
CC 		:= clang
CFLAGS 	:= -Wall -Werror -pedantic -std=c99 -I ${INCDIRS} -finput-charset=UTF-8
LDFLAGS := -lncurses

test_notacion: ${SOURCES} \
				${OBJDIR}/notacion.o \
				test_notacion.o

	${CC} ${LDFLAGS} $^ -o $@
	./test_notacion

${OBJDIR}/%.o : ${SRCDIR}/%.c
	${CC} -c ${CFLAGS} $^ -o $@

.PHONY: clean

clean:
	rm -f *.o ${APP}
