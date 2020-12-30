//张若
//2020111456
//B+tree实现字符串比对

#include<stdlib.h>
#include<stdio.h>
#include<malloc.h>
#define BUFSIZE 2048
#define M 3
#define Unavailable -1

typedef struct bPlusTreeNode *bPlusTree,*position; 
struct bPlusTreeNode
{
    int keyNum;
    unsigned int key[M+1];  //存放孩子结点中的最大值
    bPlusTree child[M+1];  //存放孩子结点
    bPlusTree next;
}Node,Tree;


bPlusTree insertTree(bPlusTree H,unsigned int hash);
bPlusTree recurseInsert(bPlusTree T,unsigned int hash,int i,bPlusTree parent);
bPlusTree initialize();
bPlusTree mallocNewNode(); 
bPlusTree insertElement(int insertKey,position parent,position x,unsigned int hash,int i,int j);
position moveElement(position src,position dst,position parent,int i,int n);
position findMostRight(position p);
position findMostLeft(position p);
position removeElement(int isKey,position parent,position x,int i,int j);
position findSibling(position parent,int i);
bPlusTree splitNode(position parent,position x,int i);
int searchBPlusTree(bPlusTree T,int hash);
unsigned int ELFHash(char *str); 


int main(int argc, char const *argv[])
{
	int searchRes=0; 
    FILE *dict=NULL,*str=NULL,*res=NULL;
    char buf[BUFSIZE];
    unsigned int hash=0;
    
    /* 利用文件构造b+树 */
    if((dict=fopen("dict.txt","r"))==NULL){
        printf("打开dict文件失败！\n");
        exit(1);
    }
    bPlusTree H=NULL; 
    while(fgets(buf,BUFSIZE,dict)!=NULL){
    	
        hash=ELFHash(buf);
        H=initialize();
        H=insertTree(H,hash);
    }
    // 在b+树中查找需要比对的字符串
    if((str=fopen("string.txt","r"))==NULL){
        printf("打开string文件失败！\n");
        exit(1);
    }
    if((res=fopen("result.txt","w"))==NULL){
        printf("打开result文件失败！\n");
        exit(1);
    }
    int r=0,numOfRes=0;
    while (fgets(buf,BUFSIZE,str)!=NULL)
    {
        hash=ELFHash(buf);
//        printf("hash:%d\n",hash);
        searchRes=searchBPlusTree(H,hash);
//        printf("searchRes:%d\n",searchRes);
        if(searchRes==1){
            fputs(buf,res);
            numOfRes++;
        }
    }
    printf("查找到的结果数量为：%d",numOfRes);
    return 0;
}
/* 搜索b+树中的内容 */
int searchBPlusTree(bPlusTree T,int hash){
	
    if(T==NULL){
//    	printf("T==NULL\n");
        return 0;
    }else
    {
    	
        int k=T->keyNum-1,limit=T->keyNum;
        while(k>=0){
//        	printf("k:%d,T->key[k]:%d\n",k,T->key[k]);
            if(T->key[k]<hash) {
            	k--;
            	continue;
			}else if(T->key[k]==hash){
				return 1;
			}else{
//				printf("recurse\n");
				return searchBPlusTree(T->child[k],hash);
//				printf("recurseFin\n");
			}
			
        }
        return 0;
    }
    
} 
/* ELFHash*/ 
unsigned int ELFHash(char *str)
{
    unsigned int hash = 0;
    unsigned int x = 0;

    while (*str)
    {
        hash = (hash << 4) + (*str++);//hash左移4位，把当前字符ASCII存入hash低四位。
        if ((x = hash & 0xF0000000L) != 0)
        {
            hash ^= (x >> 24);
            hash &= ~x;
        }
    }
    return (hash & 0x7FFFFFFF);
}
/* 初始化b+树*/ 
bPlusTree initialize(){
//	printf("initialize\n") ;
    if(M<3){
        printf("b+树的阶数不可小于3！\n");
        exit(1);
    }
    bPlusTree T;
    T=mallocNewNode();
//    T->key=hash;
//    T->keyNum++;
    return T;
    
}

/* 分配节点空间*/
bPlusTree mallocNewNode(){
//	printf("mallocNewNode\n") ;
    bPlusTree newNode=(bPlusTree)malloc(sizeof(struct bPlusTreeNode));
    if(newNode==NULL){
        printf("节点空间分配失败！\n");
        exit(1);
    }
    // 初始化节点
    newNode->keyNum=0;
    int i=0;
    for(;i<M+1;i++){
    	newNode->key[i]=0;
	}
    for(i=0;i<M+1;i++){
		newNode->child[i]=NULL;
	}
    newNode->next=NULL;
//    printf("mallocNewNodeFin\n") ;
    return newNode;
} 

/*在b+树中插入数据
	H：头结点
    hash：插入的第一个hash值
*/ 
bPlusTree insertTree(bPlusTree H,unsigned int hash){
//	printf("insertTree\n") ;
    return recurseInsert(H,hash,0,NULL);
}

/*递归构造b+树，插入成功后返回指针
    T:传入的节点
    hash：需要存储的数据
    i：hash在T中的位置
    parent：T的双亲节点
    返回修改后的T
 */ 
bPlusTree recurseInsert(bPlusTree T,unsigned int hash,int i,bPlusTree parent){  
//    printf("recurseInsert\n");
    // j作为查找T的key的下标，从0到T->keyNum-1
    // j是T中结点的下标，即插入位置
    int j=0,limit=M;
	
	//寻找在本层的插入或向下递归的位置 
    while(j<T->keyNum&&hash<=T->key[j]&&T->key[j]!=Unavailable){
        if(hash==T->key[j]){
            return T;
        }
        j++;
    }

    if(j!=0&&T->keyNum!=0){
        j--;
    }
    // 如果是叶子节点，则直接插入
    if (T->keyNum==0)
    {
        printf("insertLeaves\n");
        T=insertElement(1,parent,T,hash,i,j);
    }
    // 不是叶子节点则递归向下
    else
    {
        T->child[j]=recurseInsert(T->child[j],hash,j,T);
    }
    
    // 调整节点
    bPlusTree sibling=NULL;
    if(T->keyNum>limit){
        // 根节点直接分裂
        if(parent==NULL){
            T=splitNode(parent,T,i);
        }
        // 非根节点看能否移动，若可以移动则移动，不能则分裂
		else{
//			printf("i:%d\n",i);
//			printf("parent-keynum:%d\n",parent->keyNum);
            sibling=findSibling(parent,i);
            if(sibling!=NULL){
                moveElement(T,sibling,parent,i,1);
            }else
            {
                T=splitNode(parent,T,i);
            }   
        }   
    }
//    printf("recurseInsert3\n");
    if(parent!=NULL){
        parent->key[i]=T->key[0];
    }
//    printf("recurseInsertFin\n");
    return T;
}

/* 将结点从src移动到dst
    src：源节点
    dst：目的节点
    parent：双亲节点
    i：src在parent中的位置
    n：移动节点的数量
    返回parent
 */   
position moveElement(position src,position dst,position parent,int i,int n){  
//	printf("moveElement\n") ;
    // j：src中需要移动元素的开始下标
    int tempKey=0,j=0,srcInFront=0,tempI=1,limit=M;
    position child=NULL;

    // if(dst->keyNum>limit){
    // 	dst->keyNum=dst->keyNum-M-1;
    // 	tempI=i+1;
	// }else if(dst->keyNum<0){
	// 	dst->keyNum=dst->keyNum+M+1;
	// 	tempI=i-1;
	// }

	// dst在src后面，向后移动
    if(src->key[0]>dst->key[0]){
        // 如果src是非叶子节点，需要移动节点
        if (src->child[0]!=NULL)
        {
//        	printf("src<dst\n") ;
            // int a=0;
            while(j<n){
//            	int a=0;
//            	for(;a<src->keyNum-1;a++){
////            		printf("a:%d\n",a);
//            		if(src->child[src->keyNum-1]==NULL){
////            			printf("null\n");
//					}else{
//						printf("not null\n");
//					}
//				}
                child=src->child[src->keyNum-1];
                removeElement(0,src,child,src->keyNum-1,Unavailable);
                insertElement(0,dst,child,Unavailable,0,Unavailable);
                j++;
                // a++;
            }
        }
        // 如果src是叶子节点，需要移动数据
        else
        {
            // int a=0;
            while(j<n){
                tempKey=src->key[src->keyNum-1];
                removeElement(1,parent,src,i,src->keyNum-1);
                insertElement(1,parent,dst,tempKey,i+1,0);
                j++;
            }
        }
//        printf("parent->key[i+1]:%d,dst->key[0]\n",parent->key[i+1],dst->key[0]);
        parent->key[i+1]=dst->key[0];
        //树叶节点重新连接
        if(src->keyNum>0){
            findMostRight(src)->next=findMostLeft(dst);
        }
//        printf("parent->keynum(1):%d\n",parent->keyNum);
	}
	else
    {
        // src是非叶子节点，向前移动节点
        if (src->child[0]!=NULL)
        {
//        	printf("src>dst\n") ;
            while(j<n){
                child=src->child[0];
                removeElement(0,src,child,0,Unavailable);
                insertElement(0,dst,child,Unavailable,dst->keyNum,Unavailable);
                j++;
            }
        }
        // src是叶子节点，向前移动数据
        else
        {
            while(j<n){
//            	printf("j:%d\n",j);
                tempKey=src->key[0];
//                printf("parent->keynum(2.1.1):%d\n",parent->keyNum);
                removeElement(1,parent,src,i,0);
//                printf("parent->keynum(2.1.2):%d\n",parent->keyNum);
//                printf("i:%d\n",i);
//                printf("parent->key[i+1]:%d,dst->key[0]:%d\n",parent->key[i+1],dst->key[0]);
//				printf("dst->keynum:%d\n",dst->keyNum);
				
				insertElement(1,parent,dst,tempKey,i-1,dst->keyNum);  
                
//                printf("parent->keynum(2.1.3):%d\n",parent->keyNum);
                j++;
            }
//            printf("parent->keynum(2.2):%d\n",parent->keyNum);
        }
//        printf("parent->keynum(2):%d\n",parent->keyNum);
//        printf("parent->key[i]:%d,src->key[0]:%d\n",parent->key[i],src->key[0]);
        parent->key[i]=src->key[0];
        if(src->keyNum>0){
            findMostRight(dst)->next=findMostLeft(src);
        }
        
    }
//    printf("parent->keynum(moveElement):%d\n",parent->keyNum);
//    printf("src->keynum(moveElement):%d\n",src->keyNum);
//    printf("dst->keynum(moveElement):%d\n",dst->keyNum);
//    printf("moveElementFin\n");
    
    return parent;
}

/* 找到最右端节点*/ 
position findMostRight(position p){
    position temp=p;
    while (temp!=NULL&&temp->child[temp->keyNum-1]!=NULL)
    {
        temp=temp->child[temp->keyNum-1];
    }
    return temp;
}
/* 找到最左端节点*/ 
position findMostLeft(position p){
    position temp=p;
    while (temp!=NULL&&temp->child[0]!=NULL)
    {
        temp=temp->child[0];
    }
    return temp;
}
/* 删除节点
    isKey：删除的是数据或者节点
    isKey==1：删除数据
    parent：双亲节点
    x：插入数据的节点
    i：x在parent中的位置
    j：要删除数据在x中的位置

    isKey==0：删除节点
    parent：双亲节点
    x：要删除数据的节点
    i：x在parent中的位置
    j无效

    isKey==0：删除数据
*/ 
position removeElement(int isKey,position parent,position x,int i,int j){  
//	printf("removeElement,iskey:%d,i:%d,j:%d\n",isKey,i,j);
    int k=0,limit=x->keyNum;
    if(isKey){
        k=j+1;
        while(k<limit){
            x->key[k-1]=x->key[k++];
        }
        x->key[x->keyNum-1]=Unavailable;
        parent->key[i]=x->key[0];
        x->keyNum--;
    }
    else
    {
    	
        if(x->child[0]==NULL&&i>0){
            parent->child[i-1]->next=parent->child[i+1];
        }
        limit=parent->keyNum;
        k=i+1;
        while (k<limit)
        {
            parent->child[k-1]=parent->child[k];
            parent->key[k-1]=parent->key[k];
            k++;
        }
        parent->child[parent->keyNum-1]=NULL;
        parent->key[parent->keyNum-1]=Unavailable;
        parent->keyNum--;
    }
//    printf("removeElementFin\n");
    return x;
    
}
/* 寻找一个兄弟节点，其关键字未满，否则返回NULL
    parent：双亲节点
    i：parent中需要分裂的节点下标
    返回找到的可分流节点的位置
*/ 
position findSibling(position parent,int i){
//	printf("findSibling\n");
//	printf("i(findSibling):%d\n",i);
//	if(parent==NULL) printf("null\n");
//	else printf("not\n");
	
    position sibling=NULL;
    int limit=M;
	
    if(i==0){
//    	printf("i==0\n");
        // ?会存在0孩子需要分裂，但1孩子没有数据的情况
        if(parent->child[1]->keyNum!=0&&parent->child[1]->keyNum<limit){
            sibling=parent->child[1];
        }  
    }
    else if (parent->child[i-1]->keyNum<limit)
    {
//    	printf("<\n");
        sibling=parent->child[i-1];
        // sibling->keyNum=sibling->keyNum-M-1;
    }
    else if ((i+1)<parent->keyNum&&parent->child[i+1]->keyNum<limit)
    {
//    	printf(">\n");
        sibling=parent->child[i+1];
        // sibling->keyNum=sibling->keyNum+M+1;
    }
//    printf("findSiblingFin\n");
    return sibling;
}

/* 分离数量超过极限的节点 
    parent：双亲节点
    x：分裂位置
    i：x在parent中的位置
    返回？
*/ 
bPlusTree splitNode(position parent,position x,int i){
//	printf("splitNode\n") ;
    position newNode;

    newNode=mallocNewNode();
    int j=(x->keyNum)/2,k=0;   
//    printf("x-keyNum:%d\n",x->keyNum);
//    printf("j:%d\n",j);
    while(j<x->keyNum){
        if(x->child[0]!=NULL){
            newNode->child[k]=x->child[j];
            x->child[j]=NULL;
        }
        newNode->key[k++]=x->key[j];
        x->key[j++]=Unavailable;
        newNode->keyNum++;
        x->keyNum--;
    }
    if(parent!=NULL){
        insertElement(0,parent,newNode,Unavailable,i+1,Unavailable);
    }
    // 为根节点构造父节点
    else
    {
        parent=mallocNewNode();
        insertElement(0,parent,x,Unavailable,0,Unavailable);
        insertElement(0,parent,newNode,Unavailable,1,Unavailable);
//        printf("splitNodeFin(parent)\n");
        return parent;
    }
//    printf("splitNodeFin(x)\n");
    return x;
}

/* 插入元素，返回指向插入节点的指针
    insertKey：判断插入的是数据还是节点
    
    若insertKey==1，则插入的是数据
    parent：双亲节点
    x：插入数据的节点
    hash：插入的数据
    i：x在parent的位置
    j：数据要在x中插入的位置

    若insertKey==0，则插入的是节点
    parent：双亲节点
    x：要插入的节点
    hash：无用
    i：x要在parent中插入的位置
    j：无用
*/ 
bPlusTree insertElement(int insertKey,position parent,position x,unsigned int hash,int i,int j){
//	printf("insertElement\n") ;
    int k=0;

	if(insertKey){
//		if(parent!=NULL){
//    		printf("parentKeyNum1(insertElement):%d\n",parent->keyNum);
//		}
        k=x->keyNum-1;
        while(j<=k){
            x->key[k+1]=x->key[k];
            k--;
        }
//        if(parent!=NULL){
//			printf("parentKeyNum1(insertElement):%d\n",parent->keyNum);
//		}

        x->key[j]=hash;
//        if(parent!=NULL){
//			printf("parentKeyNum2(insertElement):%d\n",parent->keyNum);
//		}
		
        if(parent!=NULL){
//			if(i==-1){
//				int a=parent->keyNum-1;
//				while(a>=0){
//					parent->key[a+1]=parent->key[a--];
//				}
//				parent->key[0]=x->key[0];
//			}else{
			parent->key[i]=x->key[0];
			
        }
//        if(parent!=NULL){
//			printf("parentKeyNum3(insertElement):%d\n",parent->keyNum);
//		}
        x->keyNum++;
//        if(parent!=NULL){
//			printf("parentKeyNum3(insertElement):%d\n",parent->keyNum);
//		}
//        printf("x->keyNum(insertKey):%d\n",x->keyNum);
    }
    else
    {
        if(x->child[0]==NULL){
            if(i>0){
                parent->child[i-1]->next=x;
            }
            x->next=parent->child[i];
        }
        if(parent->keyNum!=0){
            k=parent->keyNum-1;
            while(k>=i){
                parent->child[k+1]=parent->child[k];
                parent->key[k+1]=parent->key[k];
                k--;
            }
        }
        
        parent->child[i]=x;
        parent->key[i]=x->key[0];
        parent->keyNum++;
//        printf("parent->keyNum(insertNode):%d\n",parent->keyNum);
    }
//    printf("insertElementFin\n");
    return x;
}