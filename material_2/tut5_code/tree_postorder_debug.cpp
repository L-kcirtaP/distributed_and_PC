#include <omp.h>
#include <cstdlib>
#include <cstdio>
#include <time.h>
#include <math.h>

#define P 0.8

bool InsertCond()
{
    return float(rand()) / RAND_MAX < P;
}

struct Node
{
    int data;
    Node *left, *right;
};

void FillTree(int max_level, int level, Node *curr_node)
{
    if (level < max_level)
    {
        curr_node->left = new Node;
        curr_node->left->left = curr_node->left->right = NULL;

        if (InsertCond())
        {
            FillTree(max_level, level + 1, curr_node->left);
        }

        curr_node->right = new Node;
        curr_node->right->left = curr_node->right->right = NULL;

        if (InsertCond())
        {
            FillTree(max_level, level + 1, curr_node->right);
        }
    }
}

void Visit(Node *curr_node)
{
    /* do work here */
}

// Post-order traversal
int PostOrderTraverse(struct Node *curr_node)
{
    int left = 0;
    int right = 0;

    if (curr_node->left)
#pragma omp task shared(left)
        left = PostOrderTraverse(curr_node->left);

    /* Default attribute for task constructs is firstprivate.
     firstprivate = private but value is initialized with the value that
     the corresponding original item has when the construct is encountered.
     */

    if (curr_node->right)
#pragma omp task shared(right)
        right = PostOrderTraverse(curr_node->right);

#pragma omp taskwait
    curr_node->data = left + right; // Number of children nodes
    Visit(curr_node);

    return 1 + left + right;
}

int main()
{
    Node *root = new Node;

    int n_level = 4; // Maximum number of levels in the tree

    int level = 1;

    // Create a random tree

    int ntrial = 10000;
    float mean = 0.;

    srand(time(NULL));

    for (int t = 0; t < ntrial; ++t)
    {
        FillTree(n_level, level, root);
#pragma omp parallel
#pragma omp single // Only a single thread should execute this
        mean += float(PostOrderTraverse(root)) / ntrial;
    }

    printf("Number of nodes in the tree: %g\n", mean);
    printf("Expected number of nodes: %g\n", 2. * (pow(2. * P, n_level - 1) - 1) / (2. * P - 1.) + 1);
}
