#pragma once

#define _gcc_kernel_address_space 1
#define _gcc_user_address_space 3

#define _GCC_ADDR_SPACE(N) __attribute__((address_space(N)))
#define _GCC_SECTION(sec) __attribute__((section(sec)))
#define __user _GCC_ADDR_SPACE(_gcc_user_address_space)

#define __same_type(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))
