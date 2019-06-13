#ifndef _cPrimitiveTypes_h_
#define _cPrimitiveTypes_h_

// boolean
typedef bool Bool;

// integer types
typedef int Int;
typedef char Int8;
typedef short Int16;
typedef long Int32;
#if defined(_MSC_VER)
	typedef __int64 Int64;
#else
	typedef long long Int64;
#endif

typedef signed int SInt;
typedef signed char SInt8;
typedef signed short SInt16;
typedef signed long SInt32;
#if defined(_MSC_VER)
	typedef signed __int64 SInt64;
#else
	typedef signed long long SInt64;
#endif

typedef unsigned int UInt;
typedef unsigned char UInt8;
typedef unsigned short UInt16;
typedef unsigned long UInt32;
#if defined(_MSC_VER)
	typedef unsigned __int64 UInt64;
#else
	typedef unsigned long long UInt64;
#endif

// characters
typedef char Char8;
#if (defined(_MSC_VER) && !defined(_WCHAR_T_DEFINED)) || defined(__ARMCC_VERSION)
	typedef unsigned short Char16;
#else
	typedef wchar_t Char16;
#endif
#if defined(UNICODE)
	typedef Char16 Char;
#else
	typedef Char8 Char;
#endif

// floating point types
typedef double Float;
typedef float Float32;
typedef double Float64;

// null
#if !defined(null)
	#define null (0)
#endif

// value ranges
#define isSignedIntegerType(Type) (!((Type)~(Type)0 > 0))
#define maximumOfIntegerType(Type) (isSignedIntegerType(Type) ? (Type)((((Type)1 << (sizeof(Type) * 8 - 2)) - 1) * 2 + 1) : (Type)~(Type)0)
#define minimumOfIntegerType(Type) (isSignedIntegerType(Type) ? (Type)((Type)-1 << (sizeof(Type) * 8 - 1)) : (Type)0)
const SInt8 maxSInt8 = maximumOfIntegerType(SInt8);
const SInt16 maxSInt16 = maximumOfIntegerType(SInt16);
const SInt32 maxSInt32 = maximumOfIntegerType(SInt32);
const SInt64 maxSInt64 = maximumOfIntegerType(SInt64);
const SInt maxSInt = maximumOfIntegerType(SInt);
const SInt8 minSInt8 = minimumOfIntegerType(SInt8);
const SInt16 minSInt16 = minimumOfIntegerType(SInt16);
const SInt32 minSInt32 = minimumOfIntegerType(SInt32);
const SInt64 minSInt64 = minimumOfIntegerType(SInt64);
const SInt minSInt = minimumOfIntegerType(SInt);
const UInt8 maxUInt8 = maximumOfIntegerType(UInt8);
const UInt16 maxUInt16 = maximumOfIntegerType(UInt16);
const UInt32 maxUInt32 = maximumOfIntegerType(UInt32);
const UInt64 maxUInt64 = maximumOfIntegerType(UInt64);
const UInt maxUInt = maximumOfIntegerType(UInt);
const UInt8 minUInt8 = minimumOfIntegerType(UInt8);
const UInt16 minUInt16 = minimumOfIntegerType(UInt16);
const UInt32 minUInt32 = minimumOfIntegerType(UInt32);
const UInt64 minUInt64 = minimumOfIntegerType(UInt64);
const UInt minUInt = minimumOfIntegerType(UInt);
const UInt8 maxInt8 = maximumOfIntegerType(UInt8);
const Int16 maxInt16 = maximumOfIntegerType(Int16);
const Int32 maxInt32 = maximumOfIntegerType(Int32);
const Int64 maxInt64 = maximumOfIntegerType(Int64);
const Int maxInt = maximumOfIntegerType(Int);
const Int8 minInt8 = minimumOfIntegerType(Int8);
const Int16 minInt16 = minimumOfIntegerType(Int16);
const Int32 minInt32 = minimumOfIntegerType(Int32);
const Int64 minInt64 = minimumOfIntegerType(Int64);
const Int minInt = minimumOfIntegerType(Int);

// arrays
#define arrayDimension(array) (sizeof(array) / sizeof(*(array)))

#define minimum(a, b) ((a) < (b) ? (a) : (b))
#define maximum(a, b) ((a) > (b) ? (a) : (b))

#endif // _cPrimitiveTypes_h_
