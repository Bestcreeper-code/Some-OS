#pragma once



struct list_head {
	struct list_head *next, *prev;
};


struct hlist_head {
	struct hlist_node *first;
};

struct hlist_node {
	struct hlist_node *next, **pprev;
};

#define hlist_for_each(pos, head) \
	for (pos = (head)->first; pos ; pos = pos->next)



static inline void hlist_add_head(struct hlist_node *node, struct hlist_head *head)
{
	struct hlist_node *first = head->first;
	node->next = first;
	if (first)
		first->pprev = &node->next;
	head->first = node;
	node->pprev = &head->first;
    
}