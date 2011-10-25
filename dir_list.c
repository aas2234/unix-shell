#include "dir_list.h"

/* create node with path string */
struct dir_node *create_node(char *path)
{
	struct dir_node *node = malloc(sizeof(struct dir_node));
	if (!node)
		return NULL;
	node->dir = malloc(strlen(path) + 1);
	if (!node->dir)
		return NULL;
	strcpy(node->dir, path);
	node->next = node->prev = NULL;
	return node;
}

void destroy_node(struct dir_node *node)
{
	if (node) {
		if (node->dir)
			free(node->dir);
		free(node);
	}
}

struct dir_list *create_dir_list(void)
{
	struct dir_list *list = malloc(sizeof(struct dir_list));
	if (!list)
		return NULL;
	list->head = list->tail = NULL;
	return list;
}

/* destroys complete list and all nodes in it */
void destroy_dir_list(struct dir_list *list)
{
	struct dir_node *tmp, *node;
	if (list) {
		if (list->head) {
			node = list->head;
			while (node) {
				tmp = node->next;
				destroy_node(node);
				node = tmp;
			}
		}
		free(list);
	}
}

/* append a node at list's tail */
int append_to_list_tail(struct dir_list *list, struct dir_node *node)
{
	if (list->tail) {
		list->tail->next = node;
		node->prev = list->tail;
		list->tail = list->tail->next;
	} else {
		list->head = list->tail = node;
	}
	return EXIT_SUCCESS;
}

/* resets links in the list appropriately and returns node */
struct dir_node *remove_node_from_list(struct dir_list *list,
					struct dir_node *node)
{
	if (list && node) {
		/* node in the middle of list */
		if (node->prev && node->next) {
			node->prev->next = node->next;
			node->next->prev = node->prev;
		/* node at tail of list */
		} else if (node->prev) {
			node->prev->next = NULL;
			list->tail = node->prev;
		/* node at head of list */
		} else if (node->next) {
			list->head = node->next;
			list->head->prev = NULL;
		} else {
			list->head = list->tail = NULL;
		}
		return node;
	} else
		return NULL;
}

struct dir_node *remove_from_list_tail(struct dir_list *list)
{
	return remove_node_from_list(list, list->tail);
}

/* find dir and destroy those nodes in list */
int remove_dir_from_list(struct dir_list *list, char *dir)
{
	struct dir_node *node = list->head, *tmp;
	int retval = 0;

	while (node) {
		if (!strcmp(dir, node->dir)) {
			retval = 1;
			remove_node_from_list(list, node);
			tmp = node->next;
			destroy_node(node);
			node = tmp;
		} else {
			node = node->next;
		}
	}
	return retval;
}

/* print list in reverse from tail to head */
void print_list_tail_to_head(struct dir_list *list)
{
	struct dir_node *node = list->tail;

	while (node) {
		fprintf(stdout, " %s", node->dir);
		node = node->prev;
	}
}
