#pragma once

#define _gcc_kernel_address_space 1
#define _gcc_user_address_space 3

#define GCC_ATTR __attribute__

#define _GCC_ADDR_SPACE(N) GCC_ATTR((address_space(N)))
#define _GCC_SECTION(sec) GCC_ATTR((section(sec)))
#define __user _GCC_ADDR_SPACE(_gcc_user_address_space)

#define __same_type(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))
