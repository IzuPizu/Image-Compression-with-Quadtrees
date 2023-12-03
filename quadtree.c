/* URECHE Andreea-Maria - 312CC*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "helpers.h"

/*prototypes*/
QuadTreeNode *createQuadTree(Pixel **img, unsigned int size,
                             unsigned int factor, int x, int y);
int mean_score(int x, int y, int size, Pixel **img, unsigned int factor,
               unsigned int *red_avg, unsigned int *blue_avg,
               unsigned int *green_avg);
int countLevels(QuadTreeNode *node, int level);
int findMax(int a, int b);
int getLeafCount(QuadTreeNode *node);
int findclosestleaf(QuadTreeNode *root);
void printLevelOrder(QuadTreeNode *root, FILE *fptrout);
void printCurrentLevel(QuadTreeNode *root, int level, FILE *fptrout);
QuadTreeNode *ReCreateQuadTree(FILE *fptrin, int size);
void set_size_leaf(QuadTreeNode *node, int size);
void build_image(QuadTreeNode *node, Pixel **image, int x, int y, int size);
void freeMatrix(Pixel **img, int height, int width);
void freeQuadTree(QuadTreeNode *root);

int main(int argc, char *argv[]) {
    /*  Usage:
      ./quadtree [-c1 factor|-c2 factor|-d] [input_file] [output_file] */
    unsigned int factor;
    int i, j;
    int flag_c1 = 0, flag_c2 = 0, flag_d = 0;
    FILE *fptrin = NULL;
    FILE *fptrout = NULL;

    /*parse arguments & open files*/
    if (argv[1] != NULL && strcmp(argv[1], "-c1") == 0) {
        flag_c1 = 1;
        factor = atoi(argv[2]);
        fptrin = fopen(argv[3], "rb");
        fptrout = fopen(argv[4], "w");
    }
    if (argv[1] !=NULL && strcmp(argv[1], "-c2") == 0) {
        flag_c2 = 1;
        factor = atoi(argv[2]);
        fptrin = fopen(argv[3], "rb");
        fptrout = fopen(argv[4], "w");
    }
    if (argv[1] !=NULL && strcmp(argv[1], "-d") == 0) {
        flag_d = 1;
        fptrin = fopen(argv[2], "rb");
        fptrout = fopen(argv[3], "w");
    }

    if (fptrin == NULL) {
        exit(1);
    }

    if (fptrout == NULL) {
        exit(1);
    }

    if (flag_d != 1) {
        /*read header from ppm file*/
        char type[3];
        unsigned int width, height;
        unsigned int maxval;
        fscanf(fptrin, "%s\n", type);
        fscanf(fptrin, "%d %d\n", &width, &height);
        fscanf(fptrin, "%d", &maxval);
        fgetc(fptrin);

        /*allocate space for matrix of pixels*/
        Pixel **img = (Pixel **)malloc(height * sizeof(Pixel *));
        for (i = 0; i < height; i++) {
            img[i] = (Pixel *)malloc(width * sizeof(Pixel));
        }

        /*read image from ppm file*/
        for (i = 0; i < height; i++) {
            for (j = 0; j < width; j++) {
                fread(&img[i][j].rgbtRed, sizeof(unsigned char), 1, fptrin);
                fread(&img[i][j].rgbtGreen, sizeof(unsigned char), 1, fptrin);
                fread(&img[i][j].rgbtBlue, sizeof(unsigned char), 1, fptrin);
            }
        }

        /*implement QuadTree*/
        QuadTreeNode *root;  // root -> whole image
        unsigned int size = height;
        /* set (x,y) to start of matrix (0,0) */
        int x = 0, y = 0;
        root = createQuadTree(img, size, factor, x, y);

        /* count number of leafs */
        unsigned int leaf_cnt = getLeafCount(root);
        /*count number of levels */
        unsigned int levels = countLevels(root, 0);
        /*find largest undivided block  <-> closest leaf to root */
        unsigned int size_largest = findclosestleaf(root);

        // task1
        if (flag_c1 == 1) {
            fprintf(fptrout, "%d\n", levels);
            fprintf(fptrout, "%d\n", leaf_cnt);
            fprintf(fptrout, "%d\n", size_largest);
        }
        // task2
        if (flag_c2 == 1) {
            fwrite(&size, sizeof(unsigned int), 1, fptrout);
            printLevelOrder(root, fptrout);
        }

        // free the memory
        freeQuadTree(root);
        freeMatrix(img, height, width);
    }

    // task3
    if (flag_d == 1) {
        int i, j;
        // read compressed image from file
        unsigned int dimension;
        fread(&dimension, sizeof(unsigned int), 1, fptrin);
        // create QuadTree based on compressed file
        fprintf(fptrout, "P6\n%d %d\n%d\n", dimension, dimension, 255);
        QuadTreeNode *root;
        root = ReCreateQuadTree(fptrin, dimension);

        set_size_leaf(root, dimension);

        // create matrix of pixels
        Pixel **image = (Pixel **)malloc(dimension * sizeof(Pixel *));
        for (i = 0; i < dimension; i++) {
            image[i] = (Pixel *)malloc(dimension * sizeof(Pixel));
        }

        // fill the matrix
        build_image(root, image, 0, 0, dimension);

        // write the image
        for (i = 0; i < dimension; i++) {
            for (j = 0; j < dimension; j++) {
                fwrite(&image[i][j].rgbtRed, sizeof(unsigned char), 1, fptrout);
                fwrite(&image[i][j].rgbtGreen, sizeof(unsigned char), 1,
                       fptrout);
                fwrite(&image[i][j].rgbtBlue, sizeof(unsigned char), 1,
                       fptrout);
            }
        }

        // free the memory
        freeMatrix(image, dimension, dimension);
        freeQuadTree(root);
        // close files
        fclose(fptrin);
        fclose(fptrout);
    }
}

QuadTreeNode *createQuadTree(Pixel **img, unsigned int size,
                             unsigned int factor, int x, int y) {
    unsigned int red_avg, blue_avg, green_avg;
    // allocate newnode
    QuadTreeNode *newnode = (QuadTreeNode *)malloc(sizeof(QuadTreeNode));
    // initialize children
    newnode->child1 = NULL;
    newnode->child2 = NULL;
    newnode->child3 = NULL;
    newnode->child4 = NULL;

    /*check if node is leaf < - > base condition to stop recursion*/
    if (mean_score(x, y, size, img, factor, &red_avg, &blue_avg, &green_avg) ==
        0) {
        newnode->is_leaf = 1;
        newnode->size_leaf = size;
        newnode->color.rgbtBlue = blue_avg;
        newnode->color.rgbtGreen = green_avg;
        newnode->color.rgbtRed = red_avg;

        return newnode;
    }
    /*node not leaf => divide the matrix */
    size = size / 2;
    newnode->is_leaf = 0;
    newnode->child1 = createQuadTree(img, size, factor, x, y);
    newnode->child2 = createQuadTree(img, size, factor, x, size + y);
    newnode->child3 = createQuadTree(img, size, factor, x + size, y + size);
    newnode->child4 = createQuadTree(img, size, factor, x + size, y);

    return newnode;
}

int mean_score(int x, int y, int size, Pixel **img, unsigned int factor,
               unsigned int *red_avg, unsigned int *blue_avg,
               unsigned int *green_avg) {
    /* (x,y) -> coordinates of starting element from submatrix
       size -> dimension of submatrix
       division = 0 -> no division needed
       division = 1  -> division
    */
    int i, j;
    int division = 1;
    unsigned long long red_sum = 0;
    unsigned long long green_sum = 0;
    unsigned long long blue_sum = 0;
    for (i = x; i < x + size; i++) {
        for (j = y; j < y + size; j++) {
            red_sum += img[i][j].rgbtRed;
            green_sum += img[i][j].rgbtGreen;
            blue_sum += img[i][j].rgbtBlue;
        }
    }
    // components of average color
    *red_avg = red_sum / (size * size);
    *green_avg = green_sum / (size * size);
    *blue_avg = blue_sum / (size * size);

    // similarity score
    unsigned long long mean_sum = 0;
    unsigned long long mean;
    for (i = x; i < x + size; i++) {
        for (j = y; j < y + size; j++) {
            mean_sum += (*red_avg - img[i][j].rgbtRed) *
                            (*red_avg - img[i][j].rgbtRed) +
                        (*green_avg - img[i][j].rgbtGreen) *
                            (*green_avg - img[i][j].rgbtGreen) +
                        (*blue_avg - img[i][j].rgbtBlue) *
                            (*blue_avg - img[i][j].rgbtBlue);
        }
    }
    mean = mean_sum / (3 * size * size);
    if (mean <= factor) {
        division = 0;  // no need for division
    }
    return division;
}

int findMax(int a, int b) { return (a > b) ? a : b; }

void printLevelOrder(QuadTreeNode *root, FILE *fptrout) {
    unsigned int h = countLevels(root, 0);
    int i;
    for (i = 1; i <= h; i++) {
        printCurrentLevel(root, i, fptrout);
    }
}

void printCurrentLevel(QuadTreeNode *root, int level, FILE *fptrout) {
    if (root == NULL) return;
    if (level == 1) {
        if (root->is_leaf != 0) {  // extern node(leaf)
            fwrite(&root->is_leaf, sizeof(unsigned char), 1, fptrout);
            fwrite(&root->color.rgbtRed, sizeof(unsigned char), 1, fptrout);
            fwrite(&root->color.rgbtGreen, sizeof(unsigned char), 1, fptrout);
            fwrite(&root->color.rgbtBlue, sizeof(unsigned char), 1, fptrout);
        } else {  // internal node (not leaf)
            fwrite(&root->is_leaf, sizeof(unsigned char), 1, fptrout);
        }
    } else if (level > 1) {
        printCurrentLevel(root->child1, level - 1, fptrout);
        printCurrentLevel(root->child2, level - 1, fptrout);
        printCurrentLevel(root->child3, level - 1, fptrout);
        printCurrentLevel(root->child4, level - 1, fptrout);
    }
}

int countLevels(QuadTreeNode *node, int level) {
    if (node == NULL) {
        return level;
    }

    int child1Levels = countLevels(node->child1, level + 1);
    int child2Levels = countLevels(node->child2, level + 1);
    int child3Levels = countLevels(node->child3, level + 1);
    int child4Levels = countLevels(node->child4, level + 1);

    // find the maximum level
    int maxLevel1 = findMax(child1Levels, child2Levels);
    int maxLevel2 = findMax(child3Levels, child4Levels);
    int maxLevel = findMax(maxLevel1, maxLevel2);

    return maxLevel;
}

int getLeafCount(QuadTreeNode *node) {
    if (node == NULL) return 0;
    if (node->child1 == NULL && node->child2 == NULL && node->child3 == NULL &&
        node->child4 == NULL)
        return 1;
    else
        return getLeafCount(node->child1) + getLeafCount(node->child2) +
               getLeafCount(node->child3) + getLeafCount(node->child4);
}

int findclosestleaf(QuadTreeNode *root) {
    if (root->is_leaf == 1) {
        return root->size_leaf;
    }

    int max_size = 0;
    max_size = findMax(max_size, findclosestleaf(root->child1));
    max_size = findMax(max_size, findclosestleaf(root->child2));
    max_size = findMax(max_size, findclosestleaf(root->child3));
    max_size = findMax(max_size, findclosestleaf(root->child4));

    if (max_size == 0) {
        max_size = root->size_leaf;
    }

    return max_size;
}

void freeMatrix(Pixel **img, int height, int width) {
    int i;
    for (i = 0; i < height; i++) {
        free(img[i]);
    }
    free(img);
}

void freeQuadTree(QuadTreeNode *root) {
    if (root == NULL)
        return;
    else {
        freeQuadTree(root->child1);
        freeQuadTree(root->child2);
        freeQuadTree(root->child3);
        freeQuadTree(root->child4);
        free(root);
    }
}

QuadTreeNode *ReCreateQuadTree(FILE *fptrin, int size) {
    QuadTreeNode *newnode = (QuadTreeNode *)malloc(sizeof(QuadTreeNode));
    // read node type (0/1)
    unsigned char node_type;
    fread(&node_type, sizeof(unsigned char), 1, fptrin);

    newnode->child1 = NULL;
    newnode->child2 = NULL;
    newnode->child3 = NULL;
    newnode->child4 = NULL;

    if (node_type == 0) {  // internal node
        newnode->is_leaf = 0;
        newnode->child1 = ReCreateQuadTree(fptrin, size);
        newnode->child2 = ReCreateQuadTree(fptrin, size);
        newnode->child3 = ReCreateQuadTree(fptrin, size);
        newnode->child4 = ReCreateQuadTree(fptrin, size);

    } else if (node_type == 1) {  // leaf node
        newnode->is_leaf = 1;
        fread(&newnode->color.rgbtRed, sizeof(unsigned char), 1, fptrin);
        fread(&newnode->color.rgbtGreen, sizeof(unsigned char), 1, fptrin);
        fread(&newnode->color.rgbtBlue, sizeof(unsigned char), 1, fptrin);
    }

    return newnode;
}

void build_image(QuadTreeNode *node, Pixel **image, int x, int y, int size) {
    int i, j;
    if (node->is_leaf == 1) {
        int level = (node->size_leaf);
        for (i = 0; i < level; i++) {
            for (j = 0; j < level; j++) {
                image[x + i][y + j].rgbtRed = node->color.rgbtRed;
                image[x + i][y + j].rgbtGreen = node->color.rgbtGreen;
                image[x + i][y + j].rgbtBlue = node->color.rgbtBlue;
            }
        }
    } else {
        int half = size / 2;
        build_image(node->child1, image, x, y, half);
        build_image(node->child2, image, x, y + half, half);
        build_image(node->child3, image, x + half, y + half, half);
        build_image(node->child4, image, x + half, y, half);
    }
}
void set_size_leaf(QuadTreeNode *node, int size) {
    if (node == NULL) {
        return;
    }

    if (node->is_leaf == 1) {
        node->size_leaf = size;
        return;
    }

    int half_size = size / 2;
    set_size_leaf(node->child1, half_size);
    set_size_leaf(node->child2, half_size);
    set_size_leaf(node->child3, half_size);
    set_size_leaf(node->child4, half_size);
}
