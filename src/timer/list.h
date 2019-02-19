#ifndef _LIST_H_
#define _LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * struct list - Doubly-linked list
 */
struct list {
	struct list *next;
	struct list *prev;
};

static inline void list_init(struct list *list)
{
	list->next = list;
	list->prev = list;
}

static inline void list_add(struct list *list, struct list *item)
{
	item->next = list->next;
	item->prev = list;
	list->next->prev = item;
	list->next = item;
}

static inline void list_add_tail(struct list *list, struct list *item)
{
	list_add(list->prev, item);
}

static inline void list_del(struct list *item)
{
	item->next->prev = item->prev;
	item->prev->next = item->next;
	item->next = NULL;
	item->prev = NULL;
}

static inline int list_empty(struct list *list)
{
	return (list->next == list);
}

static inline unsigned int list_len(struct list *list)
{
	struct list *item;
	int count = 0;
	for (item = list->next; item != list; item = item->next)
		count++;
	return count;
}

#ifndef offsetof
#define offsetof(type, member) ((long) &((type *) 0)->member)
#endif

#define list_entry(item, type, member) \
	((type *) ((char *) item - offsetof(type, member)))

#define list_first(list, type, member) \
	(list_empty((list)) ? NULL : \
	 list_entry((list)->next, type, member))

#define list_for_each(item, list, type, member) \
	for (item = list_entry((list)->next, type, member); \
	     &item->member != (list); \
	     item = list_entry(item->member.next, type, member))

#define list_for_each_safe(item, n, list, type, member) \
	for (item = list_entry((list)->next, type, member), \
		     n = list_entry(item->member.next, type, member); \
	     &item->member != (list); \
	     item = n, n = list_entry(n->member.next, type, member))

#ifdef __cplusplus
}
#endif

#endif /* _LIST_H_ */
