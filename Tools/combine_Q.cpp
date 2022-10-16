/****************************
 * 作用：把单独的、vid编号从0开始的Q文件，组合成一个整体的、vid连续向后的、e中的id也随着连续的vid变化的大Q文件
 *
 * 说明：由于单个Q_x文件（x代表0、1、2...）中，vid只在这个文件中从0开始，并且该文件中的e中的id也随着vid而变化
 * 如果想要让这些单个Q_x文件组合成一个大的Q文件，就需要让里面所有的vid按照顺序依次往下排，同时让e中的id也随着vid而变化
 ************************* */


#include <iostream>
#include <fstream>  //文件操作库
#include <map>

using namespace std;

int combine(){    //使用时，把main2修改成main
    string file_name_pre = R"(E:\Desktop\GraphQuery C++\Data\Q_)";
    string Q_multi = R"(E:\Desktop\GraphQuery C++\Data\Q_multi)";
    int id = 0;
    int id1,id2,label,weight,Q_id,total_v_num;
    char single_data;  //定义单个字符的变量，用来储存读入的数据
    int last_vid = -1 ;  //定义该值为-1 ，以便处理第一个Q
    map<int,int> vid_vid ;

    for(int i = 0 ; i < 2 ; i++){
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
                last_vid = id + last_vid + 1;   //在遇到t的阶段更新last_vid，并且由于last_vid初始值为-1.所以在处理第一个Q的时候，这里加1，表示vid从0开始
                infile >> Q_id >> total_v_num;  //第一个数据为查询图的编号，第二个数据为该查询图下有多少节点
                outfile << single_data << " " << Q_id << " " << total_v_num <<endl;
            }else if(single_data=='v'){   //如果遇到了结点
                infile >> id >> label;  //依次保存节点id和节点标签
                vid_vid[id] = id+last_vid;   //保证vid能连续的关键
                outfile << single_data << " "  << id+last_vid << " " << label<<endl;
            }else if(single_data=='e'){
                infile >> id1 >> id2 >> weight;
                outfile << single_data << " " << vid_vid[id1] << " "  << vid_vid[id2] << " "  << weight << endl;  //e中的id1和id2随着vid变化
            }
        }
        infile.close();
        outfile.close();
    }


    return 0;
}





















