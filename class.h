#ifndef QUERYC___CLASS_H
#define QUERYC___CLASS_H
#endif //QUERYC___CLASS_H

//**************使用到的库*********************************************
#include <iostream>
#include <vector>   //向量库
#include <unordered_map>  //无序哈希表map
#include <map>
#include <utility>  //pair库
using namespace std;
//**************使用到的库*********************************************




//**************类的定义*********************************************
class vertex_G_Node{   //G为无向图
public:
    int label;  //储存节点字母
    int id;  //节点id
    vector<int> neighbor; //存邻居节点id
public:
    vertex_G_Node(){
        this->label = -1;
        this->id = -1;
    }
    void save_G_node_info(int _id,int _label){   //储存节点信息
        this->label = _label;
        this->id = _id;
    }

    void add_G_neighbor(int v_id){   //添加邻居节点
        this->neighbor.push_back(v_id);
    }
};



//*****************************************************************
class vertex_Q_Node{   //Q为有向图
public:
    int v_label;  //储存节点字母
    int v_id;  //节点id
    int Q_id,total_v_num;
    vector<int> neighbor_id; //存邻居节点id
    vector<int> neighbor_label; //存邻居节点label
public:
    vertex_Q_Node(){
        this->v_id = this->total_v_num  = this->Q_id =  this->v_label =  -1 ;
    }

    void save_Q_node_info(int _Q_id,int _total_v_num,int _v_id,int _v_label){   //储存节点信息
        this->Q_id = _Q_id;
        this->total_v_num = _total_v_num;
        this->v_label = _v_label;
        this->v_id = _v_id;
    }

    void add_Q_neighbor(int _v_id,int _v_label){   //添加邻居节点
        this->neighbor_id.push_back(_v_id);
        this->neighbor_label.push_back( _v_label);
    }
};



//*****************************************************************
class EdgePairNode{   //边对节点
public:
    int Q_id;      //即：这条边属于哪个Q
    vector<int> Q_id_ptr;  //即：{Q}这种标识符，用empty来判断是否有元素
    int node_id;   //即：在rootInd中的n
    int first_node_in_degree;   //边对中第一个节点的入度 <D,E>
    int second_node_out_degree; //边对中第二个节点的出度
    pair<int,int> label_pair;
    pair<int,int> edge_pair;
    vector<EdgePairNode*> child;   //树中的指针
    EdgePairNode *parent;
public:
    EdgePairNode(){
        this->first_node_in_degree = 0;  //默认入度为0，后面只需要管那些入度不为0的，不需要再让入度本身为0的节点再进行一次赋0操作
        this->second_node_out_degree = 0;
        this->parent = nullptr;
        this->node_id = -1;
        this->Q_id = -1;
    }
};





//*****************************************************************
class PiChain{    //Pi链表
public:
    int length;  //当前这条链表的长度
    int Q_id;    //这条链表属于哪个Q
    EdgePairNode *p,*pre,*head;
public:
    PiChain(){     //初始化链表
        this->Q_id = -1;
        this->length = 0;
        this->head = this->p = this->pre = nullptr;
    }
    void add_node(EdgePairNode &node){    //链表中添加节点
        p = &node;
        if(this->head == nullptr){  //此时链表为空
            this->head = p;
            p->parent = nullptr;  //因为我定义的链表没有头节点，所以如果这里让p的父亲指向pre的话，其实就是指向p自己，可能会造成后续的麻烦，所以干脆直接让它指向null
            pre = p;
        } else{
            pre->child.push_back(p);
            p->parent = pre;
            pre = p;
        }
    }

    void clear_chain(){
        this->Q_id = -1;
        this->length = 0;
        head = p = pre = nullptr;
    }
};

