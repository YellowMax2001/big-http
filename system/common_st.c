#include <common_st.h>
#include <stdlib.h>

/* 添加一个尾巴，参数是相对于新添加的尾巴来定义的 */
static void __ListAddTail(struct list_head *new, struct list_head *prev, struct list_head *next)
{
	new->next = next;
	new->prev = prev;
	prev->next = new;
	next->prev = new;
}

void ListAddTail(struct list_head *new, struct list_head *head)
{
	__ListAddTail(new, head->prev, head);
}

void ListDelTail(struct list_head *todel)
{
	todel->next->prev = todel->prev;
	todel->prev->next = todel->next;
}


