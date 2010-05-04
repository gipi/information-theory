#ifndef __TREE_H__
#define __TREE_H__
/*
 * Copyright 2010 Gianluca Pacchiella <gianluca.pacchiella@ktln2.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


/*
 * Implementation of Binary Tree.
 *
 *   1. the root node has not parent. There is almost
 *      one root node in a rooted tree.
 *   2. a leaf node has not children.
 */
#define node_is_leaf(n) (!(n).left && !(n).right)

/*
 * Note: this data structure contains only pointer
 *       so we can modify the content without have
 *       to pass a pointer to it.
 */
typedef struct _node_t{
	void* data;
	struct _node_t* left;
	struct _node_t* right;
}node_t;

typedef node_t tree_t;

node_t* node(void*);
node_t* node_and_memcpy(void*, size_t);
node_t* node_append(node_t* parent, node_t* left, node_t* right);
tree_t tree_init(void);

typedef int (*node_callback_t)(node_t* n, unsigned int depth);
int tree_traverse(tree_t*, unsigned int depth, node_callback_t);

void tree_free(tree_t* t);

#endif
