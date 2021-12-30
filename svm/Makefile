PROG= svm
SRCS= svm.c readn.c
CFLAGS+= -I$(.CURDIR)/../lib
LDFLAGS+= -L${.CURDIR}/../lib/obj
LDADD= -lsvm

.include <bsd.prog.mk>
