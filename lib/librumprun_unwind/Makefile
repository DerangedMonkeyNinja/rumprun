.include <bsd.own.mk>

RUMPTOP= ${TOPRUMP}

LIB=	rumprun_unwind

CPPFLAGS+= -isystem ${DESTDIR}/usr/include/c++

LIBC_MACHINE_CPU?= ${MACHINE_CPU}
.include "${RUMPTOP}/../lib/libunwind/Makefile.inc"

.include <bsd.lib.mk>
