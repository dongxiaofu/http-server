#include <iostream>
#include <vector>
#include <algorithm>

using namespace std;

typedef struct {
    int size;
    int *data;
} DEMO;

void test(int arr[], int size);

int main() {
    vector<int> a;
    a.push_back(1);
    a.push_back(2);
    a.insert(a.end(),3);
    vector<int> v = {7,8,9};
//    copy(a.begin(),a.end(),v.begin());
    vector<int> c;
    a.insert(a.end(),v.begin(),v.end());
    a.insert(a.end(), a.begin(),a.end());
    cout << "hello" << endl;
    int size = 3;
    int arr[size];
    int arr2[2];
    test(arr, 3);
    return 0;
}

void test(int arr[], int size) {
    int k = 0;
    for (int i = 0; i < 3; i++) {
        arr[k++] = i;
    }
    for (int i = 0; i < 3; i++) {
        cout << "nums:" << arr[i] << endl;
    }
    DEMO demo = {3, arr};
    int *nums2 = demo.data;
    for (int i = 0; i < 3; i++) {
        cout << demo.data[i] << endl;
    }
}
