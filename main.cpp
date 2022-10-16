#include <iostream>
#include <fstream>  //文件操作库
#include <algorithm>  //find()库
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


    void clear(){
        this->first_node_in_degree = 0;  //默认入度为0，后面只需要管那些入度不为0的，不需要再让入度本身为0的节点再进行一次赋0操作
        this->second_node_out_degree = 0;
        this->Q_id = -1;
        this->child.clear();
    }
};


//*****************************************************************
//
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
vector<pair<int,int>> S; //保存更新图

vector<EdgePairNode> Pairs;   //存放所有的边对（Q1和Q2的都在里面）

map<pair<int,int>,vector<int>> edgeInd;   //无序map不能使用pair作为key，而有序map可以(key是label_pair，value是对应的节点的连接)

unordered_map<int,vector<EdgePairNode>> queryInd;   //key是Qid，value是n节点

map<unsigned long long,unsigned long long> Match_Num_Map;   //记录不同Q对应的匹配次数
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







void inputS(const string& path_of_update_stream){
    char single_data;  //定义单个字符的变量，用来储存读入的数据
    ifstream infile;
    int e_id1,e_id2,e_weight;  //用于保存e和v开头的数据
    pair<int,int> edge_node;

    infile.open(path_of_update_stream);  //打开查询图文件

    if(!infile){
        cerr << "Failed To Open Stream" << endl;  //cerr是std中的标准错误输出（和cout有区别）
        return;
    }

    while(infile >> single_data){
        if(single_data=='e'){
            infile >> e_id1 >> e_id2 >> e_weight;
            edge_node = {e_id1,e_id2};
            S.push_back(edge_node);
        }
    }
    cout << "Update Stream Loading Successfully" << endl;
}





//****************************************************************
//创建边对节点
void create_edge_pair_vector(){
    EdgePairNode e_node;
    for(auto &Q_item:Q){
        for(auto &Q_item_neighbor:Q_item.one_way_neighbor_id){
            e_node.Q_id = Q_item.Q_id;
//            e_node.Q_id_ptr.push_back(Q_item.Q_id);  //先把自己的Qid压进去
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
    for(auto &pair_item:Pairs){

        pair<int,int> label_pair = pair_item.label_pair;
        pair<int,int> reverse_label_pair = {pair_item.label_pair.second,pair_item.label_pair.first};

        if(edgeInd.find(label_pair) == edgeInd.end()){   //正着找不到
            if(edgeInd.find(reverse_label_pair) != edgeInd.end()){   //反着找到了
                if(find(edgeInd[reverse_label_pair].begin(),edgeInd[reverse_label_pair].end(),pair_item.Q_id) == edgeInd[reverse_label_pair].end()){  //如果里面没有重复的点
                    edgeInd[reverse_label_pair].push_back(pair_item.Q_id);  //把Qid保存到那个反着的地方
                }
            } else{   //反着也找不到，说明edgeInd里面没有，直接保存
                edgeInd[label_pair].push_back(pair_item.Q_id);
            }
        } else{  //正着能找到，则判断是否有重复并插进去
            if(find(edgeInd[label_pair].begin(),edgeInd[label_pair].end(),pair_item.Q_id) == edgeInd[label_pair].end()){
                edgeInd[pair_item.label_pair].push_back(pair_item.Q_id);
            }
        }
    }

    cout << "EdgeInd Create Successfully" <<endl;
}





//***************************************
//调试的时候用来输出每次可以成功匹配时候的完整Q路径
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
    vector<int> affected_Q;   //暂时保存本次更新中受影响的Q
    vector<pair<int,int>> temp_queryInd_uid;  //临时保存从queryInd里面遍历得到的一条链表
    vector<pair<int,int>> temp_queryInd_uid_backup;   //temp_queryInd_uid的备份


    for(auto &it:edgeInd[label_pair]){   //遍历所有edgeInd里面的向量
        auto ij = find(affected_Q.begin(),affected_Q.end(),it);
        if(ij == affected_Q.end()){
            affected_Q.push_back(it);   //记录本次更新中受影响的Q
        }
    }



    for(auto &affected_Q_item :affected_Q){   //遍历受影响的Q

        temp_queryInd_uid.clear();
        //把从queryInd里面遍历取出来的链表存起来(存的是uid，不是label)
        for(auto &pair_node_item:queryInd[affected_Q_item]){
            temp_queryInd_uid.push_back(pair_node_item.id_pair);
        }


        //把更新传进来的边(X,Y)固定在向量的第一个位置
        temp_queryInd_uid_backup = temp_queryInd_uid;

        int edge_count = 1;
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

                //到此，边的顺序都调整好了



                //开始计算匹配了
                pair<int,int> uid_pair_pre;   //指向上一次处理的那个uid_pair的指针
                pair<int,int> temp_uid_pair;  //用于临时保存Q中的uid_pair
                pair<int,int> temp_label_pair;  //用于临时保存Q中的label_pair
                map<int,int> temp_uid_vid;   //用于临时保存query_node_map里面uid和vid的对应关系（用来固定点）
                map<pair<int,int>,vector<GmatV_Node>> query_node_map;  //用于保存uid_pair对应的G里面的vid_pair（建立Q和G的联系）
                bool is_match = false;     //判断最终是否匹配的标志
                bool is_break_loop = false;  //判断是否结束循环的标志


                query_node_map.clear();  //每次开始处理前清理脏数据

                //处理query里面的边
                for(auto &uid_pair:temp_queryInd_uid){

                    temp_uid_pair = {uid_pair.first,uid_pair.second};
                    temp_label_pair = {Q_Uid_Ulabel[uid_pair.first],Q_Uid_Ulabel[uid_pair.second]};

                    //处理传进来的边(第一条边)
                    if(query_node_map.empty()){
                        GmatV_Node node;
                        node.vid_pair = id_pair;
                        node.uid_pair = temp_uid_pair;
                        query_node_map[temp_uid_pair].push_back(node);
                        uid_pair_pre = temp_uid_pair;    //这条条边就是下一次处理的pre边
                        continue;
                    }






                    //处理不是第一条边的边
                    //遍历上一次处理的那个uid_pair下面的id_pair向量
                    query_node_map[temp_uid_pair];  //占位置

                    for(auto &pre_item:query_node_map[uid_pair_pre]){

                        //获取前面所有的uid_vid_map
                        auto *temp_p = &pre_item;

                        temp_uid_vid.clear();

                        //前向遍历，获取前面所有已经存在的的uid，vid对应的情况
                        while (temp_p != nullptr){
                            temp_uid_vid[temp_p->uid_pair.first] = temp_p->vid_pair.first;
                            temp_uid_vid[temp_p->uid_pair.second] = temp_p->vid_pair.second;
                            temp_p = temp_p->parent;
                        }


                        //1有2有（uid1在temp_uid_vid中出现了，uid2也在temp_uid_vid中出现了）
                        if(temp_uid_vid.find(temp_uid_pair.first) != temp_uid_vid.end() && temp_uid_vid.find(temp_uid_pair.second) != temp_uid_vid.end()){
                            auto first_vid = temp_uid_vid[temp_uid_pair.first];
                            auto second_vid = temp_uid_vid[temp_uid_pair.second];

                            //如果first能和second在G图上有连接，那么就可以直接存进去
                            if(find(G[first_vid].two_way_neighbor_id.begin(),G[first_vid].two_way_neighbor_id.end(),second_vid) != G[first_vid].two_way_neighbor_id.end()){
                                GmatV_Node node;
                                node.uid_pair = temp_uid_pair;
                                node.vid_pair = {first_vid,second_vid};
                                query_node_map[temp_uid_pair].push_back(node);
                                query_node_map[temp_uid_pair].back().parent = &pre_item;  //最后一个（back()函数）压进去的点，就是刚才处理的点
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

                                    //获取该uid在Q中的所有邻居的id和label
                                    for(auto &another_Q_neighbor_id_item:Q[temp_uid_pair.second].two_way_neighbor_id){
                                        temp_Q_neighbor_id.push_back(another_Q_neighbor_id_item);
                                        temp_Q_neighbor_label.push_back(Q_Uid_Ulabel[another_Q_neighbor_id_item]);
                                    }

                                    //获取该vid在G中的所有邻居的id和label
                                    for(auto &another_G_neighbor_id_item:G[G_neighbor_id_item].two_way_neighbor_id){
                                        temp_G_neighbor_id.push_back(another_G_neighbor_id_item);
                                        temp_G_neighbor_label.push_back(G_Vid_Vlabel[another_G_neighbor_id_item]);
                                    }


                                    bool is_Q_neighbor_label_in_G_neighbor_label = true;
                                    bool is_Q_neighbor_id_in_G_neighbor_id = true;

                                    //检查这个顶点的所有在Q中邻居的label集合，是否“都”能在这个顶点的所有在G中邻居的label集合中找到
                                    for(auto &Q_neighbor_label_item:temp_Q_neighbor_label){
                                        if(find(temp_G_neighbor_label.begin(),temp_G_neighbor_label.end(),Q_neighbor_label_item) == temp_G_neighbor_label.end()){
                                            is_Q_neighbor_label_in_G_neighbor_label = false;   //如果有一个找不到，就把标志置false
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

                    if(query_node_map[temp_uid_pair].empty()){  //如果此次处理后，query_node_map[temp_uid_pair]里面没有东西，说明匹配失败
                        break;
                    }

                    uid_pair_pre = temp_uid_pair;    //如果没有执行上面的那个break，说明就目前来说没有匹配失败，就让当前的pair成为下一次处理的pre_pair
                }




                is_match = true;
                for(auto &map_item:query_node_map){
                    if(map_item.second.empty()){   //如果query_node_map里面没有空着的点，说明至少有一条路径能够匹配成功，反之则否
                        is_match = false;
                        break;
                    }
                }

                if(is_match){
                    pair<int,int> last_query_id_pair = {temp_queryInd_uid.back().first,temp_queryInd_uid.back().second};
                    match_num = match_num + query_node_map[last_query_id_pair].size();
                    Match_Num_Map[affected_Q_item] = Match_Num_Map[affected_Q_item] + query_node_map[last_query_id_pair].size();
//                    cout << id_pair.first <<" -> " << id_pair.second << " Matched !   Q : " << affected_Q_item<<endl;
//                    cout << "this edge matches : " << query_node_map[last_query_id_pair].size() <<endl;
//                    print_match_tree(query_node_map,last_query_id_pair);
//                    cout <<endl;
                }
            }
        }
    }

    return match_num;

}






//****************************************************************
//加入更新流，并更新G_matV
unsigned long long update_edge(){
    unsigned long long match_num;
    pair<int,int> id_pair,reverse_id_pair;   //保存从stream读取的边对(里面是V_id)
    pair<int,int> label_pair,reverse_label_pair;  //保存从stream读取的边对(里面是V_id对应的Label_id)

    for(auto &update_item:S){  //遍历所有的更新边
        int id1 = update_item.first;
        int id2 = update_item.second;

        id_pair = {id1,id2};   //保存从stream读取的边对
        reverse_id_pair = {id2,id1};
        label_pair = {G_Vid_Vlabel[id_pair.first],G_Vid_Vlabel[id_pair.second]}; //查询边对所对应的label
        reverse_label_pair = {G_Vid_Vlabel[id_pair.second],G_Vid_Vlabel[id_pair.first]};

        //把G图里面的邻居情况一起更新了
        if(std::find(G[id1].two_way_neighbor_id.begin(), G[id1].two_way_neighbor_id.end(), id2) == G[id1].two_way_neighbor_id.end()){
            G[id1].two_way_neighbor_id.push_back(id2);
        }

        if(std::find(G[id2].two_way_neighbor_id.begin(), G[id2].two_way_neighbor_id.end(), id1) == G[id2].two_way_neighbor_id.end()){
            G[id2].two_way_neighbor_id.push_back(id1);
        }

        //此处开始检查是否有匹配
        auto it = edgeInd.find(label_pair);
        auto reverse_it = edgeInd.find(reverse_label_pair);

        if(it != edgeInd.end()){   //说明待插入的边在edgeInd里面，满足插入的要求
            if((*it).first.first == (*it).first.second){
                match_num = match_num + subgraph_total_match_num(label_pair,id_pair);
                match_num = match_num + subgraph_total_match_num(reverse_label_pair,reverse_id_pair);
            } else{
                match_num = match_num + subgraph_total_match_num(label_pair,id_pair);
            }
        } else{      //如果待插入的边不在edgeInd里面，那么就不能插入GmatV里面
            if(reverse_it != edgeInd.end()){
                if((*reverse_it).first.first == (*reverse_it).first.second){
                    match_num = match_num + subgraph_total_match_num(label_pair,id_pair);
                    match_num = match_num + subgraph_total_match_num(reverse_label_pair,reverse_id_pair);
                } else{
                    match_num = match_num + subgraph_total_match_num(reverse_label_pair,reverse_id_pair);
                }
            }

        }
    }

    return match_num;
}







//主程序入口
int main(){
    cout <<"################################"<<endl;
    cout << "Loading Data ..." <<endl;

    string path_of_data_graph = R"(E:\Desktop\GraphQuery C++\Data\data.graph)";
    string path_of_query_graph = R"(E:\Desktop\GraphQuery C++\Data\Q_multi)";  //multi
    string path_of_update_stream = R"(E:\Desktop\GraphQuery C++\Data\insertion.graph)";

    inputG(path_of_data_graph);  //读取数据图
    inputQ(path_of_query_graph);  //读取查询图
    inputS(path_of_update_stream); //读取更新图（先把更新图保存起来，避免后期频繁IO操作读取文件）

    create_edge_pair_vector();  //创建边对向量

    create_queryInd();  //创建queryInd索引

    create_edgeInd();  //创建edgeInd索引

    cout <<"################################"<<endl;

    cout << endl;

    cout <<"################################"<<endl;
    cout << "Matching ..." <<endl;

    unsigned long long total_match_num = update_edge();  //添加更新边，并返回total_match_num


    cout << "Print Result : " <<endl;
    cout <<"################################"<<endl;

    for(auto &it:Match_Num_Map){
        cout << "Q_"<< it.first << " Match Num is : " <<it.second <<endl;
    }

    cout << "Total Match Num Is : " << total_match_num << endl;

    return 0;
}
