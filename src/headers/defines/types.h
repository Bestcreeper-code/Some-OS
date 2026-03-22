#pragma once

#include <stdint.h>
typedef unsigned short		umode_t;
typedef long long  loff_t;
typedef long long  time64_t;
typedef long long  blkcnt_t;
typedef uint32_t   kuid_t;
typedef uint32_t   kgid_t;
typedef uint32_t   dev_t;


struct list_head {
	struct list_head *next, *prev;
};

struct hlist_head {
	struct hlist_node *first;
};

struct hlist_node {
	struct hlist_node *next, **pprev;
};

