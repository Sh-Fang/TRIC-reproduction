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
class vertex_G_Node{
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
//Q为有向图
class vertex_Q_Node{
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
//边对节点
class EdgePairNode{
public:
    int Q_id;      //即：这条边属于哪个Q
    vector<int> Q_id_ptr;  //即：{Q}这种标识符，用empty来判断是否有元素
    int first_node_in_degree;   //边对中第一个节点的入度 <D,E>
    int second_node_out_degree; //边对中第二个节点的出度
    pair<int,int> label_pair;
    pair<int,int> edge_pair;
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





//*****************************************************************
vector<vertex_G_Node> G;   //初始化数据图:G用向量存
unordered_map<int,int> G_Vid_Vlabel;   //G中所有节点v_label和v_id的对应关系(key是V_id)
vector<vertex_Q_Node> Q;   //初始化多重查询图

vector<EdgePairNode> Pairs;   //存放所有的边对（Q1和Q2的都在里面）

vector<PiChain> P;   //存放所有Pi的向量（同一个Q_id的Pi存放在同一个向量里）
unordered_map<int,PiChain> PTrees;   //key是Ti的编号，value是Pi链

map<pair<int,int>,vector<EdgePairNode*>> edgeInd;   //无序map不能使用pair作为key，而有序map可以(key是label_pair，value是对应的节点的连接)

map<pair<int,int>,vector<pair<int,int>>> G_matV;    //key是label_pair，value是顶点对
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


    //建立G中所有节点v_label和v_id的对应关系
    for(auto & it : G){   //遍历G的所有节点
        G_Vid_Vlabel[it.id] = it.label;
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
            e_node.Q_id_ptr_Push_Back((*it).Q_id);   //先把自己的Qid压进去
            e_node.edge_pair = {(*it).v_id,(*it).neighbor_id[j]};
            e_node.label_pair = {(*it).v_label,(*it).neighbor_label[j]};
            Pairs.push_back(e_node);
            e_node.clear();
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
                if(Pairs[p].edge_pair.second == Pairs[q].edge_pair.first){
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




//****************************************************************
//N叉树的遍历(递归遍历，建立T与e的索引连接)
void create_T_e_index(EdgePairNode *root){
    if(root->child.empty()){   //处理那些一棵树上只有一个节点的树
        edgeInd[root->label_pair].push_back(root);
        return;
    }

    //剩下的就是那些长度大于1的树
    edgeInd[root->label_pair].push_back(root);   //因为是递归遍历，所以第一步就是把当前节点保存一下
    vector<EdgePairNode *> p = root->child;   //获取所有的孩子节点

    for(auto i = 0 ; i < p.size() ; i++){    //遍历所有的孩子节点
        if(!p[i]->child.empty()){      //如果p[i]下面还有孩子
            create_T_e_index(p[i]);   //继续递归遍历
        } else{                        //如果p[i]已经是叶节点了
            edgeInd[p[i]->label_pair].push_back(p[i]);
        }
    }

}



//建立edgeInd索引
void create_edgeInd(){
    for(auto & PTree : PTrees){   //遍历每棵树
        create_T_e_index(PTree.second.head);
    }

    cout << "EdgeInd Create Successfully" <<endl;
}


//****************************************************************
//对初始数据图建立MatV视图
void create_G_matV(){
    for(auto & it : edgeInd){
        for(auto j = 0 ; j < G.size() ; j++){
            if(it.first.first == G[j].label){   //注意：it里面所有的值是label_id，不是v_id
                for(auto k = G[j].neighbor.begin() ; k != G[j].neighbor.end() ; k++){   //遍历G[j]所有的邻居
                    if(it.first.second == G_Vid_Vlabel[(*k)]){  //查询G_Vid_Vlabel表，找出*k对应的label
                        pair<int,int> temp = {G[j].id,(*k)};   //因为*k是G[j].neighbor，也就是v_id，所以存的时候，temp的第二个参数是(*k)，而不是G[j].neighbor[*k]
                        G_matV[it.first].push_back(temp);
                    }
                }
            }
        }
    }

    cout << "G_matV Create Successfully" <<endl;
}



//****************************************************************
//找出受更新流影响的边的查询Qids（可能有多个Qid受更新的影响）
void find_affected_Q(pair<int,int> label_pair){
    vector<int> affectedQids;
    auto it = edgeInd.find(label_pair);
    if(it != edgeInd.end()){   //说明在edgeInd中找到了与之相同的节点
        for(auto & j :(*it).second){   //遍历(*it).second，也就是vector<EdgePairNode *>
            for(auto & k:(*j).Q_id_ptr){
                auto m = find(affectedQids.begin(),affectedQids.end(),k);
                if(m == affectedQids.end()){   //如果压入的id不重复，那么就压进去
                    affectedQids.push_back(k);
                }
            }
        }
    } else{
        cout << "Not Affected Q" << endl;
    }

    for(auto &i:affectedQids){
        cout << "Affected Q is : " << i << endl;
    }
}



//****************************************************************
//加入更新流，并更新G_matV
void update_G_matV(const string& path_of_stream){   //stream的格式是"e 5 7 0"，所以pair里面是v_id
    pair<int,int> new_pair;   //保存从stream读取的边对

    char single_data;
    int id1,id2,weight;

    ifstream infile;

    infile.open(path_of_stream);  //打开更新流文件

    if(!infile){
        cerr << "Failed To Stream Graph" << endl;  //cerr是std中的标准错误输出（和cout有区别）
        return;
    }

    while(infile >> single_data){
        if(single_data=='e'){   //如果遇到了边
            infile >> id1 >> id2 >> weight;
            new_pair = {id1,id2};   //保存从stream读取的边对
        }
    }


    pair<int,int> label_pair = {G_Vid_Vlabel[new_pair.first],G_Vid_Vlabel[new_pair.second]};
    G_matV[label_pair].push_back(new_pair);

    cout << "Update Successfully : " << new_pair.first << " -> " << new_pair.second <<endl;

    find_affected_Q(label_pair);
}







int main(){
    cout << "########################################################" <<endl;
    string path_of_data_graph = "E:\\GraphQuery C++\\data-graph.txt";
    string path_of_query_graph = "E:\\GraphQuery C++\\multi-query.txt";
    string path_of_stream = "E:\\GraphQuery C++\\stream.txt";

    inputG(path_of_data_graph);
    inputQ(path_of_query_graph);

    cout << "********************************************************" <<endl;

    create_edge_pair_vector();  //创建边对向量

    create_Pi_chain();    //创建Pi链表

    cout << "********************************************************" <<endl;

    create_rootInd();   //创建rootInd索引

    create_edgeInd();

    create_G_matV();

    cout << "********************************************************" <<endl;

    update_G_matV(path_of_stream);

    return 0;
}