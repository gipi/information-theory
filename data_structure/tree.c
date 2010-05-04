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
#include<stdlib.h>
#include<string.h>

#include<data_structure/tree.h>

node_t* node(void* data) {
	node_t* n = malloc(sizeof(node_t));
	if (n)
		n->data = data;
	n->left = NULL;
	n->right = NULL;
	
	return n;
}

node_t* node_and_memcpy(void* data, size_t size) {
	void* new_data = malloc(size);
	if (!new_data)
		return new_data;

	memcpy(new_data, data, size);

	return node(new_data);
}

/*
 * If we pass NULL as first argument, the function allocate a new
 * node. If we pass NULL for all the arguments then it allocates
 * a new empty node.
 */
node_t* node_append(node_t* parent, node_t* left, node_t* right) {
	if(!parent) {
		parent = malloc(sizeof(node_t));
		if (!parent) {
			return parent;
		}
	}

	if (left)
		parent->left = left;
	if (right)
		parent->right = right;

	return parent;
}

/*
 * Recursively call itself
 *
 * Return value: 0 on success, 1 on error.
 */
int tree_traverse(tree_t* root, node_callback_t callback) {
	int status;

	if (root->left)
		status  = tree_traverse(root->left, callback);

	if (root->right)
		status += tree_traverse(root->right, callback);

	/*
	 * this has to do for last in case the callback acts
	 * with the allocation of the parent.
	 */
	status = callback(root);

	return status;
}

static int free_callback(node_t* n) {
	free(n);

	return 0;
}

void tree_free(tree_t* t) {
	tree_traverse(t, free_callback);
}

#ifdef __TEST__
#include<stdio.h>

int sum = 0;

int my_callback(node_t* n) {
	sum += *(int*)n->data;

	return 0;
}

int main() {
	int data1 = 1;
	int data2 = 2;
	int data3 = 3;

	node_t* n1 = node(&data1);
	node_t* n2 = node(&data2);

	node_t* parent = node_append(NULL, n1, n2);
	parent->data = &data3;

	tree_traverse(parent, my_callback);

	printf("the total is %d\n", sum);

	tree_free(parent);

	return 0;
}
#endif
