#include<iostream>
#include<string>
#include<fstream>

using namespace std;

int main() {
//    string FileDirect = "/Users/cg/data/code/wheel/cpp/http-server/htdocs/weixiao_nvhai-001.jpg";  //图像文件地址
    string FileDirect = "/Users/cg/data/code/wheel/cpp/http-server/demo/hello";
    ifstream is(FileDirect, ios::in | ios_base::binary);
    // 2. 计算图片长度
    int i = is.tellg();
    is.seekg(0, is.end);
    int length = is.tellg();
    length = length - i;
    cout << "length:";
    cout << length;
    cout << endl;
    is.seekg(0, is.beg);
    // 3. 创建内存缓存区
    char *buffer = new char[length];
    // 4. 读取图片
    while(is.read(buffer, sizeof(buffer))){
        cout << buffer;
    }
    cout << endl;
//    while(1){
//        // 4. 读取图片
//        is.read(buffer, length);
//        cout << buffer;
//        if(is.eof()){
//            break;
//        }
//    }

    // 到此，图片已经成功的被读取到内存（buffer）中
    delete[] buffer;
    return 0;

////    int picturenumber = 1;                                                                                               //图像计数
//    int i = 0, j = 0, length = 0;                                                                                              //计算图像长度
////    string FileDirect = "/Users/cg/data/code/wheel/cpp/http-server/demo/cg.jpg";  //图像文件地址
////    string PictureDirect = FileDirect + "\\" + std::to_string(picturenumber) + ".jpg";//图像绝对地址
//    string PictureDirect = "/Users/cg/data/code/wheel/cpp/http-server/demo/cg.jpg";
//    ifstream in(PictureDirect.c_str(), ios::in | ios::binary);//打开文件，供读取以二进制方式
//    i = in.tellg();                                                                                                              //当前get流指针的位置
//    in.seekg(0, ios::end);  //基地址为文件结束处，偏移地址为0，于是指针定位在文件结束处
//    j = in.tellg();                                                                                                             //再次获取当前get流指针的位置
//    length = j - i;
//    char *buffer = new char[length];
//    in.read(buffer, length);//读取图像数据到buffer中
//    if (buffer == NULL) {
//        cout << "读取失败";
//    } else {
//        cout << "读取成功" << endl;
//    }
//    cout << buffer;

}
