//============================================================================//
//                                                                            //
// Robust Geometric predicates                                                //
//                                                                            //
// The following routines are the robust geometric predicates for orientation //
// test and point-in-sphere test implemented by Jonathan Shewchuk.            //
// He generously provided the source code in the public domain,               //
// http://www.cs.cmu.edu/~quake/robust.html.                                  //
// predicates.cxx is a C++ version of the original C code.                    //
//                                                                            //
// The original predicates of Shewchuk only use "dynamic filters", i.e., it   //
// computes the error at runtime step by step. TetGen first uses a "static    //
// filter" in each predicate. It estimates the maximal possible error in all  //
// cases.  It safely and quickly "filters" many easy cases.                   //
//                                                                            //
//============================================================================//


#pragma once
#define REAL double         
void exactinit(int, int, int, REAL, REAL, REAL);

REAL orient3d(REAL* pa, REAL* pb, REAL* pc, REAL* pd);
REAL orient3dexact(REAL* pa, REAL* pb, REAL* pc, REAL* pd);

REAL insphere(REAL* pa, REAL* pb, REAL* pc, REAL* pd, REAL* pe);
REAL insphereexact(REAL* pa, REAL* pb, REAL* pc, REAL* pd, REAL* pe);
REAL orient4d(REAL* pa, REAL* pb, REAL* pc, REAL* pd, REAL* pe, REAL ah, REAL bh, REAL ch, REAL dh, REAL eh);

REAL orient2dexact(REAL* pa, REAL* pb, REAL* pc);
REAL orient3dexact(REAL* pa, REAL* pb, REAL* pc, REAL* pd);
REAL orient4dexact(REAL* pa, REAL* pb, REAL* pc, REAL* pd, REAL* pe, REAL ah, REAL bh, REAL ch, REAL dh, REAL eh);