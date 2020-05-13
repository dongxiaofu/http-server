#include <iostream>
#include <vector>

using namespace std;

// vector的使用
int main() {
    // 有的资料说这种方式可以，但我却未编译通过
//    vector<char> v {1, 2, 3, 4};
    vector<int> v;
    v.push_back(1);
    v.push_back(2);
    v.push_back(3);
    v.push_back(4);
    vector<int>::iterator it;
    // 若改成vector<char> v，输出的都是不可见字符
    // it 的终止条件是 < v.end()，不包含v.end()
    for (it = v.begin(); it < v.end(); it++) {
        // vector中的值 *it
        cout << *it << endl;
    }
    cout << "================" << endl;
    vector<int> m;
    cout << "m.size()=" << m.size() << endl;
    return 0;
}