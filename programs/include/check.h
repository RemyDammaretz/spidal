#ifndef CHECK_LIB
#define CHECK_LIB

#include <stdlib.h>
#include <stdio.h>

#define CHECK(sts,msg) if ((sts) == -1) { perror(msg); exit(EXIT_FAILURE); }

#define CHECK_T(sts,msg) if ((sts) != 0) { 			\
	fprintf(stderr, "pthread error : %s\n", msg); 	\
	exit(EXIT_FAILURE); 							\
}

#endif
