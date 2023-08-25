#
# $Id: Makefile 30886 2020-02-06 19:46:30Z rodrigo.barboza $
#
include ../make.rules

OBJS=musl_sort.o

#SVNREV := -D'SVNREV="$(shell svnversion -n .)"'
#CFLAGS+=-I. -I${RELEASE_DIR}/include $(SVNREV)
CFLAGS+=-I. -D'SVNREV="devel"'

LIB=libsort.a
#LDLIBS+=-L.

#ifeq ($(WASM),yes)
#	CFLAGS+=--target=wasm32-unknown-wasi --sysroot /wexport4/vol_safe/wasi-libc --no-standard-libraries -O2
#	AR=llvm-ar
#	CC=clang
#else
#	CFLAGS+=	-std=c++11 
#endif

#CFLAGS+=--target=wasm32-unknown-wasi --sysroot /wexport4/vol_safe/wasi-libc -nostartfiles --no-standard-libraries -Wl,--export-all -Wl,--no-entry -O2

#From Mpaland original Makefile
CFLAGS+=	-Wall                             \
                -pedantic                         \
                -Wmain                            \
                -Wundef                           \
                -Wuninitialized                   \
                -Wshadow                          \
                -Wunreachable-code                \
                -Wswitch-default                  \
                -Wswitch                          \
                -Wcast-align                      \
                -Wmissing-include-dirs            \
                -Winit-self                       \
                -Wdouble-promotion                \
                -fno-exceptions                   \
                -O2                               \
                -ffunction-sections               \
                -fdata-sections                   \
                -fverbose-asm                     \
                -Wextra                           \
                -Wunused-parameter                \
                -Wfloat-equal
#
# To silence a known warning
CFLAGS+=        -Wno-sign-compare

all:: ${LIB}

$(OBJS64): %.o: %.c
	$(CC) -c ${CFLAGS} ${FLAG64} $< -o $@

$(LIB): ${OBJS}
	${AR} rcs ${LIB} ${OBJS}

devpack: all
	echo '#!/bin/sh' > install_devpack.sh
	echo 'cp '"$(PROGS)"' /usr/local/bin' >> install_devpack.sh
	echo 'cp '"$(LIB)"' /usr/local/lib' >> install_devpack.sh
	tar -czf devpack.tgz $(PROGS) $(LIB) install_devpack.sh
	${RM} install_devpack.sh

install::
	@echo ""
	@echo "Use make release, e não make install"
	@echo ""
	@exit 1

updaterelease:
	cd ${RELEASE_DIR} && ./update

release:: all
	${INSTALLDIR}             ${RELEASE_DIR}/lib
	${INSTALLDIR}             ${RELEASE_DIR}/include/ht
	${INSTALLLIB} ${LIB}      ${RELEASE_DIR}/lib
	${INSTALLLIB} ht/*.h   	  ${RELEASE_DIR}/include/ht

commit: updaterelease clean release
	cd ${RELEASE_DIR}; for f in ${LIB} ; do cvs commit -m '' lib/$$f; done
	cd ${RELEASE_DIR}; for f in ht/*.h ;  do cvs commit -m '' include/$$f; done

clean::
	rm -f *.o *.a *.class ${LIB} install_devpack.sh devpack.tgz

# EOF
