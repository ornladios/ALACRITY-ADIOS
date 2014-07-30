/*
 * trycatch.h
 *
 * A set of macros that emulate a limited try-catch block in C.
 *
 * Each try-catch block has a name (label), assigned in the
 * TRY(name) macro. The name must be a valid C identifier.
 * This name must be unique across the compilation
 * unit (i.e., source file). The name must be matched in any
 * THROW(name, el, msg) and CATCH(name) macros, as well.
 *
 * Try-catch blocks can be (properly) nested,
 * and "exceptions" can be thrown completely out of the current block
 * (although that's not usually advisable).
 *
 * "Exceptions" can only be thrown within the scope that the TRY block
 * was declared (or any subscope). Attempting to do otherwise has
 * undefined results (it could get ugly).
 *
 * A try-catch block must be formatted exactly as follows (whitespace
 * is not important):
 *
 * TRY(<name>) {
 *   ... code ...
 *   THROW(<name>, <positive integer>, "message")
 *   ... code ...
 *   THROW(<name>, <positive integer>, "another message")
 *   ... code ...
 *   ...
 * } CATCH(<name>) {
 *   IF_EL(<positive integer>):
 *   ... code ...
 *   IF_EL(<positive integer>):
 *   ... code ...
 *   ...
 * } ENDTRYCATCH
 *
 * IF_EL statements in the CATCH clause map to case: statements in a
 * switch-case, and so share the fall-through semantic, unless a break
 * statement is inserted. This is intentional, to allow progressive
 * cleanup.
 *
 * The typical use case is to number each successive THROW statement
 * with the next positive integer, starting at 1, and list the IF_EL
 * statements counting down in reverse order, each cleaning up
 * resources unique to the interval between the corresponding
 * throw statement and the one before that.
 *
 *  Created on: Nov 14, 2012
 *      Author: David A. Boyuka II
 */

#ifndef TRYCATCH_H_
#define TRYCATCH_H_


#define TRY(NAME)                       \
    int _tc_el_##NAME = -1;             \
    char _tc_emsg_##NAME[BUFSIZ];

#define THROW(NAME, EL, ...) {             \
    _tc_el_##NAME = EL;                    \
    sprintf(_tc_emsg_##NAME, __VA_ARGS__); \
    goto _tc_lbl_##NAME;                   \
    }

#define CATCH(NAME)                      \
    _tc_lbl_##NAME:                      \
    {                                    \
    const char *EMSG = _tc_emsg_##NAME;  \
    const int EL = _tc_el_##NAME;        \
    switch (EL)

#define IF_EL(N) case N

#define ENDTRYCATCH }


#endif /* TRYCATCH_H_ */
