#include <iostream>
#include <fstream>  //文件操作库
#include <algorithm>  //find_if库
#include <vector>   //向量库
#include <unordered_map>  //无序哈希表map
#include <map>
#include <utility>  //pair库

using namespace std;



//**************类的定义*********************************************
//G为无向图
class GNode{
public:
    int v_label;  //储存节点字母
    int v_id;  //节点id
    vector<int> two_way_neighbor_id; //双向保存邻居节点id
public:
    GNode(){
        this->v_label = -1;
        this->v_id = -1;
    }
    void save_G_node_info(int id,int label){   //储存节点信息
        this->v_label = label;
        this->v_id = id;
    }
    void add_G_neighbor(int id){   //添加邻居节点
        this->two_way_neighbor_id.push_back(id);
    }
};



//*****************************************************************
//Q为有向图
class QNode{
public:
    int u_label;  //储存节点字母
    int u_id;  //节点id
    int Q_id;
    vector<int> one_way_neighbor_id; //存邻居节点id
    vector<int> two_way_neighbor_id;  //双向邻居节点
public:
    QNode(){
        this->u_id =  this->Q_id = this->u_label =  -1 ;
    }

    void save_Q_node_info(int local_Q_id,int local_u_id,int local_u_label){   //储存节点信息
        this->Q_id = local_Q_id;
        this->u_label = local_u_label;
        this->u_id = local_u_id;
    }

    void add_Q_neighbor(int local_u_id){   //添加邻居节点
        this->one_way_neighbor_id.push_back(local_u_id);
    }
};



//*****************************************************************
//边对节点
class EdgePairNode{
public:
    int Q_id;      //即：这条边属于哪个Q
    vector<int> Q_id_ptr;  //即：{Q}这种标识符，用empty来判断是否有元素
    int first_node_in_degree;   //边对中第一个节点的入度 <D,E>
    int second_node_out_degree; //边对中第二个节点的出度
    pair<int,int> label_pair;
    pair<int,int> id_pair;
    vector<EdgePairNode*> child;   //树中的指针
public:
    EdgePairNode(){
        this->first_node_in_degree = 0;  //默认入度为0，后面只需要管那些入度不为0的，不需要再让入度本身为0的节点再进行一次赋0操作
        this->second_node_out_degree = 0;
        this->Q_id = -1;
    }

    void Q_id_ptr_Push_Back(int id){  //在压入Q_id_ptr时候进行判断，向量中是否有重复的元素
        auto it = find(this->Q_id_ptr.begin(),this->Q_id_ptr.end(),id);
        if(it == this->Q_id_ptr.end()){  //如果找不到重复元素，那么就压进去
            this->Q_id_ptr.push_back(id);
        }
    }

    void clear(){
        this->first_node_in_degree = 0;  //默认入度为0，后面只需要管那些入度不为0的，不需要再让入度本身为0的节点再进行一次赋0操作
        this->second_node_out_degree = 0;
        this->Q_id = -1;
        this->Q_id_ptr.clear();
        this->child.clear();
    }
};





//*****************************************************************
//Pi链表
class PiChain{
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
            pre = p;
            this->length += 1;
            this->Q_id = node.Q_id;
        } else{
            auto it = find(pre->child.begin(),pre->child.end(),p);
            if(it == pre->child.end()){   //如果没有找到重复的元素，那么就插入
                pre->child.push_back(p);
                pre = p;
                this->length += 1;
                this->Q_id = node.Q_id;
            }
        }
    }

    void clear_chain(){
        this->Q_id = -1;
        this->length = 0;
        head = p = pre = nullptr;
    }
};



class GmatV_Node{
public:
    pair<int,int> vid_pair;
    pair<int,int> uid_pair;
    vector<GmatV_Node*> child;
    GmatV_Node *parent;

public:
    GmatV_Node(){
        parent = nullptr;
    }
};


//*****************************************************************
vector<GNode> G;   //初始化数据图:G用向量存
unordered_map<int,int> G_Vid_Vlabel;   //G中所有节点v_label和v_id的对应关系(key是V_id)
vector<QNode> Q;   //初始化多重查询图
unordered_map<int,int> Q_Uid_Ulabel;

vector<EdgePairNode> Pairs;   //存放所有的边对（Q1和Q2的都在里面）

vector<PiChain> P;   //存放所有Pi的向量（同一个Q_id的Pi存放在同一个向量里）
unordered_map<int,PiChain> PTrees;   //key是Ti的编号，value是Pi链

map<pair<int,int>,vector<EdgePairNode*>> edgeInd;   //无序map不能使用pair作为key，而有序map可以(key是label_pair，value是对应的节点的连接)

unordered_map<int,vector<EdgePairNode>> queryInd;   //key是Qid，value是n节点

map<pair<int,int>,vector<pair<int,int>>> G_matV;    //key是label_pair，value是顶点对

//*****************************************************************










//**********************函数的定义*******************************************

//加载数据图
void inputG(const string& path_of_data_graph){
    char single_data;  //定义单个字符的变量，用来储存读入的数据
    ifstream infile;   //读取文件的数据流
    int id,id1,id2,label,weight;
    GNode v;

    infile.open(path_of_data_graph);  //打开数据图文件

    if(!infile){
        cerr << "Failed To Load Data Graph" << endl;  //cerr是std中的标准错误输出（和cout有区别）
        return;
    }

    while(infile >> single_data){   //依次读取其中的数据
        if(single_data=='v'){   //如果遇到了结点
            infile >> id >> label;  //依次保存节点id和节点标签
            v.save_G_node_info(id, label);
            G.push_back(v);   //把这个节点存到G中
        }else if(single_data=='e'){
            infile >> id1 >> id2 >> weight;
            G[id1].add_G_neighbor(id2);
            G[id2].add_G_neighbor(id1);  //因为是无向图，所以要存两次
        }
    }


    //建立G中所有节点v_label和v_id的对应关系
    for(auto & G_item : G){   //遍历G的所有节点
        G_Vid_Vlabel[G_item.v_id] = G_item.v_label;
    }

    infile.close();

    cout << "Data Graph Loading Successfully" << endl;
}



//****************************************************************
//加载查询图
void inputQ(const string& path_of_query_graph){
    char single_data;  //定义单个字符的变量，用来储存读入的数据
    ifstream infile;
    int u_id,e_id1,e_id2,u_label,e_weight;  //用于保存e和v开头的数据
    int Q_id,total_v_num;  //用于保存t开头的数据
    QNode u;

    infile.open(path_of_query_graph);  //打开查询图文件

    if(!infile){
        cerr << "Failed To Load Query Graph" << endl;  //cerr是std中的标准错误输出（和cout有区别）
        return;
    }

    while(infile >> single_data){   //依次读取其中的数据
        if(single_data=='t'){   //如果遇到了t
            infile >> Q_id >> total_v_num;  //第一个数据为查询图的编号，第二个数据为该查询图下有多少节点
        }else if(single_data=='v'){    //如果遇到了结点
            infile >> u_id >> u_label;  //依次保存节点id和节点标签
            u.save_Q_node_info(Q_id, u_id, u_label);
            Q.push_back(u);   //把u依次压入向量中
        }else if(single_data=='e'){
            infile >> e_id1 >> e_id2 >> e_weight;
            Q[e_id1].add_Q_neighbor(e_id2);   //存储有向图版本的Q
            Q[e_id1].two_way_neighbor_id.push_back(e_id2);  //存储无向图版本的Q
            Q[e_id2].two_way_neighbor_id.push_back(e_id1);
        }
    }

    //建立Q中所有节点v_label和v_id的对应关系
    for(auto & Q_item : Q){   //遍历G的所有节点
        Q_Uid_Ulabel[Q_item.u_id] = Q_item.u_label;
    }

    infile.close();

    cout << "Query Graph Loading Successfully" << endl;
}



//****************************************************************
//创建边对节点
void create_edge_pair_vector(){
    EdgePairNode e_node;
    for(auto &Q_item:Q){
        for(auto &Q_item_neighbor:Q_item.one_way_neighbor_id){
            e_node.Q_id = Q_item.Q_id;
            e_node.Q_id_ptr.push_back(Q_item.Q_id);  //先把自己的Qid压进去
            e_node.id_pair = {Q_item.u_id, Q_item_neighbor};
            e_node.label_pair = {Q_item.u_label, Q_Uid_Ulabel[Q_item_neighbor]};
            Pairs.push_back(e_node);
            e_node.clear();
        }
    }

    //现在，所有的边对都保存了，接下来是统计所有节点的入度
    for(auto i = 0 ; i < Pairs.size() ; i++){
        for(auto j = 0 ; j < Pairs.size() ; j++){
            //如果边1与所有节点的边2无重复，并且还都是在一个Q里面，则说明该节点的入度为0
            if(Pairs[i].Q_id == Pairs[j].Q_id){
                //统计第一个节点的入度
                if(Pairs[i].id_pair.first == Pairs[j].id_pair.second){
                    Pairs[i].first_node_in_degree++;
                }

                //统计第二个节点的出度
                if(Pairs[i].id_pair.second == Pairs[j].id_pair.first){
                    Pairs[i].second_node_out_degree++;
                }
            }

        }
    }

    cout << "EdgePair Create Successfully" << endl;
}






//*****************************************************************
//将边对添加到链表中去
void create_Pi_chain(){
    PiChain Pc;   //实例化一个链表
    int pre,p,q;
    pre = 0;
    while(pre < Pairs.size()){   //pre是单向的，只向前移动，不回头，一次移动一个单位
        q = 0;                  //q是循环指针，每次都从0开始，一直到结尾
        p = pre;                //p是临时工作指针，如果q找到了连起来的边，那么就把p指向q；每次处理开始，让p回到pre的位置
        if(Pairs[pre].first_node_in_degree == 0){    //如果第一节点入度为0，那么一定是起点
            Pc.add_node(Pairs[pre]);             //先把起点存起来（因为在add_node这个函数里面，我已经写了“重复了就不往里面存”的判断，所以可以放心的往里面存）
            if(Pairs[pre].second_node_out_degree == 0){    //如果第一节点入度为0的同时，第二节点也为0，那么就是单边
                P.push_back(Pc);     //因为是单边，所以可以直接把链表压入P向量中
                Pc.clear_chain();   //清空链表(每压入一次，就清空一次)
                pre++ ;              //每压入链表一次，就让pre向后移动一个
                continue;            //pre移动以后，就可以直接开始下一次处理了
            }
        } else{            //如果第一节点入度不为0，说明不是起点，而是中间的点
            pre++;         //因为这个位置的代码是最开头，如果这个地方就遇到了中间节点，那么直接就可以pre++了
            continue;
        }

        while(q < Pairs.size()){    //q这个循环指针
            if(Pairs[p].Q_id == Pairs[q].Q_id){
                if(Pairs[p].id_pair.second == Pairs[q].id_pair.first){
                    Pc.add_node(Pairs[q]);
                    p = q;            //如果q找到了可以配对的边，就让p指向q的位置
                    q = 0;            //为下一次的处理做准备，让q回到0的位置
                    continue;
                }
            }
            q++;           //如果Qid不同，就让q继续往下
        }

        P.push_back(Pc);     //把链表压入P向量中
        Pc.clear_chain();   //清空链表(每压入一次，就清空一次)
        pre++;             //每次压入链表后，就让pre++
    }


    cout << "PiChain Create Successfully" << endl;
}





//****************************************************************
//建立rootInd索引
void create_rootInd(){
    PiChain P1;  //P1的作用就是临时获取Pi链表
    int Tree_id = 0; //用于判定在map的第几个位置上新建一颗树
    EdgePairNode *Tp;   //指向树Ti的指针
    EdgePairNode *Pp;   //指向P1链表的指针
    bool is_create_a_new_tree;  //用于控制是否新建一棵树的开关
    bool is_joint_rest_node_to_tree = true;   //用于控制是否将链表剩下的节点拼接到树上的开关（默认为true）

    for(auto i = 0 ; i < P.size() ; i++){
        P1 = P[i];   //P1的作用就是临时获取Pi链表
        if(PTrees.empty()){   //如果map为空，那么就让第一条Pi直接存到map里面去
            PTrees.insert(pair<int,PiChain>{Tree_id,P1});
            P1.clear_chain();
            Tree_id++;   //Tree_id++是让下一次从第二个位置开始建树
        }
        else{
            if(P1.length == 1){   //如果是一条单边，也可以直接新建一棵树存进去
                PTrees.insert(pair<int,PiChain>{Tree_id,P1});
                P1.clear_chain();
                Tree_id++;
            }
            else{    //如果长度大于1，就挨个遍历看看有没有起点一样的
                //先把Tp和Pp两个指针定好位（先检查所有已经存在的树的第一个节点是否相同）
                for(auto j = 0 ; j < Tree_id ; j++){
                    if(PTrees[j].head->label_pair == P1.head->label_pair){  //如果找到了起点相同的，就不用往后找了
                        is_create_a_new_tree = false;   //不用新建一棵树
                        Tp = PTrees[j].head;   //定位指针
                        Pp = P1.head;
                        break;
                    }
                    else{
                        is_create_a_new_tree = true; //需要新建一棵树
                    }
                }


                if(is_create_a_new_tree){   //如果最后它的值都是true，那么就表示，整个for循环之中没有找到起点相同的节点
                    PTrees.insert(pair<int,PiChain>{Tree_id,P1});   //新建一棵树存进去
                    P1.clear_chain();
                    Tree_id++;
                } else{  //如果最后它的值都是false，说明找到了起点相同的节点，而且定位好了两个指针
                    while(Tp){
                        Tp->Q_id_ptr_Push_Back(Pp->Q_id);  //处理根节点（把P链表中与Ptree树中相同的节点的Qid压进去）

                        if(!Pp->child.empty()){
                            Pp = Pp->child[0];    //先让P链表的指针向后移动一个
                        } else{
                            break;
                        }

                        //接下来向下进行BFS
                        if(!Tp->child.empty()){
                            for(auto k = 0 ; k < Tp->child.size() ; k++){  //遍历所有孩子节点(BFS)
                                if(Tp->child[k]->label_pair == Pp->label_pair){  //注意：这里只是建立rootInd，只要起点不同，就不用往下遍历了（不用考虑后面有重复节点遍历不到的问题）
                                    Tp->child[k]->Q_id_ptr_Push_Back(Pp->Q_id);  //把有公共节点的另一条链表的Qid存进去
                                    Tp = Tp->child[k];
                                }
                            }
                        } else{    //如果遍历到这棵树的叶节点，那么就直接把剩余的链表压入叶节点后面
                            Tp->child.push_back(Pp);
                        }
                    }
                }
            }
        }
    }

    cout << "RootInd Create Successfully" << endl;
}


//创建queryInd（用最简单的方法，直接从Pairs里面获取边对）
void create_queryInd(){
    EdgePairNode temp_node;
    for(auto &it:Pairs){
        temp_node = it;
        temp_node.child.clear();
        queryInd[temp_node.Q_id].push_back(temp_node);
    }

    cout << "QueryInd Create Successfully" <<endl;
}



//建立edgeInd索引（用最简单的方法，直接从queryInd里面获取边对）
void create_edgeInd(){
    for(auto & queryInd_key : queryInd){
        for(auto &queryInd_value:queryInd_key.second){
            edgeInd[queryInd_value.label_pair].push_back(&queryInd_value);
        }
    }

    cout << "EdgeInd Create Successfully" <<endl;
}


//****************************************************************
//对初始数据图建立MatV视图
void create_G_matV(){
    for(auto &edgeInd_item:edgeInd){
        G_matV[edgeInd_item.first];   //占位，保证edgeInd里面所有的key都在matV里面
    }

    for(auto &G_item : G){
        for(auto &node_neighbor:G_item.two_way_neighbor_id){
            pair<int,int> label_pair = {G_item.v_label,G_Vid_Vlabel[node_neighbor]};
            if(edgeInd.find(label_pair) != edgeInd.end()){
                G_matV[label_pair].push_back({G_item.v_id, node_neighbor});
            }
        }
    }



    cout << "G_matV Create Successfully" <<endl;
}










//***************************************

map<int,int> uid_vid;

void print_match_tree(map<pair<int,int>,vector<GmatV_Node>> &query_node_map , pair<int,int> &last_pair){
    for(auto &it:query_node_map[last_pair]){
        auto p = &it;
        while(p != nullptr){
            uid_vid[p->uid_pair.first] = p->vid_pair.first;
            uid_vid[p->uid_pair.second] = p->vid_pair.second;
            p = p->parent;
        }
        cout << endl;
        for(auto &ij:uid_vid){
            cout << ij.second <<" ";
        }
        uid_vid.clear();
    }
    cout <<endl;
    cout <<"-----------" <<endl;
}






//****************************************************************
//判断子图是否在大图中匹配
unsigned long long subgraph_total_match_num(pair<int,int> label_pair,pair<int,int> id_pair){

    unsigned long long match_num = 0 ;   //保存总共能够匹配的数量

    vector<EdgePairNode*> edgeInd_p;   //指向edgeInd里面的向量
    vector<int> affected_Q;   //暂时保存本次更新中受影响的Q
    vector<pair<int,int>> temp_queryInd_uid;  //临时保存从queryInd里面遍历得到的一条链表
    vector<pair<int,int>> temp_queryInd_uid_backup;   //temp_queryInd_uid的备份

    edgeInd_p = edgeInd[label_pair];  //指向edgeInd里面的向量

    for(auto &it:edgeInd_p){   //遍历所有edgeInd里面的向量
        for(auto &ij:it->Q_id_ptr){   //遍历所有edgeInd里面的向量的Q标志
            auto ik = find(affected_Q.begin(),affected_Q.end(),ij);
            if(ik == affected_Q.end()){
                affected_Q.push_back(ij);   //记录本次更新中受影响的Q
            }
        }
    }


    for(auto &affected_Q_item :affected_Q){   //遍历受影响的Q

        //把从queryInd里面遍历取出来的链表存起来(存的是uid，不是label)
        for(auto &pair_node_item:queryInd[affected_Q_item]){
            temp_queryInd_uid.push_back(pair_node_item.id_pair);
        }


        //把更新传进来的边(X,Y)固定在向量的第一个位置
        temp_queryInd_uid_backup = temp_queryInd_uid;

        for(int i =0 ; i < temp_queryInd_uid.size() ; i++){
            if((Q_Uid_Ulabel[temp_queryInd_uid[i].first] == label_pair.first && Q_Uid_Ulabel[temp_queryInd_uid[i].second] == label_pair.second) || (Q_Uid_Ulabel[temp_queryInd_uid[i].first] == label_pair.second && Q_Uid_Ulabel[temp_queryInd_uid[i].second] == label_pair.first)){

                bool is_reverse = false;
                if((Q_Uid_Ulabel[temp_queryInd_uid[i].first] == label_pair.second && Q_Uid_Ulabel[temp_queryInd_uid[i].second] == label_pair.first)){
                    is_reverse = true;
                }

                temp_queryInd_uid = temp_queryInd_uid_backup;
                swap(temp_queryInd_uid[0],temp_queryInd_uid[i]);


                if(is_reverse){
                    auto origin_first = temp_queryInd_uid[0].first;
                    auto origin_second = temp_queryInd_uid[0].second;
                    temp_queryInd_uid[0].first = origin_second;
                    temp_queryInd_uid[0].second = origin_first;
                }

                //对temp_queryInd_uid整形，调整一下顺序，让他能更好的计算
                vector<int> exist_uid;   //保存已经保存在里面的label
                vector<pair<int,int>> temp = temp_queryInd_uid;  //拷贝一份temp_queryInd_uid
                temp_queryInd_uid.clear();

                //把刚才固定好的边的两个顶点保存进来
                exist_uid.push_back(temp[0].first);
                exist_uid.push_back(temp[0].second);

                //开始调整次序，从第二个位置开始（因为第一个已经定住了）
                auto it = temp.begin() + 1 ;

                for(;it!=temp.end();it++){    //i是单向移动的指针，当i移动到最后的时候，循环结束（j才是工作指针）
                    auto ij = it + 1;   //j永远保持在i的后面一个位置

                    //i现在是一个边对，如果i里面任何两个顶点都在exist_label能找到，说明与i有关的边已经存进去了，则i的两个点可以直接存进去
                    if(find(exist_uid.begin(),exist_uid.end(),it->first) != exist_uid.end() || find(exist_uid.begin(),exist_uid.end(),it->second) != exist_uid.end()){
                        exist_uid.push_back(it->first);
                        exist_uid.push_back(it->second);
                        continue;
                    } else{
                        //如果i此时不能满足“在exist_label能找到”，但是j可以满足，那么就把i和j换位置，任何再把i存进去
                        for(;ij!=temp.end();ij++){
                            if(find(exist_uid.begin(),exist_uid.end(),ij->first) != exist_uid.end() || find(exist_uid.begin(),exist_uid.end(),ij->second) != exist_uid.end()){
                                swap(*it,*ij);
                                exist_uid.push_back(it->first);
                                exist_uid.push_back(it->second);
                                break;
                            }
                        }
                    }
                }

                temp_queryInd_uid = temp;

                temp.clear();
                exist_uid.clear();



                //开始计算匹配了
                pair<int,int> uid_pair_pre;   //指向上一次处理的那个uid_pair的指针
                pair<int,int> temp_uid_pair;
                pair<int,int> temp_label_pair;
                map<int,int> temp_uid_vid;   //用于临时保存query_node_map里面uid和vid的对应关系（用来固定点）
                map<pair<int,int>,vector<GmatV_Node>> query_node_map;  //用于保存uid_pair对应的G里面的vid_pair
                bool is_match = false;
                bool is_break_loop = false;

                query_node_map.clear();

                //处理query里面的边
                for(auto &uid_pair:temp_queryInd_uid){

                    temp_uid_pair = {uid_pair.first,uid_pair.second};
                    temp_label_pair = {Q_Uid_Ulabel[uid_pair.first],Q_Uid_Ulabel[uid_pair.second]};


                    //处理传进来的边
                    if(query_node_map.empty()){
                        GmatV_Node node;
                        node.vid_pair = id_pair;
                        node.uid_pair = temp_uid_pair;
                        query_node_map[temp_uid_pair].push_back(node);
                        uid_pair_pre = temp_uid_pair;
                        continue;
                    }

                    query_node_map[temp_uid_pair];  //占位置

                    //处理不是第一条边的边
                    //遍历上一次处理的那个uid_pair下面的id_pair向量
                    for(auto &pre_item:query_node_map[uid_pair_pre]){

                        //获取前面所有的uid_vid_map
                        auto *temp_parent = &pre_item;
                        auto *temp_p = &pre_item;

                        temp_uid_vid.clear();

                        //前向遍历，获取前面所有已经存在的的uid，vid对应的情况
                        while (temp_p != nullptr){
                            temp_uid_vid[temp_p->uid_pair.first] = temp_p->vid_pair.first;
                            temp_uid_vid[temp_p->uid_pair.second] = temp_p->vid_pair.second;
                            temp_p = temp_p->parent;
                        }


                        //1有2有
                        if(temp_uid_vid.find(temp_uid_pair.first) != temp_uid_vid.end() && temp_uid_vid.find(temp_uid_pair.second) != temp_uid_vid.end()){
                            auto first_vid = temp_uid_vid[temp_uid_pair.first];
                            auto second_vid = temp_uid_vid[temp_uid_pair.second];

                            //如果first能和second在G图上有连接，那么就可以直接存进去
                            if(find(G[first_vid].two_way_neighbor_id.begin(),G[first_vid].two_way_neighbor_id.end(),second_vid) != G[first_vid].two_way_neighbor_id.end()){
                                GmatV_Node node;
                                node.uid_pair = temp_uid_pair;
                                node.vid_pair = {first_vid,second_vid};
                                query_node_map[temp_uid_pair].push_back(node);
                                query_node_map[temp_uid_pair].back().parent = &pre_item;
                            }
                            continue;
                        }

                        //1有2无
                        if(temp_uid_vid.find(temp_uid_pair.first) != temp_uid_vid.end() && temp_uid_vid.find(temp_uid_pair.second) == temp_uid_vid.end()){
                            auto first_vid = temp_uid_vid[temp_uid_pair.first];
                            for(auto &G_neighbor_id_item:G[first_vid].two_way_neighbor_id){
                                if(G_Vid_Vlabel[G_neighbor_id_item] == Q_Uid_Ulabel[temp_uid_pair.second]){

                                    vector<int> temp_Q_neighbor_id;
                                    vector<int> temp_Q_neighbor_label;
                                    vector<int> temp_G_neighbor_id;
                                    vector<int> temp_G_neighbor_label;

                                    for(auto &another_Q_neighbor_id_item:Q[temp_uid_pair.second].two_way_neighbor_id){
                                        temp_Q_neighbor_id.push_back(another_Q_neighbor_id_item);
                                        temp_Q_neighbor_label.push_back(Q_Uid_Ulabel[another_Q_neighbor_id_item]);
                                    }

                                    for(auto &another_G_neighbor_id_item:G[G_neighbor_id_item].two_way_neighbor_id){
                                        temp_G_neighbor_id.push_back(another_G_neighbor_id_item);
                                        temp_G_neighbor_label.push_back(G_Vid_Vlabel[another_G_neighbor_id_item]);
                                    }


                                    bool is_Q_neighbor_label_in_G_neighbor_label = true;
                                    bool is_Q_neighbor_id_in_G_neighbor_id = true;

                                    for(auto &Q_neighbor_label_item:temp_Q_neighbor_label){
                                        if(find(temp_G_neighbor_label.begin(),temp_G_neighbor_label.end(),Q_neighbor_label_item) == temp_G_neighbor_label.end()){
                                            is_Q_neighbor_label_in_G_neighbor_label = false;
                                            break;
                                        }
                                    }


                                    if(is_Q_neighbor_label_in_G_neighbor_label){  //代表Q中全部的label都在G中找到了
                                        for(auto &Q_neighbor_id_item:temp_Q_neighbor_id){
                                            int right_vid = 0;
                                            int Q_label = Q_Uid_Ulabel[Q_neighbor_id_item];
                                            //如果这个uid已经再uid_vid_map里面了，那么就把这个vid取出来，后面定点用
                                            if(temp_uid_vid.find(Q_neighbor_id_item) != temp_uid_vid.end()){
                                                right_vid = temp_uid_vid[Q_neighbor_id_item];
                                                if(find(temp_G_neighbor_id.begin(),temp_G_neighbor_id.end(),right_vid) == temp_G_neighbor_id.end()){
                                                    is_Q_neighbor_id_in_G_neighbor_id = false;
                                                    break;
                                                }
                                            }
                                        }

                                        if(is_Q_neighbor_label_in_G_neighbor_label && is_Q_neighbor_id_in_G_neighbor_id){  //如果Q中所有在uid_vid里面的id，都能在G里里面找到
                                            GmatV_Node node;
                                            node.uid_pair = temp_uid_pair;
                                            node.vid_pair = {first_vid,G_neighbor_id_item};
                                            query_node_map[temp_uid_pair].push_back(node);
                                            query_node_map[temp_uid_pair].back().parent = &pre_item;
                                        }
                                    } else{
                                        continue;
                                    }
                                }
                            }
                            continue;
                        }

                        //1无2有
                        if(temp_uid_vid.find(temp_uid_pair.first) == temp_uid_vid.end() && temp_uid_vid.find(temp_uid_pair.second) != temp_uid_vid.end()){
                            auto second_vid = temp_uid_vid[temp_uid_pair.second];
                            for(auto &G_neighbor_id_item:G[second_vid].two_way_neighbor_id){
                                if(G_Vid_Vlabel[G_neighbor_id_item] == Q_Uid_Ulabel[temp_uid_pair.first]){
                                    vector<int> temp_Q_neighbor_id;
                                    vector<int> temp_Q_neighbor_label;
                                    vector<int> temp_G_neighbor_id;
                                    vector<int> temp_G_neighbor_label;

                                    for(auto &another_Q_neighbor_id_item:Q[temp_uid_pair.first].two_way_neighbor_id){
                                        temp_Q_neighbor_id.push_back(another_Q_neighbor_id_item);
                                        temp_Q_neighbor_label.push_back(Q_Uid_Ulabel[another_Q_neighbor_id_item]);
                                    }

                                    for(auto &another_G_neighbor_id_item:G[G_neighbor_id_item].two_way_neighbor_id){
                                        temp_G_neighbor_id.push_back(another_G_neighbor_id_item);
                                        temp_G_neighbor_label.push_back(G_Vid_Vlabel[another_G_neighbor_id_item]);
                                    }

                                    bool is_Q_neighbor_label_in_G_neighbor_label = true;
                                    bool is_Q_neighbor_id_in_G_neighbor_id = true;

                                    for(auto &Q_neighbor_label_item:temp_Q_neighbor_label){
                                        if(find(temp_G_neighbor_label.begin(),temp_G_neighbor_label.end(),Q_neighbor_label_item) == temp_G_neighbor_label.end()){
                                            is_Q_neighbor_label_in_G_neighbor_label = false;
                                            break;
                                        }
                                    }


                                    if(is_Q_neighbor_label_in_G_neighbor_label){  //代表Q中全部的label都在G中找到了
                                        for(auto &Q_neighbor_id_item:temp_Q_neighbor_id){
                                            int right_vid = 0;
                                            int Q_label = Q_Uid_Ulabel[Q_neighbor_id_item];
                                            //如果这个uid已经再uid_vid_map里面了，那么就把这个vid取出来，后面定点用
                                            if(temp_uid_vid.find(Q_neighbor_id_item) != temp_uid_vid.end()){
                                                right_vid = temp_uid_vid[Q_neighbor_id_item];
                                                if(find(temp_G_neighbor_id.begin(),temp_G_neighbor_id.end(),right_vid) == temp_G_neighbor_id.end()){
                                                    is_Q_neighbor_id_in_G_neighbor_id = false;
                                                    break;
                                                }
                                            }
                                        }

                                        if(is_Q_neighbor_label_in_G_neighbor_label && is_Q_neighbor_id_in_G_neighbor_id){  //如果Q中所有在uid_vid里面的id，都能在G里里面找到
                                            GmatV_Node node;
                                            node.uid_pair = temp_uid_pair;
                                            node.vid_pair = {G_neighbor_id_item,second_vid};
                                            query_node_map[temp_uid_pair].push_back(node);
                                            query_node_map[temp_uid_pair].back().parent = &pre_item;
                                        }
                                    } else{
                                        continue;
                                    }
                                }
                            }
                            continue;
                        }
                    }

                    if(query_node_map[temp_uid_pair].empty()){
                        break;
                    }

                    uid_pair_pre = temp_uid_pair;
                }


                is_match = true;
                for(auto &map_item:query_node_map){
                    if(map_item.second.empty()){
                        is_match = false;
                        break;
                    }
                }

                if(is_match){
                    cout << id_pair.first <<" -> " << id_pair.second << " Matched !   Q : " << affected_Q_item<<endl;
                    pair<int,int> last_query_id_pair = {temp_queryInd_uid.back().first,temp_queryInd_uid.back().second};
                    match_num = match_num + query_node_map[last_query_id_pair].size();
                    cout << "this edge matches : " << query_node_map[last_query_id_pair].size() <<endl;
                    print_match_tree(query_node_map,last_query_id_pair);

                    cout <<endl;

                }
            }
        }
    }

    return match_num;

}




//****************************************************************
//加入更新流，并更新G_matV
unsigned long long update_G_matV(const string& path_of_stream){
    unsigned long long match_num;
    ifstream infile;
    char single_data;
    int id1,id2,weight;


    pair<int,int> id_pair,reverse_id_pair;   //保存从stream读取的边对(里面是V_id)
    pair<int,int> label_pair,reverse_label_pair;  //保存从stream读取的边对(里面是V_id对应的Label_id)

    infile.open(path_of_stream);  //打开更新流文件

    if(!infile){
        cerr << "Failed To Stream Graph" << endl;  //cerr是std中的标准错误输出（和cout有区别）
        return -1;
    }

    while(infile >> single_data){
        if(single_data=='e'){
            infile >> id1 >> id2 >> weight;
            id_pair = {id1,id2};   //保存从stream读取的边对
            reverse_id_pair = {id2,id1};
            label_pair = {G_Vid_Vlabel[id_pair.first],G_Vid_Vlabel[id_pair.second]}; //查询边对所对应的label
            reverse_label_pair = {G_Vid_Vlabel[id_pair.second],G_Vid_Vlabel[id_pair.first]};

            //更新GmatV的同时，也把G图里面的邻居情况一起更新了
            if(std::find(G[id1].two_way_neighbor_id.begin(), G[id1].two_way_neighbor_id.end(), id2) == G[id1].two_way_neighbor_id.end()){
                G[id1].two_way_neighbor_id.push_back(id2);
            }

            if(std::find(G[id2].two_way_neighbor_id.begin(), G[id2].two_way_neighbor_id.end(), id1) == G[id2].two_way_neighbor_id.end()){
                G[id2].two_way_neighbor_id.push_back(id1);
            }

            //此处开始更新GmatV
            auto it = edgeInd.find(label_pair);
            auto reverse_it = edgeInd.find(reverse_label_pair);

            if(it != edgeInd.end()){   //说明待插入的边在edgeInd里面，满足插入的要求
                if((*it).first.first == (*it).first.second){
                    match_num = match_num + subgraph_total_match_num(label_pair,id_pair);
                    match_num = match_num + subgraph_total_match_num(reverse_label_pair,reverse_id_pair);
                } else{
                    auto ik = find(G_matV[label_pair].begin(),G_matV[label_pair].end(),id_pair);
                    if(ik == G_matV[label_pair].end()){     //如果没有重复的，则插入
                        G_matV[label_pair].push_back(id_pair);
                    }

                    match_num = match_num + subgraph_total_match_num(label_pair,id_pair);
                }


            } else{          //如果待插入的边不在edgeInd里面，那么就不能插入GmatV里面
                if(reverse_it != edgeInd.end()){
                    if((*reverse_it).first.first == (*reverse_it).first.second){
                        match_num = match_num + subgraph_total_match_num(label_pair,id_pair);
                        match_num = match_num + subgraph_total_match_num(reverse_label_pair,reverse_id_pair);
                    } else{
                        auto ik = find(G_matV[reverse_label_pair].begin(),G_matV[reverse_label_pair].end(),reverse_id_pair);
                        if(ik == G_matV[reverse_label_pair].end()){     //如果没有重复的，则插入
                            G_matV[reverse_label_pair].push_back(reverse_id_pair);
                        }

                        match_num = match_num + subgraph_total_match_num(reverse_label_pair,reverse_id_pair);
                    }

                }

            }
        }
    }

    infile.close();
    return match_num;
}








int main(){
    cout << "########################################################" <<endl;
//    string path_of_data_graph = R"(E:\GraphQuery C++\TestData\data-graph.txt)";
//    string path_of_query_graph = R"(E:\GraphQuery C++\TestData\multi-query.txt)";
//    string path_of_stream = R"(E:\GraphQuery C++\TestData\stream.txt)";
//
    string path_of_data_graph = R"(E:\Desktop\GraphQuery C++\Data\data.graph)";
    string path_of_query_graph = R"(E:\Desktop\GraphQuery C++\Data\Q_9 )";
    string path_of_stream = R"(E:\Desktop\GraphQuery C++\Data\insertion.graph)";

//    string path_of_data_graph = R"(E:\GraphQuery C++\Data2\data-graph.txt)";
//    string path_of_query_graph = R"(E:\GraphQuery C++\Data2\multi-query.txt)";
//    string path_of_stream = R"(E:\GraphQuery C++\Data2\stream.txt)";

    inputG(path_of_data_graph);
    inputQ(path_of_query_graph);

    cout << "********************************************************" <<endl;

    create_edge_pair_vector();  //创建边对向量

    create_Pi_chain();    //创建Pi链表

    cout << "********************************************************" <<endl;

    create_rootInd();   //创建rootInd索引

    create_queryInd();  //创建queryInd索引

    create_edgeInd();  //创建edgeInd索引

    create_G_matV();   //创建物化视图

    cout << "********************************************************" <<endl;

    unsigned long long total_match_num = update_G_matV(path_of_stream);  //添加更新边，并返回total_match_num

    cout << "Total Match Num Is : " << total_match_num << endl;

    return 0;
}