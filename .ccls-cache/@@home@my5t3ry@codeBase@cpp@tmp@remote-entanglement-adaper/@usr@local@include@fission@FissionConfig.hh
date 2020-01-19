#pragma once
#if !defined(__FISSION_CONFIG__HH__)
#define __FISSION_CONFIG__HH__

#if __has_cpp_attribute(deprecated)
	#define DEPRECATED(x) [[deprecated(x)]]
#else
	#define DEPRECATED(x) __attribute__ ((deprecated(x)))
#endif

/* == Fission Defines == */
#define FISSION_VERSION "0.1.0"
#define FISSION_BACKEND "X11GL3"

/* == Compiler Details == */
#define COMPILER_NAME "gcc"
#define COMPILER_VERSION "8.3.0"

/* == Internal / Meta stuff == */

#define __X11GL3 1
/* #undef __X11VULKAN */
/* #undef __WAYGL3 */
/* #undef __WAYVULKAN */

#endif /* __FISSION_CONFIG__HH__ */