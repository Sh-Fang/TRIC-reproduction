#include <iostream>
#include <fstream>  //文件操作库
#include <map>

using namespace std;

int main2(){
    string file_name_pre = R"(E:\Desktop\GraphQuery C++\Data\Q_)";
    string Q_multi = R"(E:\Desktop\GraphQuery C++\Data\Q_multi)";
    int id = 0;
    int id1,id2,label,weight,Q_id,total_v_num;
    char single_data;  //定义单个字符的变量，用来储存读入的数据
    int last_vid = -1 ;
    map<int,int> vid_vid ;

    for(int i = 0 ; i < 10 ; i++){
        string file_name ;

        ifstream infile;   //读取文件的数据流
        ofstream  outfile;

        file_name = file_name_pre + to_string(i);
        infile.open(file_name);  //打开数据图文件
        outfile.open(Q_multi,ios::app);

        if(!infile){
            cerr << "Failed To Load Data Graph" << endl;  //cerr是std中的标准错误输出（和cout有区别）
            return 0;
        }

        if(!outfile){
            cerr << "Failed To Load Data Graph2" << endl;  //cerr是std中的标准错误输出（和cout有区别）
            return 0;
        }

        while(infile >> single_data){
            if(single_data=='t'){   //如果遇到了t
                vid_vid.clear();
                last_vid = id + last_vid + 1;
                infile >> Q_id >> total_v_num;  //第一个数据为查询图的编号，第二个数据为该查询图下有多少节点
                outfile << single_data << " " << Q_id << " " << total_v_num <<endl;
            }else if(single_data=='v'){   //如果遇到了结点
                infile >> id >> label;  //依次保存节点id和节点标签
                vid_vid[id] = id+last_vid;
                outfile << single_data << " "  << id+last_vid << " " << label<<endl;
            }else if(single_data=='e'){
                infile >> id1 >> id2 >> weight;
                outfile << single_data << " " << vid_vid[id1] << " "  << vid_vid[id2] << " "  << weight << endl;
            }
        }
        infile.close();
        outfile.close();
    }


    return 0;
}





















