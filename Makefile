
include make.wasm

LIB=libabd.a
DIRS=string io data mpaland safe sort abd

.PHONY: all $(DIRS)

all: ${LIB}

${LIB}: ${DIRS}
	$(AR) rcs ${LIB} */*.o

$(DIRS):
	${MAKE} -C $@ $(MAKECMDGOALS)

clean:
	for a in ${DIRS}; do ${MAKE} -C $$a clean; done
	rm -f ${LIB}
