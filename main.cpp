#include <iostream>
#include <fstream>  //文件操作库
#include <algorithm>  //find_if库
#include "class.h"   //导入类头文件
using namespace std;



//*****************************************************************
vector<vertex_G_Node> G;   //初始化数据图:G用向量存
vector<vertex_Q_Node> Q;   //初始化多重查询图

vector<EdgePairNode> Pairs;   //存放所有的边对（Q1和Q2的都在里面）
vector<PiChain> P;   //存放所有Pi的向量（同一个Q_id的Pi存放在同一个向量里）
unordered_map<int,PiChain> PTrees;   //key是Ti的编号，value是Pi链

//*****************************************************************






//**********************函数的定义*******************************************

//加载数据图
void inputG(const string& path_of_data_graph){
    char single_data;  //定义单个字符的变量，用来储存读入的数据
    ifstream infile;   //读取文件的数据流
    int id,id1,id2,label,weight;
    vertex_G_Node v;

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

    cout << "Data Graph Loading Successfully" << endl;
}



//****************************************************************
//加载查询图
void inputQ(const string& path_of_query_graph){
    char single_data;  //定义单个字符的变量，用来储存读入的数据
    ifstream infile;
    int v_id,e_id1,e_id2,v_label,e_weight;  //用于保存e和v开头的数据
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
            infile >> v_id >> v_label;  //依次保存节点id和节点标签
            u.save_Q_node_info(Q_id,total_v_num,v_id,v_label);
            Q.push_back(u);   //以Q_id为key，把u依次压入向量中
        }else if(single_data=='e'){
            infile >> e_id1 >> e_id2 >> e_weight;
            Q[e_id1].add_Q_neighbor(e_id2,Q[e_id2].v_label);   //Q为有向图，所以存储结果为id1 -> id2
                                                //注：此处 ↑ 可以直接Q[e_id2]取label的原因：因为是先读取v再读取e的，所以此时所有v的信息都已经读取进去了
        }
    }

    cout << "Query Graph Loading Successfully" << endl;
}



//****************************************************************
//创建边对节点
void create_edge_pair_vector(){
    EdgePairNode e_node;
    for(auto it = Q.begin() ; it != Q.end() ; it++){   //遍历所有节点
        for(auto j = 0 ; j < (*it).neighbor_id.size() ; j++){  //遍历节点的所有邻居
            e_node.Q_id = (*it).Q_id;
            e_node.edge_pair = {(*it).v_id,(*it).neighbor_id[j]};
            e_node.label_pair = {(*it).v_label,(*it).neighbor_label[j]};
            Pairs.push_back(e_node);
        }
    }


    //现在，所有的边对都保存了，接下来是去除不符合要求的边对
    //统计所有节点的入度
    for(auto i = 0 ; i < Pairs.size() ; i++){
        for(auto j = 0 ; j < Pairs.size() ; j++){
            //如果边1与所有节点的边2无重复，并且还都是在一个Q里面，则说明该节点的入度为0
            if(Pairs[i].Q_id == Pairs[j].Q_id){
                //统计第一个节点的入度
                if(Pairs[i].edge_pair.first == Pairs[j].edge_pair.second){
                    Pairs[i].first_node_in_degree++;
                }

                //统计第二个节点的出度
                if(Pairs[i].edge_pair.second == Pairs[j].edge_pair.first){
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
    for(auto i = 0 ; i < Pairs.size() ; i++){  //找出能连起来的边对，将其保存到链表
        for(auto j = 0 ; j < Pairs.size() ; j++){
            //如果能连起来，而且同一个Q
            if(Pairs[i].Q_id == Pairs[j].Q_id){
                //如果能连起来
                if(Pairs[i].edge_pair.second == Pairs[j].edge_pair.first){
                    Pc.add_node(Pairs[i]);   //把能连起来的边表保存
                    Pc.length = Pc.length+1;    //每添加一个节点，就让length加一
                    if(Pairs[j].second_node_out_degree == 0){  //如果最后一个点出度为0，那这个点就是最后一个点
                        Pairs[j].Q_id_ptr.push_back(Pairs[j].Q_id);   //如果这个节点是最后一个点，那么它一定是rootInd中有{Q}标志的那个
                        Pc.add_node(Pairs[j]);   //把最后这个节点保存再链表中
                        Pc.Q_id = Pairs[i].Q_id;   //保存好Q_id
                        Pc.length = Pc.length + 1;
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
                    Pairs[i].Q_id_ptr.push_back(Pairs[i].Q_id);  //如果这个节点是最后一个点，那么它一定是rootInd中有{Q}标志的那个
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



//N叉树的遍历(递归遍历)
void add_Nid(EdgePairNode *root , int &next_N_id){
    root->node_id = next_N_id++;      //先给root节点加Nid
    vector<EdgePairNode *> p = root->child;    //【还可以这样】让p一次性指向root所有的孩子节点
    for(auto i = 0 ; i < p.size() ; i++){
        if(!p[i]->child.empty()){    //如果p[i]的孩子不为空
            add_Nid(p[i],next_N_id);
        } else{                     //如果p[i]没有孩子
            p[i]->node_id = next_N_id++;
        }
    }
}



//TODO:上传github，画好流程图，画好UML图
//****************************************************************
//建立rootInd索引
void create_rootInd(){
    int N_id = 0;   //为每棵树上的节点添加ni
    PiChain P1;  //P1的作用就是临时获取Pi链表
    int Tree_id = 0; //用于判定在map的第几个位置上新建一颗树
    EdgePairNode *Tp;   //指向树Ti的指针
    EdgePairNode *Pp;   //指向P1链表的指针
    bool is_create_a_new_tree;  //用于控制是否新建一棵树的开关

    for(auto i = 0 ; i < P.size() ; i++){
        P1 = P[i];   //P1的作用就是临时获取Pi链表
        if(PTrees.empty()){   //如果map为空，那么就让第一条Pi直接存到map里面去
            PTrees.insert(pair<int,PiChain>{Tree_id,P1});
            Tree_id++;   //Tree_id++是让下一次从第二个位置开始建树
        }
        else{
            if(P1.length == 1){   //如果是一条单边，也可以直接新建一棵树存进去
                PTrees.insert(pair<int,PiChain>{Tree_id,P1});
                Tree_id++;
            }
            else{    //如果长度大于1，就挨个遍历看看有没有起点一样的
                //先把Tp和Pp两个指针定好位（先检查所有已经存在的树的第一个节点是否相同）
                for(auto j = 0 ; j < Tree_id ; j++){
                    if(PTrees[j].head->label_pair == P1.head->label_pair){  //如果找到了起点相同的，就不用往后找了
                        is_create_a_new_tree = false;   //不用新建一棵树
                        Tp = PTrees[j].head;
                        Pp = P1.head;
                        break;
                    }
                    else{
                        is_create_a_new_tree = true; //需要新建一棵树
                    }
                }


                if(is_create_a_new_tree == true){   //如果最后它的值都是true，那么就表示，整个for循环之中没有找到起点相同的节点
                    PTrees.insert(pair<int,PiChain>{Tree_id,P1});
                    Tree_id++;
                } else{  //如果最后它的值都是false，说明找到了起点相同的节点，而且定位好了两个指针
                    Pp = Pp->child[0];
                    while(!Tp->child.empty()){   //如果child向量为空，说明该节点是叶节点
                        //TODO:这里还没有考虑如果树的长度比P1短的情况，后续加上
                        for(auto ik = Tp->child.begin() ; ik != Tp->child.end() ; ik++){  //遍历所有孩子节点
                            if((*ik)->label_pair == Pp->label_pair){   //这里其实是一个广度搜索BFS！
                                if(Pp->child.empty()){   //如果到了P1的最后一个节点
                                    (*ik)->Q_id_ptr.push_back(Pp->Q_id);   //把有公共节点的另一条链表的Qid存进去
                                } else{   //如果P1还没有到最后
                                    Pp = Pp->child[0];
                                    Tp = (*ik);
                                    break;   //因为在BFS中找到了相同的节点，所以不用再往后面继续搜索了
                                }
                            } else{  //当遇到不相同的地方，后面就不用再继续了，直接这个节点以及该节点后面所有的节点全部压进去
                                Tp->child.push_back(Pp);
                                break;
                            }
                        }
                        break;   //如果上面那个for循环里面没有满足if条件，就说明只有第一个节点符合，后面都不符合，就没必要再继续执行while了
                    }
                }
            }
        }
    }

    //此时，已经建好了树，但是还没有每个节点n的信息和Q的标识符，下面的代码将对这两者进行添加
    for(auto it = PTrees.begin() ; it != PTrees.end() ; it++){   //遍历每棵树
        add_Nid((*it).second.head,N_id);
    }

    cout << "RootInd Create Successfully" << endl;
}




int main(){
    string path_of_data_graph = "E:\\QueryC++\\data-graph.txt";
    string path_of_query_graph = "E:\\QueryC++\\multi-query.txt";
    inputG(path_of_data_graph);
    inputQ(path_of_query_graph);

    create_edge_pair_vector();  //创建边对向量

    create_Pi_chain();    //创建Pi链表

    create_rootInd();   //创建rootInd索引


    return 0;
}