// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria, Aaron Giles
/***************************************************************************

    emucore.h

    General core utilities and macros used throughout the emulator.
***************************************************************************/

#ifndef MAME_EMU_EMUCORE_H
#define MAME_EMU_EMUCORE_H

#pragma once

// standard C includes
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>

// some cleanups for Solaris for things defined in stdlib.h
#if defined(__sun__) && defined(__svr4__)
#undef si_status
#undef WWORD
#endif

// standard C++ includes
#include <cassert>
#include <exception>
#include <type_traits>
#include <typeinfo>

// core system includes
#include "common.h"


//**************************************************************************
//  COMPILER-SPECIFIC NASTINESS
//**************************************************************************

// Suppress warnings about redefining the macro 'PPC' on LinuxPPC.
#undef PPC

// Suppress warnings about redefining the macro 'ARM' on ARM.
#undef ARM

//**************************************************************************
//  USEFUL COMPOSITE TYPES
//**************************************************************************

// PAIR is an endian-safe union useful for representing 32-bit CPU registers
union PAIR
{
#ifdef LSB_FIRST
	struct { u8 l,h,h2,h3; } b;
	struct { u16 l,h; } w;
	struct { s8 l,h,h2,h3; } sb;
	struct { s16 l,h; } sw;
#else
	struct { u8 h3,h2,h,l; } b;
	struct { s8 h3,h2,h,l; } sb;
	struct { u16 h,l; } w;
	struct { s16 h,l; } sw;
#endif
	u32 d;
	s32 sd;
};

// PAIR16 is a 16-bit extension of a PAIR
union PAIR16
{
#ifdef LSB_FIRST
	struct { u8 l,h; } b;
	struct { s8 l,h; } sb;
#else
	struct { u8 h,l; } b;
	struct { s8 h,l; } sb;
#endif
	u16 w;
	s16 sw;
};


// PAIR64 is a 64-bit extension of a PAIR
union PAIR64
{
#ifdef LSB_FIRST
	struct { u8 l,h,h2,h3,h4,h5,h6,h7; } b;
	struct { u16 l,h,h2,h3; } w;
	struct { u32 l,h; } d;
	struct { s8 l,h,h2,h3,h4,h5,h6,h7; } sb;
	struct { s16 l,h,h2,h3; } sw;
	struct { s32 l,h; } sd;
#else
	struct { u8 h7,h6,h5,h4,h3,h2,h,l; } b;
	struct { u16 h3,h2,h,l; } w;
	struct { u32 h,l; } d;
	struct { s8 h7,h6,h5,h4,h3,h2,h,l; } sb;
	struct { s16 h3,h2,h,l; } sw;
	struct { s32 h,l; } sd;
#endif
	u64 q;
	s64 sq;
};



//**************************************************************************
//  COMMON CONSTANTS
//**************************************************************************

// constants for expression endianness
enum endianness_t
{
	ENDIANNESS_LITTLE,
	ENDIANNESS_BIG
};


// declare native endianness to be one or the other
#ifdef LSB_FIRST
const endianness_t ENDIANNESS_NATIVE = ENDIANNESS_LITTLE;
#else
const endianness_t ENDIANNESS_NATIVE = ENDIANNESS_BIG;
#endif

#endif //MAME_EMU_EMUCORE_H