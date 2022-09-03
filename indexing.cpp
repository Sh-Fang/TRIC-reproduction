#include <iostream>
#include <fstream>  //文件操作库
#include <vector>   //向量库
#include <unordered_map>  //无序哈希表map
//#include <unordered_set>  //无序集合set
#include <memory>  //智能指针库
#include <utility>  //pair库
#include <typeinfo>


using namespace std;


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


class vertex_Q_Node{   //Q为有向图
public:
    int label;  //储存节点字母
    int id;  //节点id
    int Q_id,total_v_num;
    vector<int> neighbor; //存邻居节点id
public:
    vertex_Q_Node(){
        this->id = this->Q_id = this->label =  -1 ;
        this->total_v_num = 0;
    }

    void save_Q_node_info(int _Q_id,int _total_v_num,int _id,int _label){   //储存节点信息
        this->Q_id = _Q_id;
        this->total_v_num = _total_v_num;
        this->id = _id;
        this->label = _label;
    }

    void add_Q_neighbor(int v_id){   //添加邻居节点
        this->neighbor.push_back(v_id);
    }
};


class EdgePairNode{   //边对节点
public:
    int Q_id;      //即：这条边属于哪个Q
    vector<int> Q_id_ptr;  //即：{Q}这种标识符，用empty来判断是否有元素
    int node_id;   //即：在rootInd中的n
    int first_node_in_degree;   //边对中第一个节点的入度 <D,E>
    int second_node_out_degree; //边对中第二个节点的出度
    pair<int,int> edge_pair;
    EdgePairNode *next;
    vector<EdgePairNode*> child;
    int child_count;    //记录现在可以插入的孩子是第几个位置
    EdgePairNode *parent;
public:
    EdgePairNode(){
        this->first_node_in_degree = 0;  //默认入度为0，后面只需要管那些入度不为0的，不需要再让入度本身为0的节点再进行一次赋0操作
        this->second_node_out_degree = 0;
        this->next = this->parent = nullptr;
        this->node_id = -1;
        this->Q_id = -1;
    }

    void compose_edge(int edge1 , int edge2){
        this->edge_pair.first = edge1;
        this->edge_pair.second = edge2;
    }
};



class PiChainNode{   //链表表头节点
public:
    int length;  //当前这条链表的长度
    int Q_id;    //这条链表属于哪个Q
    EdgePairNode *next;
public:
    PiChainNode(){     //初始化链表头
        this->Q_id = -1;
        this->length = -1;
        this->next = nullptr;
    }
};

class PiChain{    //Pi链表
public:
    int length;  //当前这条链表的长度
    int Q_id;    //这条链表属于哪个Q
    EdgePairNode *p,*pre,*next;
//    EdgePairNode *head,*p,*pre;
public:
    PiChain(){     //初始化链表
        this->Q_id = -1;
        this->length = -1;
        this->next = this->p = this->pre = nullptr;
//        this->head = p = pre = nullptr;
    }
    void add_node(EdgePairNode &node){    //链表中添加节点
        p = &node;
        if(this->next == nullptr){  //此时链表为空
            this->next = p;
            pre = p;
            p->next = nullptr;   //处理尾指针
        } else{
            pre->next = p;
            pre = p;
            p->next = nullptr;   //处理尾指针
        }
    }

    void clear_chain(){
        this->Q_id = -1;
        this->length = -1;
        next = p = pre = nullptr;
    }
};



class rootInd_Node{   //rootInd中的节点的定义
public:
    pair<int,int> directed_edge;
    int query_id;   //对应类型为t时候的第一个参数Q_id
    int child_id;   //由于使用的是N叉树，所以需要记录每个节点下面的孩子数
    int node_id;
    vector<rootInd_Node*> child;
    rootInd_Node *parent;   //使用双向树
};




int node_id = 0;  //使用全局变量，使得在T1,T2...的不同树中，n的序号能接上

class rootInd_Tree {   //rootInd的树的定义
public:
    rootInd_Node *pre,*p,*head;
public:

    rootInd_Tree(){  //初始化构造函数
        pre = p = head = nullptr;
    }

    void add_Node(rootInd_Node *node){
        p = node;
        if(head == nullptr){   //如果是该树的第一个节点
            head->child[p->child_id++] = p;   //每次添加一个孩子后，把child_id加一
            p->node_id = node_id++;    //把n保存到该节点里面
            pre = p;
        } else{
            p->parent = pre;  //此时pre停在上一个节点那里
            pre->child[p->child_id++] = p;
            p->node_id = node_id++;    //把n保存到该节点里面
            pre = p;
        }

        cout << "Adding Node Successfully" << endl;

    }

//	void del_Node(int node_id){  //使用全树唯一的node_id来确认节点
//		//TODO:是否需要这个删除节点的操作
//	}
};


class PiTreeNode{
public:
    EdgePairNode *node;
    vector<PiTreeNode*> child;
    int child_count;    //记录现在可以插入的孩子是第几个位置
    PiTreeNode *parent;
public:
    PiTreeNode(){
        this->node = nullptr;
        this->parent = nullptr;
        this->child_count = 0;
    }

    void clear(){
        this->node = nullptr;
        this->parent = nullptr;
        this->child_count = 0;
        this->child.clear();
    }
};

class PiTreeClass{
public:
    EdgePairNode *p,*pre,*next;
public:
    PiTreeClass(){
        this->next = this->p = this->pre = nullptr;
    }

    void add_node(EdgePairNode &node){
        p = &node;
        if(this->next == nullptr){  //此时链表为空
            this->next = p;
            pre = p;
            p->next = nullptr;   //处理尾指针
        } else{
            pre->next = p;
            pre = p;
            p->next = nullptr;   //处理尾指针
        }
    }
};



//class PiTreeClass{
//public:
//    pair<int,int> key_pair;  //以pair为key
//    EdgePairNode root_node;
////    EdgePairNode *head;
////    PiTreeNode *p,*pre,*head;
////    PiTreeNode TNode;
//public:
//    PiTreeClass(){
//        this->key_pair.first = -1;
//        this->key_pair.second = -1;
////        this->key_pair[0] = -1;
////        this->key_pair[1] = -1;
////        this->p = this->pre = this->head = nullptr;
////        this->head = &root_node;
//    }
//
//    void add_child_node(EdgePairNode *node){   //添加子节点
//        EdgePairNode *p = nullptr;
//        EdgePairNode *pre = nullptr;
//        EdgePairNode *head = &(PiTreeClass::root_node);
//        p = node;
////        p = node;
////        this->p = &TNode;
////        TNode.node = node_p;   //把传进来的节点包装一下，后面统一使用PiTreeNode作为PiTree的节点
//        if(1){   //如果树为空
//            head->next = p;
////            node->parent = head;
////            pre = node;
////            TNode.clear();
//        } else{
//            pre->child[pre->child_count++] = p;   //先添加孩子节点，再把孩子节点的count数加一
//            p->parent = pre;
//            pre = p;
////            TNode.clear();
//        }
//    }
//
////    void add_sibling_node(EdgePairNode &node){  //添加兄弟节点
////        p=&node;
//////        TNode.node = node;   //把传进来的节点包装一下，后面统一使用PiTreeNode作为PiTree的节点
////        pre = pre->parent;  //让pre自己回溯，找到父节点
////        pre->child[pre->child_count++] = p;
////        p->parent = pre;
////        pre = p;
////    }
//
//};






//class PTreeVector{
//public:
//    int count;
//    PiTreeClass PiTree[];
//public:
//    PTreeVector(){
//        count = 0 ;
//    }
//    void push_back(PiTreeClass node){
//        PiTree[count++] = node;
//    }
//};


//*************************************构造数据结构********************************************************
//*********************************************************************************************

vector<vertex_G_Node> G;   //初始化数据图
vector<vertex_Q_Node> Q;   //初始化多重查询图

vector<EdgePairNode> Pairs;   //存放所有的边对（Q1和Q2的都在里面）
vector<PiChain> P;   //存放所有Pi的向量（同一个Q_id的Pi存放在同一个向量里）
//unordered_map<int, vector<PiChain>> P_map;  //以Q_id为key，存放所有Pi
//PTreeVector PTree;
vector<PiTreeClass> PTree;
//vector<temp> PTreetemp;
//*************************************加载数据********************************************************
//*********************************************************************************************



//加载数据图
void inputG(string path_of_data_graph){
    char single_data;  //定义单个字符的变量，用来储存读入的数据
    ifstream infile;
    int id,id1,id2,lable,weight;
    vertex_G_Node v;

    infile.open(path_of_data_graph);  //打开数据图文件

    if(!infile){
        cerr << "Failed To Load Data Graph" << endl;  //cerr是std中的标准错误输出（和cout有区别）
        return;
    }

    while(infile >> single_data){   //依次读取其中的数据
        if(single_data=='v'){   //如果遇到了结点
            infile >> id >> lable;  //依次保存节点id和节点标签
            v.save_G_node_info(id,lable);
            G.push_back(v);   //把这个节点存到Q中
        }else if(single_data=='e'){
            infile >> id1 >> id2 >> weight;
            G[id1].add_G_neighbor(id2);
            G[id2].add_G_neighbor(id1);  //因为是无向图，所以要存两次
        }
    }

    cout << "Data Graph Loading Successfully" << endl;
}



//加载查询图
void inputQ(string path_of_query_graph){
    char single_data;  //定义单个字符的变量，用来储存读入的数据
    ifstream infile;
    int v_id,e_id1,e_id2,v_lable,e_weight;  //用于保存e和v开头的数据
    int Q_id,total_v_num;  //用于保存t开头的数据
    vertex_Q_Node u;

    infile.open(path_of_query_graph);  //打开查询图文件

    if(!infile){
        cerr << "Failed To Load Query Graph" << endl;  //cerr是std中的标准错误输出（和cout有区别）
        return;
    }

    while(infile >> single_data){   //依次读取其中的数据
        if(single_data=='t'){   //如果遇到了t
            infile >> Q_id >> total_v_num;  //第一个数据为查询图的编号，第二个数据为该查询图下有多少节点
        }else if(single_data=='v'){    //如果遇到了结点
            infile >> v_id >> v_lable;  //依次保存节点id和节点标签
            u.save_Q_node_info(Q_id,total_v_num,v_id,v_lable);
            Q.push_back(u);
        }else if(single_data=='e'){
            infile >> e_id1 >> e_id2 >> e_weight;
            Q[e_id1].add_Q_neighbor(e_id2);
//			Q[e_id1].add_Q_neighbor(e_id2);  //Q为有向图，所以存储结果为id1 -> id2
        }
    }

    cout << "Query Graph Loading Successfully" << endl;
}


void create_edge_pair_vector(){
    EdgePairNode e_node;
    for(auto i =0 ; i < Q.size() ; i++){
        for(auto it = Q[i].neighbor.begin();it != Q[i].neighbor.end();it++){  //遍历当前节点的所有邻居
            if(*it != Q[i].id){   //说明该节点是有邻居的
                e_node.edge_pair.first = Q[i].id;
                e_node.edge_pair.second = *it;
                e_node.Q_id = Q[i].Q_id;
                Pairs.push_back(e_node);  //保存边对
            }
        }
    }

    //现在，所有的边对都保存了，接下来是去除不符合要求的边对
    //统计所有节点的入度
    for(auto i = 0 ; i < Pairs.size() ; i++){
        for(auto j = 0 ; j < Pairs.size() ; j++){
            //如果边1与所有节点的边2无重复，并且还都是在一个Q里面，则说明该节点的入度为0
            if(Pairs[i].edge_pair.first == Pairs[j].edge_pair.second){
                if(Pairs[i].Q_id == Pairs[j].Q_id){
                    Pairs[i].first_node_in_degree++;
                }
            }
            //统计第二个节点的出度
            if(Pairs[i].edge_pair.second == Pairs[j].edge_pair.first){
                if(Pairs[i].Q_id == Pairs[j].Q_id){
                    Pairs[i].second_node_out_degree++;
                }
            }
        }
    }

    cout << "EdgePair Create Successfully" << endl;
}





//将边对添加到链表中去
void create_Pi_chain(){
    PiChain Pc;   //实例化一个链表
    int chainLength = 0;

    for(auto i = 0 ; i < Pairs.size() ; i++){  //找出能连起来的边对，将其保存到链表
        for(auto j = 0 ; j < Pairs.size() ; j++){
            //如果能连起来，而且同一个Q
            if(Pairs[i].Q_id == Pairs[j].Q_id){
                //如果能连起来
                if(Pairs[i].edge_pair.second == Pairs[j].edge_pair.first){
                    Pc.add_node(Pairs[i]);   //把能连起来的边表保存
                    chainLength++;   //每添加一个节点，就让length加一
                    if(Pairs[j].second_node_out_degree == 0){  //如果最后一个点出度为0，那这个点就是最后一个点
                        Pc.add_node(Pairs[j]);   //把最后这个节点保存再链表中
                        chainLength++;    //长度加一
                        Pc.Q_id = Pairs[i].Q_id;   //保存好Q_id
                        Pc.length = chainLength;
                        P.push_back(Pc);     //把链表压入P向量中
                        Pc.clear_chain();   //清空链表(每压入一次，就清空一次)
                    } else{
                        i = j; //下一次，让i跳过Pairs[j]这个节点，直接让i从j开始
                        continue;  //直接开始下一次内层for循环
                    }

                }

                //如果不能连起来，是单独的一个边对
                if (Pairs[i].second_node_out_degree == 0 && Pairs[i].first_node_in_degree == 0) {   //防止多次保存不同Q的同名节点
                    PiChain Pc2;
                    Pc2.add_node(Pairs[i]);
                    Pc2.Q_id = Pairs[i].Q_id;
                    Pc2.length = 1;
                    P.push_back(Pc2);  //把这个查询链P压入P向量中
                    Pc2.clear_chain();    //清空链表
                    break;   //j的目的是找能连起来的边，既然此时的i指向的是必然的单边，所以可以直接break结束当前的内循环
                }
            }
        }
    }

    cout << "PiChain Create Successfully" << endl;
}





//TODO：{Q}标识符一定都是在叶节点的！
//TODO：如果两个边对的起点就不相同，那么肯定不重合（即在rootInd里面不会重合）
//TODO:两层for循环，会让节点重复比较
//TODO：如何判断“完全重合”，“部分重合”。判断完成以后，如何标注{Q}，如何在建立rootInd的时候，让重复的节点只保留一次
//TODO：每次保存节点的时候都遍历所有的树的key（因为树是map，key就是第一条边），找出所有起点与该节点相同的树，
//      如果没有，就新建一棵树。如果有，那么就依次从树根开始匹配，直到查找到无法再匹配下去的节点X，此时把该节点
//      以及后续的节点都保存在X的父节点下面（也就是和X成为兄弟节点）。当保存完成以后，在该链表的最后一个节点上面打上{Q}




//**********************************************************************************
//PiTreeClass *PTree = new PiTreeClass[P.size()];  //以pair为key，存放所有Pi(有多少个P就建多少棵树)
//**********************************************************************************



//建立rootInd索引
void create_rootInd(){
    PiChain P1;
//    找出是否有重合的边
    for(auto i = 0 ; i < P.size() ; i++){
        int PTree_count = 0;   //每次处理不同Pi的时候，就新建一棵树
        P1 = P[i];   //P1的作用就是临时获取Pi链表
//        EdgePairNode *p = P1.next;    //p的作用就是用来递归获取Pi链表中的边对
        if(PTree.empty()){
//        if(PTree[PTree_count].key_pair.first == -1 && PTree[PTree_count].key_pair.second == -1){  //说明此时map为空
            for(auto j = 0 ; j < P1.length ; j++){   //因为链表是空的，所以把整个链表存进去
                if(P1.next != nullptr){
                    PTree[PTree_count].add_node(P1.next);   //作为孩子节点，保存到树中
                    //TODO:vector只能用pushback压进去，不能像数组那样插入数据
//                    p = p->next;
                }
            }
//            PTree[PTree_count].key_pair.first = p->edge_pair.first;  //当把完整的链表插入到空表里面的时候，更新map的key值
//            PTree[PTree_count].key_pair.second = p->edge_pair.second;
            PTree_count++;    //此时第一条P已经存进去了，所以处理下一条P的时候，需要新建一棵树，所以count++
        }
        else{  //如果链表不为空，就进行判断，判断边是否有重合
//            if(PTree[PTree_count].key_pair == p->edge_pair){  //如果从根节点就可以重合
//                EdgePairNode treePtr = PTree[PTree_count].head->node;
//                while (treePtr.edge_pair == p->edge_pair){
////                    treePtr = treePtr.next;
//                    p = p->next;
//                }
//
//                if(p->next == nullptr){   //说明p更短
//                    //TODO：则把{Q}标在与p最后一个节点同名的节点上
//                }
//
//                if(treePtr.next == nullptr){   //说明treePtr更短
//                    PTree[PTree_count].add_sibling_node(p);   //因为p指向的是一条链表，所以直接把p后面的节点全部加进去
//                }
//            }
        }
    }

    cout << "RootInd Create Successfully" << endl;
}

int main() {
    string path_of_data_graph = "E:\\QueryC++\\data-graph.txt";
    string path_of_query_graph = "E:\\QueryC++\\multi-query.txt";
    inputG(path_of_data_graph);    //读取Graph
    inputQ(path_of_query_graph);   //读取Query

    create_edge_pair_vector();   //创建边对向量

    create_Pi_chain();


    create_rootInd();

//    create_rootInd(P);

//    for( auto i = Pairs.begin() ; i != Pairs.end() ; i++){
//        cout << (*i).edge_pair.first << "->" << (*i).edge_pair.second <<endl;
//    }



//    create_rootInd(Q);
    return 0;
}