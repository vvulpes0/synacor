PROG= svm
SRCS= svm.c readn.c
CFLAGS+= -I$(.CURDIR)/../lib
# do our best to find the built libsvm
LDFLAGS+= -L${.CURDIR}/../lib -L${.CURDIR}/../lib/obj
LDADD= -lsvm

.include <bsd.prog.mk>
