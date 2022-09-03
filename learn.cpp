#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>

using namespace std;
using u_set = unordered_set<int>;
using vec = vector<int>;

// ====================      定义基本数据结构      ====================

struct vertex_Q{
    int label;
    u_set nei;
    unordered_map<int, vec> rep_nei;  // 存储每个查询结点的同标签邻居【是个vector比较好，后面要按照顺序检查】
    //unordered_map<int, u_set> nei_lb;  // 存储邻居的各标签数量，同样>=0表示出边,<0表示入边
    // value的信息都是>=0
};

struct vertex_G{
    int label;
    u_set nei;
    unordered_map<int, unordered_map<int, u_set>> cand;  // 数据结点v的候选查询结点及其邻居信息
    // 第一层key - 候选查询结点
    // 第二层key - 邻居
    // 第二层value - 邻居的候选结点
    unordered_map<int, bool> LI;
};

// 一些声明
vector<vertex_Q> Q;
vector<vertex_G> G;
int Q_size, G_size;
vec update;  // 更新流
unordered_map<int, vec> labels;  // 存储查询图中各标签对应结点

// 辅助函数——判断是否在vector当中
bool isInVec(int a, vec& v){
    for(int i=0; i<v.size(); i++){
        if(a == v[i]) return 1;
    }
    return 0;
}
// 辅助函数——求交
u_set intersection(u_set& us1, u_set& us2){  //todo:返回引用
    u_set result;
    if(us1.size()<=us2.size()){
        for(auto& item : us1){
            if(us2.find(item)!=us2.end()) result.insert(item);
        }
    }
    else{
        for(auto& item : us2){
            if(us1.find(item)!=us1.end()) result.insert(item);
        }
    }
    return result;
}
// 辅助函数——输出
void dumpG(const string& path){
    ofstream outfile(path);
    for(int vi=0; vi<G_size; vi++){
        for(auto& candi : G[vi].cand){
            int ui = candi.first;
            if(G[vi].LI[ui]){
                outfile << "v" << vi << "-u" << ui << " nei:";
                for(auto& nei : candi.second){
                    int uj = nei.first;
                    outfile << " [u" << uj << "]";
                    for(int vj : nei.second){
                        if(G[vj].LI[uj]) outfile << " v" << vj;
                    }
                }
                outfile << endl;
            }
        }
    }
    outfile.close();
}


// ====================      读取查询图的数据      ====================

void inputQ(string& qp){
    // 输入流
    ifstream qg(qp);
    if(!qg) cerr << "Fail to open query file." << endl;

    char c;
    int id, id1, id2, lb, dvir;  // TODO: dvir不知道是干嘛的
    vertex_Q u;
    while(qg >> c){
        if(c == 'v'){
            qg >> id >> lb;
            if(lb == -1) lb = 0;
            u.label = lb;
            Q.push_back(u);
        }
        else{
            qg >> id1 >> id2;
            Q[id1].nei.insert(id2);
            Q[id2].nei.insert(id1);
            break;
        }
    }
    while(qg >> c){
        qg >> id1 >> id2;
        Q[id1].nei.insert(id2);
        Q[id2].nei.insert(id1);
    }
    qg.close();
    Q_size = Q.size();

//    // 更新查询图的nei_lb信息，即各结点的所有邻居标签的数量
//    for(int ui=0; ui<Q_size; ui++){
//        for(auto& nei : Q[ui].nei){
//            int nei_lb;
//            if(nei>=0){
//                nei_lb = Q[nei].label;
//                Q[ui].nei_lb[nei_lb].insert(nei);
//            }
//            else{
//                nei_lb = -Q[nei].label -1;
//                Q[ui].nei_lb[nei_lb].insert(-nei-1);
//            }
//        }
//    }

    // 将查询图的所有标签存放在labels中
    for(int ui=0; ui<Q_size; ui++){
        labels[Q[ui].label].emplace_back(ui);  // 将每个标签对应的查询结点保存
        // 处理同标签邻居
        unordered_map<int, vec> rep_nei;
        for(auto& uni : Q[ui].nei){
            int& uni_label = Q[uni].label;
            rep_nei[uni_label].emplace_back(uni);
        }
        for(auto& repi : rep_nei){
            if(repi.second.size()>1){
                Q[ui].rep_nei[repi.first] = repi.second;
            }
        }
    }
}

// 生成核结点、壳结点和matchingorder
struct split{
    vec core;
    vector<u_set> core_nei;
    unordered_map<int, u_set> c_s_nei;  // 核结点的壳邻居
    vec shell;
    vector<u_set> shell_nei;
};

unordered_map<int, unordered_map<int, split>> matching_order;

// 将i的所有未访问邻居加入到temp当中
void insertNei(u_set &temp, int i, int ui, int uj, split& uj_second){
    for(auto& nei : Q[i].nei){
        int n = nei;
        if(n!=ui && n!=uj && !isInVec(n, uj_second.core) && !isInVec(n, uj_second.shell)){
            temp.insert(n);
        }
    }
}

// 判断i的邻居是否全部为核结点
bool neiAllCore(int i, split& uj_second, int ui, int uj){
    for(auto& nei : Q[i].nei){
        int n = nei;
        if(n!=ui && n!=uj && !isInVec(n, uj_second.core)){
            return false;
        }
    }
    return true;
}

void generateMO(){
    for(int ui=0; ui<Q_size; ui++) {
        unordered_map<int, split> ui_first;
        for (auto &uj : Q[ui].nei) {
            if (uj >= 0) {
                split uj_second;
                u_set temp;  // 临时存储未分类的结点
                // 首先将ui, uj的邻居加入temp当中
                insertNei(temp, ui, ui, uj, uj_second);
                insertNei(temp, uj, ui, uj, uj_second);
                while(!temp.empty()){
                    // 首先将邻居均为核结点的加入到壳结点当中
                    u_set temp2 = temp;
                    for(auto& ti : temp2){
                        if(neiAllCore(ti, uj_second, ui, uj)){
                            u_set nei_info = Q[ti].nei;
                            int v_core;
                            for(int & i : uj_second.core){
                                if(nei_info.count(i)){
                                    v_core = i;
                                }
                            }
                            uj_second.c_s_nei[v_core].insert(uj_second.shell.size());
                            uj_second.shell.push_back(ti);
                            uj_second.shell_nei.push_back(nei_info);
                            temp.erase(ti);
                        }
                    }
                    // 然后加入一个核结点
                    if(!temp.empty()){
                        int nei_num=0;
                        int core_v;
                        for(auto& t : temp){
                            u_set nei_noij = Q[t].nei;
                            nei_noij.erase(ui);
                            nei_noij.erase(uj);
                            if(nei_noij.size() > nei_num){
                                nei_num = nei_noij.size();
                                core_v = t;
                            }
                        }
                        uj_second.core.push_back(core_v);
                        u_set nei_info;
                        for(auto& nei : Q[core_v].nei){
                            int n = nei;
                            if(n == ui || n == uj || isInVec(n, uj_second.core)){
                                nei_info.insert(nei);
                            }
                        }
                        uj_second.core_nei.push_back(nei_info);
                        insertNei(temp, core_v, ui, uj, uj_second);
                        temp.erase(core_v);
                    }
                }
                ui_first[uj] = uj_second;
            }
        }
        matching_order[ui] = ui_first;
    }
}

void inputG(string& gp){
    // 输入流
    ifstream dg(gp);
    if(!dg) cerr << "Fail to open graph file." << endl;

    char c;
    int id, id1, id2, lb;
    vertex_G v;
    while(dg >> c){
        if(c == 'v'){
            dg >> id >> lb;
            if(labels.count(lb))  v.label = lb;
            else  v.label = -1;  // 与查询结点无关的数据结点可以跳过
            G.push_back(v);
        }
        else{
            G_size = G.size();
            dg >> id1 >> id2;
            if(id1<G_size && id2<G_size && G[id1].label!=-1 && G[id2].label!=-1){  // 与查询结点无关的数据结点可以跳过
                G[id1].nei.insert(id2);
                G[id2].nei.insert(id1);
            }
            break;
        }
    }
    while(dg >> c){
        dg >> id1 >> id2;
        if(id1<G_size && id2<G_size && G[id1].label!=-1 && G[id2].label!=-1){
            G[id1].nei.insert(id2);
            G[id2].nei.insert(id1);
        }
    }
    dg.close();
}


// ====================   构建数据图的查询结点候选   ====================

void constructCand(){
    // 先将查询图的每个结点邻居信息存储下来，这样nei_info表示的是查询结点邻居标签的数量
    // 出边邻居的标签和入边邻居的标签如果一样的话，分别存储为>=0和<0的值
    // e.g. [{0:1}, {0:1, -1:1}, {-1:1}] 表示三个结点，标签都为0，结构为 0->1->2
    for(int vi=0; vi<G_size; vi++){
        int& lb = G[vi].label;
        if(lb!=-1){
            for(auto& ui: labels[lb]){
                unordered_map<int, u_set> ui_cand;
                for(auto& vj : G[vi].nei){
                    int vj_lb = G[vj].label;
                    for(auto& uj : Q[ui].nei){
                        if(uj>=0 && vj>=0 && Q[uj].label==vj_lb){  // todo: 这边或许判断可以减少
                            ui_cand[uj].insert(vj);
                        }
                    }
                }
                G[vi].cand[ui] = ui_cand;
                G[vi].LI[ui] = 1;
            }
        }
    }
    //dumpG("./dump/G_init");
}


// ====================        熄灭与点亮        ====================
bool tryNei(int th, int vi, int ui, u_set& used, vec& to_check){
    // 如果不满足匹配，则返回0；否则返回1
    if(th==to_check.size()){
        return 1;
    }
    else{
        auto& uj = to_check[th];
        for(auto& vj : G[vi].cand[ui][uj]){
            if(G[vj].label!=Q[uj].label) {
                G[vj].cand.erase(uj);
                G[vj].LI.erase(uj);
                continue;
            }
            if(used.find(vj)==used.end()){
                used.insert(vj);
                if(tryNei(th+1, vi, ui, used, to_check)) return 1;
                used.erase(vj);
            }
        }
        return 0;
    }
}

bool checkNei(int vi, int ui){  // todo: 是否可以用匈牙利算法
    // 先检查各邻居的候选是否为空，若是则直接返回0
    for(auto& uj : Q[ui].nei){
        if(G[vi].cand[ui].find(uj)==G[vi].cand[ui].end()) return 0;
        else if(!G[vi].cand[ui][uj].empty()) continue;
        else return 0;
    }

    // 检查每一组同标签邻居
    for(auto& rep_nei : Q[ui].rep_nei){
        u_set used;
        if(!tryNei(0, vi, ui, used, rep_nei.second)) return 0;
    }
    return 1;
}

// 熄灭传播过程
void deleteAndCheck(int ui, int vi){

    for(auto& nei : G[vi].cand[ui]){
        int uj = nei.first;
        for(auto& vj : nei.second){
            if(G[vj].label!=Q[uj].label) {
                G[vj].cand.erase(uj);
                G[vj].LI.erase(uj);
                continue;
            }
            if(G[vj].LI[uj]) {
                // 因为如果vi的邻居vj已经熄灭了，则那个邻居vj在熄灭的时候就会来vi这里将vj删去
                G[vj].cand[uj][ui].erase(vi);
                if (G[vj].cand[uj][ui].empty()) {
                    G[vj].LI[uj] = 0;
                    deleteAndCheck(uj, vj);
                }
                // 如果熄灭点是同标签的邻居之一，则需要检查该标签的邻居是否仍然能够满足
                else{
                    auto lb = Q[ui].label;
                    if (Q[uj].rep_nei.find(lb)!=Q[uj].rep_nei.end()){
                        auto& rep_nei = Q[uj].rep_nei[lb];
                        u_set used;
                        if(!tryNei(0, vj, uj, used, rep_nei)){
                            G[vj].LI[uj] = 0;
                            deleteAndCheck(uj, vj);
                        }
                    }
                }
            }
        }
    }
}

void turnOff(int& vi){
    for(auto& candi : G[vi].cand){
        auto& ui = candi.first;
        if(G[vi].label!=Q[ui].label) {
            G[vi].cand.erase(ui);
            G[vi].LI.erase(ui);
            continue;
        }
        if(!G[vi].LI[ui]) continue; // 本身是熄灭状态，就不需要考虑会被熄灭
        if ( !checkNei(vi, ui) ){
            G[vi].LI[ui] = 0;
            deleteAndCheck(ui, vi);
        }
    }
}


void turnOffProcess(int v1, int v2){
    G[v1].nei.erase(v2);
    G[v2].nei.erase(v1);
    for(auto& candi : G[v1].cand){
        int ui = candi.first;
        if(G[v1].label!=Q[ui].label){
            G[v1].cand.erase(ui);
            G[v1].LI.erase(ui);
            continue;
        }
        for(auto& ui_nei : candi.second){
            int uj = ui_nei.first;
            if(Q[uj].label==G[v2].label){
                G[v1].cand[ui][uj].erase(v2);
                G[v2].cand[uj][ui].erase(v1);
            }
        }
    }

    turnOff(v1);
    turnOff(v2);
}

void addAndCheck(int ui, int vi, vec& temp_v, vec& temp_u){
    // ui-vi表示点亮的结点
    for(auto& nei : G[vi].cand[ui]){
        int uj = nei.first;
        for(auto& vj : nei.second){
            if(G[vj].label!=Q[uj].label) {
                G[vj].cand.erase(uj);
                G[vj].LI.erase(uj);
                continue;
            }
            G[vj].cand[uj][ui].insert(vi);
            if(!G[vj].LI[uj]){
                if(checkNei(vj, uj)){
                    G[vj].LI[uj] = 1;
                    addAndCheck(uj, vj, temp_v, temp_u);
                }
                else{
                    temp_v.emplace_back(vj);
                    temp_u.emplace_back(uj);
                }
            }
        }
    }
}

void turnOnProcess(int& v1, int& v2){
    vec temp_v, temp_u;
    // 首先将新增边加入到G当中
    G[v1].nei.insert(v2);
    G[v2].nei.insert(v1);
    // ===== 在G.cand当中加入新增边 =====
    for(auto& candi : G[v1].cand){
        int ui = candi.first;   // ui匹配v1
        if(G[v1].label!=Q[ui].label){
            G[v1].cand.erase(ui);
            G[v1].LI.erase(ui);
            continue;
        }
        for(auto& ui_nei : candi.second){
            int uj = ui_nei.first;  // uj匹配v2
            // ===== 考虑v1-v2匹配ui-uj =====
            if(G[v2].label == Q[uj].label){
                // ===== 这边先不考虑v2-uj是否点亮状态，假设它是点亮的 =====
                G[v1].cand[ui][uj].insert(v2);
                // ===== 此时我们需要考虑，v1-ui是否因为加边而被点亮了 =====
                if(G[v1].LI[ui]){
                    // v2-uj的ui邻居候选为空时，说明它是熄灭的，可能因为增加了v1这个候选而被点亮
                    G[v2].cand[uj][ui].insert(v1);
                    addAndCheck(ui, v1, temp_v, temp_u);
                    // 接下来进行一个熄灭操作
                    for(int i=0; i<temp_v.size(); i++){
                        int& v = temp_v[i];
                        int& u = temp_u[i];
                        // 将u-v视为是上轮被熄灭的结点
                        if(!G[v].LI[u]){
                            // 这边是说，这些结点并没有成功被点燃，需要沿着原路一路熄灭
                            deleteAndCheck(u, v);
                        }
                    }
                    temp_v.clear();
                    temp_u.clear();
//                    if(!G[v2].LI[uj]){
//                        G[v1].cand[ui][uj].erase(v2);
//                    }
                }
                // ===== v1-ui本身是熄灭的情况 =====
                else if ( checkNei(v1, ui) ){ // 如果原先是灭的，则检查能否点亮
                    // 满足条件，则将其点亮；若无法点亮，则其v2-uj结点也不会改变状态的
                    G[v1].LI[ui] = true;
                    // 将该点点亮后，需要考虑它的所有邻居（包括v2）
                    // 因为此时该点的邻居实际相当于原先所说的「出边邻居」
                    addAndCheck(ui, v1, temp_v, temp_u);
                    // 接下来进行一个熄灭操作
                    for(int i=0; i<temp_v.size(); i++){
                        int& v = temp_v[i];
                        int& u = temp_u[i];
                        // 将u-v视为是上轮被熄灭的结点
                        if(!G[v].LI[u]){
                            // 这边是说，这些结点并没有成功被点燃，需要沿着原路一路熄灭
                            deleteAndCheck(u, v);
                        }
                    }
                    temp_v.clear();
                    temp_u.clear();
                }

            }
        }
    }


}

// ====================         静态筛选         ====================
void staticFilter(){
    for(int vi=0; vi<G_size; vi++){
        turnOff(vi);
    }
    //dumpG("./dump/G_static");
}


// ====================         搜索匹配         ====================
void dumpMatch(unordered_map<int, int>& m){
    cerr << "匹配结果：" ;
    for(int i=0; i<m.size(); i++){
        int v = m[i];
        cerr << " u" << i << "-v" << v;
    }
    cerr << endl;
}

bool shellCand(vector<u_set>& result, unordered_map<int, int>& m, const vec& s, const vector<u_set>& s_n, vec& used){
    int s_size = s.size();
    result.resize(s_size);
    for(int i = 0; i<s_size; i++) {
        auto p_ui = s_n[i].begin();
        if(s_n[i].size()>1){
            //clock_t t = clock();
            auto ui_l = *p_ui;
            ++p_ui;
            auto ui = * p_ui;
            result[i] = intersection(G[m[ui_l]].cand[ui_l][s[i]], G[m[ui]].cand[ui][s[i]]);
            for(++p_ui;p_ui!=s_n[i].end();++p_ui){
                ui = *p_ui;
                result[i] = intersection(result[i], G[m[ui]].cand[ui][s[i]]);
            }
            //t2 += double(clock() - t)*1000 / CLOCKS_PER_SEC;
            if (result[i].empty()) return 1;
        }
        else{
            auto ui = *p_ui;
            result[i] = G[m[ui]].cand[ui][s[i]];
        }
        //clock_t t = clock();
        for(auto& vth : used){
            result[i].erase(vth);
        }
        //t3 += double(clock() - t)*1000 / CLOCKS_PER_SEC;
        if (result[i].empty()) return 1;
    }
    return 0;
}

int numAdd(int th, const vector<u_set>& cand, u_set& used){
    int result = 0;
    if(th==cand.size()-1){
        int del = 0;
        for(auto& vth : used){
            if(cand[th].find(vth)!=cand[th].end()){
                del += 1;
            }
        }
        return cand[th].size() - del;
    }
    for(auto& vth : cand[th]){
        if(used.find(vth)==used.end()){
            used.insert(vth);
            result += numAdd(th+1, cand, used);
            used.erase(vth);
        }
    }
    return result;
}

bool notExit(int shell, u_set nei, unordered_map<int, int>& m){
    if(nei.size()==1){
        return false;
    }
    int n = *nei.begin();
    for (auto cand : G[m[n]].cand[n][shell]){
        int count = 0;
        for(int ni : nei){
            count += 1;
            if(count==1){
                continue;
            }
            else{
                if(G[m[ni]].cand[ni][shell].count(cand)){
                    if(count == nei.size()){
                        return false;
                    }
                }
                else{
                    break;
                }
            }
        }
    }
    return true;
}

int searchCore(int th, unordered_map<int, int>& m, vec& used, const vec& c, const vector<u_set>& c_n, const vec& s, const vector<u_set>& s_n, unordered_map<int, u_set>& c2check){
    int result = 0;
    // 核结点处理好了，处理壳结点
    if(th == c.size()){
        //clock_t ta = clock();
        //return 1;
        vector<u_set> candidates;
        if(shellCand(candidates, m, s, s_n, used)) return 0;
        u_set used_v;
        return numAdd(0, candidates, used_v);  // 耗时一般
    }
    // 处理第th个核结点
    u_set candidates;
    auto p_ui = c_n[th].begin();
    auto ui = *p_ui;
    if(c_n[th].size()>1){
        u_set* temp;
        temp = &G[m[ui]].cand[ui][c[th]];
        candidates = intersection(*temp,G[m[ui]].cand[ui][c[th]]);
        for(++p_ui;p_ui!=c_n[th].end();++p_ui){
            ui = *p_ui;
            candidates = intersection(candidates,G[m[ui]].cand[ui][c[th]]);
            if(candidates.empty()) return 0;
        }
        for(auto& vi : used){
            candidates.erase(vi);
        }
        for(auto& vth : candidates){
            m[c[th]] = vth;
            if(c2check.count(c[th])){
                bool to_continue = false;
                for (auto& shell : c2check[c[th]]){
                    if(notExit(s[shell], s_n[shell], m)) {
                        to_continue = true;
                    }
                }
                if(to_continue) {
                    continue;
                }
            }
            used.emplace_back(vth);
            result += searchCore(th+1, m, used, c, c_n, s, s_n, c2check);
            used.pop_back();
        }
    }
    else{
        candidates = G[m[ui]].cand[ui][c[th]];
        for(auto& vth : candidates){
            if(!isInVec(vth, used)){
                m[c[th]] = vth;
                if(c2check.count(c[th])){
                    bool to_continue = false;
                    for (auto& shell : c2check[c[th]]){
                        if(notExit(s[shell], s_n[shell], m)) {
                            to_continue = true;
                        }
                    }
                    if(to_continue) {
                        continue;
                    }

                }
                used.emplace_back(vth);
                result += searchCore(th+1, m, used, c, c_n, s, s_n, c2check);
                used.pop_back();
            }
        }
    }
    return result;
}

int searchMatch(int v1, int v2){
    int update_result = 0;
    for(auto& li : G[v1].LI){
        if(li.second){
            int u1 = li.first;
            for(auto& candi : G[v1].cand[u1]){
                int u2 = candi.first;  // 需要是一个正值
                if(u2>=0 && candi.second.find(v2)!=candi.second.end()){
                    unordered_map<int, int> matching;
                    matching[u1] = v1;
                    matching[u2] = v2;
                    vec core_v;
                    core_v.emplace_back(v1);
                    core_v.emplace_back(v2);

                    update_result += searchCore(0, matching, core_v, matching_order[u1][u2].core,
                                                matching_order[u1][u2].core_nei, matching_order[u1][u2].shell,
                                                matching_order[u1][u2].shell_nei, matching_order[u1][u2].c_s_nei);
                }
            }
        }
    }
    return update_result;
}


// ====================      动态更新候选区域      ====================
// 读取更新流数据，数据集中加边v1、v2是正值，删边v1、v2是负值
void inputUpdate(string& path, int max_num){
    update.clear();  // 清空update
    ifstream infile(path);
    char c;
    int v1, v2, w;
    int cnt = 0;
    while (infile >> c >> v1 >> v2){
        if(max_num!=0 && ++cnt>max_num) break;
        update.push_back(v1);           
        update.push_back(v2);
    }
}

// 更新和匹配
void updateAndMatching() {
    long long add_matches = 0;
    long long del_matches = 0;
    double update_time = 0.0;
    double search_time = 0.0;
    clock_t time;

    for (int t = 0; t < update.size(); t += 2) {
        int v1 = update[t];
        int v2 = update[t + 1];

        // 删边
        if(v1<0){
            if(G[-v1-1].label==-1 || G[-v2-1].label==-1) continue; // 无关结点的更新可以跳过
            if(!G[-v1-1].nei.count(-v2-1)) continue;
            // 更新的匹配结果
            time = clock();
            del_matches += searchMatch(-v1-1, -v2-1);
            search_time += double(clock() - time)*1000 / CLOCKS_PER_SEC;
            // 熄灭操作
            time = clock();
            turnOffProcess(-v1-1, -v2-1);
            update_time += double(clock() - time) * 1000 / CLOCKS_PER_SEC;
        }

            // 加边
        else{
            if(G[v1].label==-1 || G[v2].label==-1) continue; // 无关结点的更新可以跳过
            if(G[v1].nei.count(v2)) continue;
            // 点亮操作
            time = clock();
            turnOnProcess(v1, v2);
            update_time += double(clock() - time)*1000 / CLOCKS_PER_SEC;
            // 更新的匹配结果
            time = clock();
            add_matches += searchMatch(v1, v2);
            search_time += double(clock() - time)*1000 / CLOCKS_PER_SEC;
        }
        //count_time[int(double(clock() - time2)*1000 / CLOCKS_PER_SEC /100)]+=1;
    }
    cerr << "added matches: " << add_matches << endl;
    cerr << "deleted matches: " << del_matches << endl;
    cerr << "updated matches: " << add_matches + del_matches << endl;
    cerr << "update time: " << update_time << "ms" << endl;
    cerr << "search time: " << search_time << "ms" << endl;
}


int main(int argc, char** argv){

    // 数据集地址
    string g_path = "../dataset/dz_3/initial";
    string q_path = "../dataset/dz_3/Q/8/q10";
    string s_path = "../dataset/dz_3/s";
    int cnum = 0;
    // 更改数据集地址
    for (int i=1; i<argc; i++) {
        if(string(argv[i]) == "-d")
            g_path = argv[i+1];
        else if(string(argv[i]) == "-q")  // 表示查询图的位置
            q_path = argv[i+1];
        else if(string(argv[i]) == "-c")  // 表示选取数据流前面的多少行，默认是所有
            cnum = atoi(argv[i+1]);
        else if(string(argv[i]) == "-s") {
            s_path = argv[i+1];
        }
    }

    // 读取数据
    clock_t time = clock();
    cerr << "========== Start inputting. ==========" << endl;
    inputQ(q_path);
    generateMO();
    inputG(g_path);
    inputUpdate(s_path, cnum);
    cerr << "Inputting cost (ms): " << double(clock() - time)*1000 / CLOCKS_PER_SEC << endl;
    cerr << "The number of vertices in Q: " << Q_size << endl;
    cerr << "The number of vertices in G: " << G_size << endl;
    cerr << "========== End inputting. ==========" << endl;

    // 构建数据结点的候选信息
    time = clock();
    cerr << "========== Start constructing. ==========" << endl;
    constructCand();
    cerr << "Constructing cost (ms): " << double(clock() - time)*1000 / CLOCKS_PER_SEC << endl;
    cerr << "========== End constructing. ==========" << endl;

    // 静态筛选
    time = clock();
    cerr << "========== Start static filtering. ==========" << endl;
    staticFilter();
    cerr << "Static filtering cost (ms): " << double(clock() - time)*1000 / CLOCKS_PER_SEC << endl;
    cerr << "========== End static filtering. ==========" << endl;

    // 动态更新和匹配
    time = clock();
    cerr << "========== Start updating. ==========" << endl;
    updateAndMatching();
    cerr << "Updating totally cost (ms): " << double(clock() - time)*1000 / CLOCKS_PER_SEC << endl;
    cerr << "========== End updating. ==========" << endl;

    cerr << "========== DONE!! ==========" << endl;

    return 0;
}