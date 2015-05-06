OBJS_BMK+=	arch/i386/locore32.o arch/i386/machdep.o arch/i386/boot.o
OBJS_BMK+=	arch/i386/vgacons.o
OBJS_BMK+=	arch/i386/clock.o arch/i386/identcpu.o arch/i386/tsc.o \
	arch/i386/pvclock.o arch/i386/kvm.o

LDSCRIPT=	arch/i386/kern.ldscript

CFLAGS+=	-mno-sse -mno-mmx -march=i686

arch/i386/locore32.o: arch/i386/locore32.S
	${CC} ${CFLAGS} ${CPPFLAGS} -D_LOCORE -c -o arch/i386/locore32.o $<
