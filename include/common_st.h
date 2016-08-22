/*
 * 提供list_head结构体结构及其操作函数
 * 这个结构体将起到连接所有本工程内结构体链表的作用
 */

#ifndef __COMMON_ST_H__
#define __COMMON_ST_H__

struct list_head{
	struct list_head *next;
	struct list_head *prev;
};

/* 宏操作，定义一个list_head头部 */
#define DECLARE_HEAD(head) \
	static struct list_head head = { \
		.next = &head, \
		.prev = &head, \
	}

#define LIST_ENTRY(list_head, type, member_name) \
	(type *)((unsigned int)list_head - (unsigned int)(&(((type*)(0))->member_name)))

/* from head to tail */
#define LIST_FOR_EACH_ENTRY_H(pos, head) \
	for(pos = (head)->next; pos != head; pos = pos->next)

/* from tail to head */
#define LIST_FOR_EACH_ENTRY_T(pos, head) \
	for(pos = (head)->prev; pos != head; pos = pos->prev)

void ListAddTail(struct list_head *new, struct list_head *head);
void ListDelTail(struct list_head *todel);

#endif
