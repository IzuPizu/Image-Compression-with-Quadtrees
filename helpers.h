/* URECHE Andreea-Maria - 312CC */

/*structure for RGB-triplet*/
typedef struct 
{   unsigned char rgbtBlue;
    unsigned char rgbtGreen;
    unsigned char  rgbtRed;
}Pixel;
 
/*structure for node */
typedef struct QuadTreeNode 
{
    struct QuadTreeNode *child1 ; //NW
   struct  QuadTreeNode *child2 ; //NE
   struct QuadTreeNode *child3 ; //SE
   struct QuadTreeNode *child4;  //SW
    Pixel color;
    int is_leaf ;  // 1-> leaf  |  0-> not leaf
    unsigned int size_leaf;
   
}QuadTreeNode;

 /*
    NW(1)  |  NE(2)
    ---------------
    SW(4)  |  SE(3)

 */

