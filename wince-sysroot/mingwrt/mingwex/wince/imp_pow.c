/*
 * mingwrt's complex/cpow.c calls pow through the old dllimport-pointer
 * convention: extern double (*_imp__pow)(double,double); #define pow
 * (*_imp__pow).  GNU dlltool emitted such pointers; llvm-dlltool emits
 * only __imp_-style ones.  Provide the pointer so cpow keeps working.
 */
#include <math.h>

double (*_imp__pow) (double, double) = pow;
