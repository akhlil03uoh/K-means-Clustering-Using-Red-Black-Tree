#include <stdio.h>
#include <stdlib.h>
#include <math.h>
struct Point {
    double x;
    double y;
    int cluster;
};

struct RBNode {
    struct Point point;
    int color; // 0 = RED, 1 = BLACK
    struct RBNode* left;
    struct RBNode* right;
    struct RBNode* parent;
};

int numPoints = 0;
int k;
struct Point points[3000];
struct Point centroids[3000];
struct RBNode* roots[3000] = {NULL};

// ---------- Red-Black Tree Functions ----------

struct RBNode* createNode(struct Point point) {
    struct RBNode* node = (struct RBNode*)malloc(sizeof(struct RBNode));
    node->point = point;
    node->color = 0; // RED
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    return node;
}

void leftRotate(struct RBNode** root, struct RBNode* x) {
    struct RBNode* y = x->right;
    x->right = y->left;
    if (y->left != NULL)
        y->left->parent = x;
    y->parent = x->parent;
    if (x->parent == NULL)
        *root = y;
    else if (x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;
    y->left = x;
    x->parent = y;
}

void rightRotate(struct RBNode** root, struct RBNode* y) {
    struct RBNode* x = y->left;
    y->left = x->right;
    if (x->right != NULL)
        x->right->parent = y;
    x->parent = y->parent;
    if (y->parent == NULL)
        *root = x;
    else if (y == y->parent->right)
        y->parent->right = x;
    else
        y->parent->left = x;
    x->right = y;
    y->parent = x;
}

void insertFixup(struct RBNode** root, struct RBNode* z) {
    while (z->parent != NULL && z->parent->color == 0) {
        if (z->parent == z->parent->parent->left) {
            struct RBNode* y = z->parent->parent->right;
            if (y != NULL && y->color == 0) {
                z->parent->color = 1;
                y->color = 1;
                z->parent->parent->color = 0;
                z = z->parent->parent;
            } else {
                if (z == z->parent->right) {
                    z = z->parent;
                    leftRotate(root, z);
                }
                z->parent->color = 1;
                z->parent->parent->color = 0;
                rightRotate(root, z->parent->parent);
            }
        } else {
            struct RBNode* y = z->parent->parent->left;
            if (y != NULL && y->color == 0) {
                z->parent->color = 1;
                y->color = 1;
                z->parent->parent->color = 0;
                z = z->parent->parent;
            } else {
                if (z == z->parent->left) {
                    z = z->parent;
                    rightRotate(root, z);
                }
                z->parent->color = 1;
                z->parent->parent->color = 0;
                leftRotate(root, z->parent->parent);
            }
        }
    }
    (*root)->color = 1;
}

void insert(struct RBNode** root, struct Point point) {
    struct RBNode* z = createNode(point);
    struct RBNode* y = NULL;
    struct RBNode* x = *root;

    while (x != NULL) {
        y = x;
        if (point.x < x->point.x)
            x = x->left;
        else
            x = x->right;
    }

    z->parent = y;
    if (y == NULL)
        *root = z;
    else if (point.x < y->point.x)
        y->left = z;
    else
        y->right = z;

    insertFixup(root, z);
}

struct RBNode* treeMinimum(struct RBNode* node) {
    while (node->left != NULL)
        node = node->left;
    return node;
}

void rbTransplant(struct RBNode** root, struct RBNode* u, struct RBNode* v) {
    if (u->parent == NULL)
        *root = v;
    else if (u == u->parent->left)
        u->parent->left = v;
    else
        u->parent->right = v;
    if (v != NULL)
        v->parent = u->parent;
}

void deleteFixup(struct RBNode** root, struct RBNode* x) {
    while (x != *root && (x == NULL || x->color == 1)) {
        if (x == x->parent->left) {
            struct RBNode* w = x->parent->right;
            if (w != NULL && w->color == 0) {
                w->color = 1;
                x->parent->color = 0;
                leftRotate(root, x->parent);
                w = x->parent->right;
            }
            if ((w->left == NULL || w->left->color == 1) && (w->right == NULL || w->right->color == 1)) {
                w->color = 0;
                x = x->parent;
            } else {
                if (w->right == NULL || w->right->color == 1) {
                    if (w->left != NULL)
                        w->left->color = 1;
                    w->color = 0;
                    rightRotate(root, w);
                    w = x->parent->right;
                }
                w->color = x->parent->color;
                x->parent->color = 1;
                if (w->right != NULL)
                    w->right->color = 1;
                leftRotate(root, x->parent);
                x = *root;
            }
        } else {
            struct RBNode* w = x->parent->left;
            if (w != NULL && w->color == 0) {
                w->color = 1;
                x->parent->color = 0;
                rightRotate(root, x->parent);
                w = x->parent->left;
            }
            if ((w->right == NULL || w->right->color == 1) && (w->left == NULL || w->left->color == 1)) {
                w->color = 0;
                x = x->parent;
            } else {
                if (w->left == NULL || w->left->color == 1) {
                    if (w->right != NULL)
                        w->right->color = 1;
                    w->color = 0;
                    leftRotate(root, w);
                    w = x->parent->left;
                }
                w->color = x->parent->color;
                x->parent->color = 1;
                if (w->left != NULL)
                    w->left->color = 1;
                rightRotate(root, x->parent);
                x = *root;
            }
        }
    }
    if (x != NULL)
        x->color = 1;
}

void deleteNode(struct RBNode** root, struct RBNode* z) {
    struct RBNode* y = z;
    struct RBNode* x;
    int yOriginalColor = y->color;

    if (z->left == NULL) {
        x = z->right;
        rbTransplant(root, z, z->right);
    } else if (z->right == NULL) {
        x = z->left;
        rbTransplant(root, z, z->left);
    } else {
        y = treeMinimum(z->right);
        yOriginalColor = y->color;
        x = y->right;
        if (y->parent == z) {
            if (x != NULL)
                x->parent = y;
        } else {
            rbTransplant(root, y, y->right);
            y->right = z->right;
            y->right->parent = y;
        }
        rbTransplant(root, z, y);
        y->left = z->left;
        y->left->parent = y;
        y->color = z->color;
    }

    if (yOriginalColor == 1)
        deleteFixup(root, x);
    free(z);
}

struct RBNode* search(struct RBNode* root, double x, double y) {
    while (root != NULL) {
        if (x < root->point.x)
            root = root->left;
        else if (x > root->point.x)
            root = root->right;
        else if (y == root->point.y)
            return root;
        else
            root = root->right;
    }
    return NULL;
}

void inorder(struct RBNode* root) {
    if (root != NULL) {
        inorder(root->left);
        printf("(%.2f, %.2f, %d)\n", root->point.x, root->point.y, root->color);
        inorder(root->right);
    }
}

// ---------- K-Means Functions ----------

double distance(struct Point a, struct Point b) {
    return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

void kMeans() {
    int i, j;
    for (i = 0; i < k; i++) {
        centroids[i] = points[i];
    }

    int changed;
    do {
        changed = 0;
        for (i = 0; i < numPoints; i++) {
            double minDist = distance(points[i], centroids[0]);
            int closest = 0;
            for (j = 1; j < k; j++) {
                double dist = distance(points[i], centroids[j]);
                if (dist < minDist) {
                    minDist = dist;
                    closest = j;
                }
            }
            if (points[i].cluster != closest) {
                points[i].cluster = closest;
                changed = 1;
            }
        }

        for (i = 0; i < k; i++) {
            double sumX = 0.0, sumY = 0.0;
            int count = 0;
            for (j = 0; j < numPoints; j++) {
                if (points[j].cluster == i) {
                    sumX += points[j].x;
                    sumY += points[j].y;
                    count++;
                }
            }
            if (count > 0) {
                centroids[i].x = sumX / count;
                centroids[i].y = sumY / count;
            }
        }
    } while (changed);

    for (i = 0; i < numPoints; i++) {
        insert(&roots[points[i].cluster], points[i]);
    }
}

// ---------- Main Function ----------

int main() {
    FILE* fp = fopen("points.txt", "r");
    if (fp == NULL) {
        printf("Error: Could not open points.txt\n");
        return 1;
    }

    int i = 0;
    while (fscanf(fp, "%lf %lf", &points[i].x, &points[i].y) == 2) {
        points[i].cluster = -1;
        i++;
        if (i >= 3000) break;
    }
    fclose(fp);

    numPoints = i;
    printf("Read %d points from points.txt\n", numPoints);

    printf("Enter the number of clusters (k ≤ %d): ", numPoints);
    scanf("%d", &k);

    if (k <= 0 || k > numPoints) {
        printf("Invalid number of clusters.\n");
        return 1;
    }

    kMeans();

    for (int j = 0; j < k; j++) {
        printf("\nCluster %d (inorder traversal):\n", j);
        inorder(roots[j]);
    }

    // Example deletion prompt
    printf("\nDelete a point: Enter cluster index, x, y: ");
    int ci;
    double dx, dy;
    if (scanf("%d %lf %lf", &ci, &dx, &dy) == 3 && ci >= 0 && ci < k) {
        struct RBNode* target = search(roots[ci], dx, dy);
        if (target != NULL) {
            deleteNode(&roots[ci], target);
            printf("\nAfter deletion:\n");
            inorder(roots[ci]);
        } else {
            printf("Point not found in the cluster.\n");
        }
    }

    return 0;
}
