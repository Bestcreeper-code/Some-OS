#pragma once

#define _gcc_kernel_address_space 1
#define _gcc_user_address_space 3

#define _GCC_ADDR_SPACE(N) __attribute__((address_space(N)))
#define __user _GCC_ADDR_SPACE(_gcc_user_address_space)