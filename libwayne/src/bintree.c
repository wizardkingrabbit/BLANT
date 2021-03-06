/* Version 0.0
** From "Wayne's Little DSA Library" (DSA == Data Structures and
** Algorithms) Feel free to change, modify, or burn these sources, but if
** you modify them please don't distribute your changes without a clear
** indication that you've done so.  If you think your change is spiffy,
** send it to me and maybe I'll include it in the next release.
**
** Wayne Hayes, wayne@cs.utoronto.ca (preffered), or wayne@cs.toronto.edu
*/

/* Binary tree algorithms from Lewis & Denenberg
*/
#include <string.h>
#include "bintree.h"
#include <math.h> // for logarithm

static foint CopyInt(foint i)
{
    return i;
}

static void FreeInt(foint i) {}

static int CmpInt(foint i, foint j) { return i.i - j.i; }


BINTREE *BinTreeAlloc(pCmpFcn cmpKey,
    pFointCopyFcn copyKey, pFointFreeFcn freeKey,
    pFointCopyFcn copyInfo, pFointFreeFcn freeInfo)
{
    BINTREE *tree = Malloc(sizeof(BINTREE));
    tree->root = NULL;
    tree->cmpKey = cmpKey ? cmpKey : CmpInt;
    tree->copyKey = copyKey ? copyKey : CopyInt;
    tree->freeKey = freeKey ? freeKey : FreeInt;
    tree->copyInfo = copyInfo ? copyInfo : CopyInt;
    tree->freeInfo = freeInfo ? freeInfo : FreeInt;
    tree->n = tree->depthSum = tree->depthSamples = 0;
    return tree;
}


static Boolean inRebalance;
static void BinTreeRebalance(BINTREE *tree);
void BinTreeInsert(BINTREE *tree, foint key, foint info)
{
    int depth = 0;
    BINTREENODE *p = tree->root, **locative = &(tree->root);
    while(p)
    {
	++depth;
	int cmp = tree->cmpKey(key, p->key);
	if(cmp == 0)
	{
	    tree->freeInfo(p->info);
	    p->info = tree->copyInfo(info);
	    return;
	}
	else if(cmp < 0)
	{
	    locative = &(p->left);
	    p = p->left;
	}
	else
	{
	    locative = &(p->right);
	    p = p->right;
	}
    }

    p = (BINTREENODE*) Calloc(1,sizeof(BINTREENODE));
    p->key = tree->copyKey(key);
    p->info = tree->copyInfo(info);
    p->left = p->right = NULL;
    *locative = p;
    tree->n++;

    tree->depthSum += depth; ++tree->depthSamples;
    double meanDepth = tree->depthSum/(double)tree->depthSamples;
    if(tree->n > 5 && tree->depthSamples > 20 && meanDepth > 3*log(tree->n) && !inRebalance) BinTreeRebalance(tree);
}


Boolean BinTreeDelete(BINTREE *tree, foint key)
{
    int depth = 0;
    BINTREENODE *p = tree->root, **locative = &(tree->root), *child;
    while(p)
    {
	++depth;
	int cmp = tree->cmpKey(key, p->key);
	if(cmp == 0)
	    break;
	else if(cmp < 0)
	{
	    locative = &(p->left);
	    p = p->left;
	}
	else
	{
	    locative = &(p->right);
	    p = p->right;
	}
    }
    if(!p) return false;

    // At this point, p points to the node we want to delete. If either child is NULL, then the other child moves up.

    child = NULL;
    if(p->left) child = p->left;
    else if(p->right) child = p->right;
    *locative = child;

    tree->freeInfo(p->info);
    tree->freeKey(p->key);
    Free(p);

    tree->n--;
    return true;
}


Boolean BinTreeLookup(BINTREE *tree, foint key, foint *pInfo)
{
    int depth=0;
    BINTREENODE *p = tree->root;
    while(p)
    {
	++depth;
	int cmp = tree->cmpKey(key, p->key);
	if(cmp == 0)
	{
	    if(pInfo) *pInfo = p->info;
	    return true;
	}
	if(cmp < 0)
	    p = p->left;
	else
	    p = p->right;
    }
    tree->depthSum += depth; ++tree->depthSamples;
    double meanDepth = tree->depthSum/(double)tree->depthSamples;
    if(tree->n > 5 && tree->depthSamples > 20 && meanDepth > 3*log(tree->n) && !inRebalance) BinTreeRebalance(tree);
    return false;
}

static Boolean BinTreeTraverseHelper ( BINTREENODE *p, pFointTraverseFcn f)
{
    Boolean cont = true;
    if(p) {
	if(p->left) cont = cont && BinTreeTraverseHelper(p->left, f);
	if(cont) cont = cont && f(p->key, p->info);
	if(cont && p->right) cont = cont && BinTreeTraverseHelper(p->right, f);
    }
    return cont;
}

Boolean BinTreeTraverse ( BINTREE *tree, pFointTraverseFcn f)
{
    return BinTreeTraverseHelper(tree->root, f);
}

#if 0
/* LookupKey: the tree is ordered on key, not info.  So we have to do a
** full traversal, O(size of tree), not O(log(size of tree)).  This is
** only used when errors occur.
*/
foint BinTreeLookupKey(BINTREE *tree, foint info)
{
    BINTREENODE *p = tree->root;
    if(p)
    {
	if(tree->cmpKey(p->info, info) == 0)
	    return p->key;
	else
	{
	    foint key = BinTreeLookupKey(p->left, info);
	    if(key)
		return key;
	    else
		return BinTreeLookupKey(p->right, info);
	}
    }
    return NULL;
}
#endif


static void BinTreeNodeFree(BINTREE *tree, BINTREENODE *t)
{
    if(t)
    {
	BinTreeNodeFree(tree, t->left);
	BinTreeNodeFree(tree, t->right);
	tree->freeKey(t->key);
	tree->freeInfo(t->info);
	free(t);
    }
}

void BinTreeFree(BINTREE *tree)
{
    BinTreeNodeFree(tree, tree->root);
    free(tree);
}


//////////////////// REBALANCING CODE
static foint *keyArray, *dataArray;
static int arraySize, currentItem;

// Squirrel away all the item *in sorted order*
static Boolean TraverseTreeToArray(foint key, foint data) {
    assert(currentItem < arraySize);
    keyArray[currentItem] = key;
    dataArray[currentItem] = data;
    ++currentItem;
    return true;
}

static void BinTreeInsertMiddleElementOfArray(BINTREE *tree, int low, int high) // low to high inclusive
{
    if(low <= high) { // we're using low though high *inclusive* so = is a valid case.
	int mid = (low+high)/2;
	BinTreeInsert(tree, keyArray[mid], dataArray[mid]);
	BinTreeInsertMiddleElementOfArray(tree, low, mid-1);
	BinTreeInsertMiddleElementOfArray(tree, mid+1,high);
    }
}

static void BinTreeRebalance(BINTREE *tree)
{
    //Warning("inRebalance tree %x size %d mean depth %g", tree, tree->n, tree->depthSum/(double)tree->depthSamples);
    assert(!inRebalance);
    inRebalance = true;
    if(tree->n > arraySize){
	arraySize = tree->n;
	keyArray = Realloc(keyArray, arraySize*sizeof(foint));
	dataArray = Realloc(dataArray, arraySize*sizeof(foint));
    }
    currentItem = 0;
    BinTreeTraverse (tree, TraverseTreeToArray);
    assert(currentItem == tree->n);
    
    BINTREE *newTree = BinTreeAlloc(tree->cmpKey , tree->copyKey , tree->freeKey , tree->copyInfo , tree->freeInfo);
    // Now re-insert the items in *perfectly balanced* order.
    BinTreeInsertMiddleElementOfArray(newTree, 0, tree->n - 1);
    assert(tree->n == newTree->n);
    // Swap the roots, record new depth.
    BINTREENODE *tmp = tree->root; tree->root = newTree->root; newTree->root = tmp;
    BinTreeFree(newTree);
    inRebalance = false;
}
