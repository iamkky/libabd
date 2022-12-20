
CFLAGS+=	-I..
OBJS=printf.o

#From Mpaland original Makefile
CFLAGS+=	-Wall                             \
                -pedantic                         \
                -Wmain                            \
                -Wundef                           \
                -Wsign-conversion                 \
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
# Removed, not supported by clang
#                -ffat-lto-objects                 \

# To silence a known warning
CFLAGS+=	-Wno-sign-conversion

include ../make.rules
