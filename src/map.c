#include "map.h"
#include <stdio.h>
#include <string.h>

void *_new(unsigned long size)
{
	void * p = malloc(size);
	memset(p, 0, size);
	return p;
}
RBNode * allocRBNode()
{
	RBNode * node = (RBNode*)_new(sizeof(RBNode));
	return node;
}

void deallocRBNode(RBNode * n)
{
	free(n);
}

Key ptrToKey(void *ptr)
{
	return (Key)ptr;
}
Value ptrToValue(void * ptr)
{
	return (Value)ptr;
}

void * keyToPtr(Key key)
{
	return (void*)key;
}

void * valueToPtr(Value value)
{
	return (void*)value;
}


bool checkFull(RBNode * node)
{
	if (!node || !node->left || !node->right) 
		return false;
	return true;
}

void reverseColor(RBNode * node) 
{
	if (!node) return;
	node->color ^= 1;
}

void swapColor(RBNode * a, RBNode * b)
{
	if (!a || !b) return;
	Color t = a->color;
	a->color = b->color;
	b->color = t;
}
void swapPair(RBNode *a, RBNode * b)
{
	if (!a || !b) return;
	Pair t = a->pair;
	a->pair = b->pair;
	b->pair = t;
}
void setChild(RBNode * parent, RBNode * child, Align pos)
{
	if (!parent || pos == NONE) return;

	if (child) 
		child->parent = parent;
	if (pos == LEFT) {
		parent->left = child;
	}else {
		parent->right = child;
	}
}

RBNode * getChild(RBNode * parent, Align pos)
{
	if (!parent || pos == NONE) return NULL;
	return pos == LEFT? parent->left : parent->right;
}

Color getColor(RBNode * node)
{
	if (!node) return BLACK;
	return node->color;
}


Align nodePosition(RBNode * node)
{
	if (!node || !node->parent) return 0;

	if (node == node->parent->left) {
		return LEFT;
	}
	return RIGHT;
}

RBNode * rotate(RBTree * tree, RBNode * node, Align align) 
{
	if (!tree || !getChild(node, -align)) {
		return NULL;
	}

	Align pos = nodePosition(node);
	RBNode * newRoot = getChild(node, -align);
	setChild(node, getChild(newRoot, align), -align);
	newRoot->parent = node->parent;
	setChild(newRoot, node, align);

	if (!pos) {
		tree->root = newRoot;
	}else {
		setChild(newRoot->parent, newRoot, pos);
	}
	return newRoot;
}

const Value * mapSearch(RBTree * tree,Key key) 
{
	if (!tree || !tree->root || !tree->_compare) return NULL;

	RBNode * cur = tree->root;
	while (cur) {
		int res = tree->_compare(tree->onwer, cur->pair.first, key);
		if (!res) {
			return &cur->pair.second;
		}else if (res > 0) {
			cur = cur->left;
		}else {
			cur = cur->right;
		}
	}
	return NULL;
}


RBNode ** ptrNode(RBTree * tree, RBNode * node)
{
	if (!tree || !tree->root || !node) {
		return NULL;
	}
	if (node == tree->root) {
		return &tree->root;
	}

	return nodePosition(node) == LEFT? &node->parent->left : &node->parent->right;
}

void mapInsert(RBTree * tree, Key key, Value value)
{
	if (!tree || !tree->_compare) return;

	if (!tree->root) {
		RBNode * root = allocRBNode();
		root->pair.first = key;
		root->pair.second = value;
		root->color = BLACK;	
		tree->root = root;
		return;
	}
	
	RBNode * parent = NULL;
	RBNode * cur = tree->root;
	Align posToInsert = NONE;

	while (cur) {
		parent = cur;
		int res = tree->_compare(tree->onwer,cur->pair.first, key);
		if (!res) {
			cur->pair.second = value;
			return;
		}else if (res > 0) {
			cur = cur->left;
			posToInsert = LEFT;
		}else {
			cur = cur->right;
			posToInsert = RIGHT;
		}
	}

	RBNode * me = allocRBNode();
	me->pair.first = key;
	me->pair.second = value;

	if (parent) {
		setChild(parent, me, posToInsert);
	}

	while (me->color == RED && parent->color == RED) {
		RBNode * grandParent = parent->parent;
		Align grandParentAlian = nodePosition(grandParent);
		Align parentAlian = nodePosition(parent);
		Align myAlign = nodePosition(me);

		RBNode * uncle = getChild(grandParent, -parentAlian);

		if (uncle && uncle->color == RED) {
			uncle->color = parent->color = BLACK;
			grandParent->color = RED;
			me = grandParent;	
		}else { 
			if (parentAlian != myAlign) {
				parent = rotate(tree, parent, parentAlian);
			}
			swapColor(grandParent, parent);
			me = rotate(tree, grandParent, -parentAlian);
		}
		parent = me->parent;
		if (!parent) {
			if (me->color == RED) {
				me->color = BLACK;
			}
			break;
		}
	}

}


RBNode * postNode(RBNode * cur)
{
	if (!cur) return NULL;

	RBNode * post = NULL;
	if (cur->right) { //没有右节点
		post = cur->right;
		while (post->left) post = post->left;
	}else {
		post = cur->parent;
		while (post && nodePosition(post) == RIGHT) {
			post = post->parent;
		}
	}
	return post;
}
void eraseNode(RBTree * tree, RBNode * n)
{
	if (!tree || !n) return;
	if (tree->root == n) {
		tree->root = NULL;
	}else {
		setChild(n->parent,NULL, nodePosition(n));
	}
	deallocRBNode(n);
}

void mapDelete(RBTree * tree, Key key)
{
	if (!tree || !tree->root || !tree->_compare) return;

	RBNode * cur = tree->root;
	while (cur) {
		int res = tree->_compare(tree->onwer, cur->pair.first, key);
		if (!res) break;
		else if (res > 0) cur = cur->left;
		else cur = cur->right;
	}

	//找到删除节点
	if (!cur) {
		return;
	}

	//找后继节点
	RBNode * replace = cur; 
	int re = 0;
	while (replace ) {
		if (!replace->left && !replace->right) {
			break;
		}else if (replace->left && replace->right) {
			replace = postNode(replace);
		}else {
			if (replace->left) {
				replace = replace->left;
			}else {
				replace = replace->right;
			}
		}
		re = replace->pair.second;
		swapPair(cur, replace);
		cur = replace;
	}

	//旋转调色
	//算法导论rb-delete-fixup
	RBNode * me = replace;
	while (me != tree->root && me->color == BLACK) {
		Align myPos = nodePosition(me);
		RBNode * parent = me->parent;
		RBNode * bro = getChild(parent,-myPos);
		if (bro->color == RED) { //兄弟节点为红色
			swapColor(bro, parent);
			rotate(tree, parent, myPos); 
		}else {//兄弟节点为黑色
			//远侄子
			RBNode * nf = getChild(bro, -myPos);
			//近侄子
			RBNode * nn = getChild(bro,  myPos);

			if (getColor(nf) == BLACK && getColor(nn) == BLACK) { //且侄子节点都是黑色
				bro->color = RED;
				me = parent;
			}else { 
				if (getColor(nf) == RED) {
					nf->color = BLACK;
					swapColor(bro, parent);
					rotate(tree, parent, myPos);
					me = tree->root;
				}else {
					swapColor(bro, nn);
					rotate(tree, bro, -myPos);
				}
			}
		}
	}
	me->color = BLACK;
	eraseNode(tree, replace);
}


int defaultCompare(void * owner, Key a, Key b)
{
	return a - b;
}
void defaultSave(void * owner,Pair * pair, Key key, Value value)
{
	pair->first = key;
	pair->second = value;
}
void defaultDelete(void * owner, Pair * pair)
{
}
RBTree * newRBTree(void *owner,Compare compare,Save save,Delete delete)
{
	RBTree * tree = (RBTree*)_new(sizeof(RBTree));
	tree->_compare = compare?compare:defaultCompare;
	tree->_save = save?save:defaultSave;
	tree->_delete = delete?delete:defaultDelete;
	return tree;
}

void recursiceDelete(RBTree * tree, RBNode * node)
{
	if (!tree || !node || !tree->_delete) return;
	recursiceDelete(tree, node->left);
	recursiceDelete(tree, node->right);
	tree->_delete(tree->onwer, &node->pair);
}
void deleteRBTree(RBTree * tree)
{
	if (!tree || !tree->_delete) return;
	if (tree->root) {
		recursiceDelete(tree, tree->root);
	}
}



void printRBNode(RBNode * cur)
{
	if (!cur) 
		printf(" [ | ]");
	else 
		printf(" [%s|%ld]",cur->color?"B":"R",cur->pair.second);
}


void printTree(RBTree * tree)
{
	if(!tree || !tree->root) return;

	int len = 1024;
	RBNode * buff[1024] = {0};
	int w = 0, r = 0;
	buff[w++%len] = tree->root;
	int last = r;
	while (w != r) {
		RBNode * cur = buff[r];
		if (cur ) {
			printRBNode(cur);

			printRBNode(cur->left);
			if (cur->left) {
				buff[w] = cur->left;
				if (last == r) {
					last = w;
				}
				w = (w+1)%len;
			}
			printRBNode(cur->right);
			if (cur->right) {
				buff[w] = cur->right;
				if (last == r) {
					last = w;
				}
				w = (w+1)%len;
			}

			printf("\n");
		}

		r = (r+1)%len;
	}

}
/*
int compare(void * owner, long a, long b)
{
	return strcmp((char*)a,(char*)b);
}

int main ()
{
	RBTree * tree = newRBTree(NULL,compare, NULL, NULL);
	struct {
		char * key;
		char * count;
	} tests[] = {
		{"abc","cde"},
		{"cbd","efg"},
		{"ddd","aa"},
		{"ttt","bbb"}
	};

	for (int i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
		mapInsert(tree, tests[i].key, tests[i].count);
	}
	printTree(tree);

	for (int i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
		Value * value = mapSearch(tree, tests[i].key);
		if (value) {
			printf("%s --> %s\n", tests[i].key, *value); 
		}
	}
	for (int i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
		mapDelete(tree, tests[i].key);
		printTree(tree);
		printf("\n");
	}

}
*/
