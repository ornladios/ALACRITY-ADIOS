#include <stdio.h>
#include <stdlib.h>

#define TRY(NAME)						\
    int _tc_el_##NAME = -1;				\
    const char *_tc_emsg_##NAME = NULL;

#define THROW(NAME, EL, MSG)	\
    _tc_el_##NAME = EL;			\
    _tc_emsg_##NAME = MSG;		\
    goto _tc_lbl_##NAME;

#define CATCH(NAME)						\
    _tc_lbl_##NAME:						\
    {									\
    const char *EMSG = _tc_emsg_##NAME;	\
    const int EL = _tc_el_##NAME;		\
    switch (EL)

#define IF_EL(N) case N

#define FINALLY

#define ENDTRYCATCH }

int main(int argc, char **argv) {
    TRY(bob) {
        printf("Do something 1\n");
        //THROW(bob, 1, "Something 1 failed");

        printf("Do something 2\n");
        //THROW(bob, 2, "Something 2 failed");

        printf("Do something 3\n");
        THROW(bob, 3, "Something 3 failed");

        printf("Do something 4\n");
        //THROW(bob, 4, "Something 4 failed");
    } CATCH(bob) {
        IF_EL(4): printf("Cleaned up for error level 4\n");
        IF_EL(3): printf("Cleaned up for error level 3\n");
        IF_EL(2): printf("Cleaned up for error level 2\n");
        IF_EL(1): printf("Cleaned up for error level 1\n");
    } FINALLY {
        printf("Finally\n");
        printf("The error message was %s, and the error level was %d\n", EMSG, EL);
    } ENDTRYCATCH;

    return 0;
}

