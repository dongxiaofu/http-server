#include<iostream>
#include<string>
#include<fstream>

using namespace std;

int main() {
    string FileDirect = "/Users/cg/data/code/wheel/cpp/http-server/htdocs/weixiao_nvhai-001.jpg";  //图像文件地址
//    string FileDirect = "/Users/cg/data/code/wheel/cpp/http-server/demo/hello";
//    string FileDirect = "/Users/cg/data/code/wheel/cpp/http-server/demo/tcg";
    // 1. 打开图片文件
    ifstream is(FileDirect, ifstream::in | ifstream::binary);
    // 2. 计算图片长度
    is.seekg(0, is.end);
    int length = is.tellg();
    is.seekg(0, is.beg);
    // 3. 创建内存缓存区
    char *buffer = new char[length];
    // 4. 读取图片
    int read_len = 0;
    string content;
    while(!is.eof()){
        is.read(buffer, length);
        read_len += is.gcount();
        content += buffer;
    }

    cout << length << endl;
    cout << read_len << endl;
    cout << "content:" << endl;
    cout << content;
    // 到此，图片已经成功的被读取到内存（buffer）中
    delete[] buffer;
    return 0;


}
