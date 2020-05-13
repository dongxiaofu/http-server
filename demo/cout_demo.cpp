#include <iostream>

using namespace std;

typedef struct {
    int size;
    int *data;
} DEMO;

void test(int arr[], int size);

char *get_content();

int max(int num1, int num2);

void get_content_by_ptr(char *ptr);

char *get_name();

int main() {
    int size = 3;
    int arr[size];
    test(arr, 3);
    cout << "==========================" << endl;
    char *content = get_content();
    cout << "content:" << content << endl;
    cout << "==========================" << endl;
    int max_num = max(7, -1);
    cout << "max_num:" << max_num << endl;
    cout << "==========================" << endl;
    char *name = get_name();
    cout << "name:" << name << endl;
    delete [] name;
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

char *get_content() {
    char content[200] = "hello,world";
    return content;
}

int max(int num1, int num2) {
    // 局部变量声明
    int result;
    if (num1 > num2)
        result = num1;
    else
        result = num2;
    return result;
}


void get_content_by_ptr(char *ptr) {
    char content[200] = "hello,world!";
    ptr = content;
}

char *get_name() {
    char get_name[30];
    cout << "Enter your name :";// a word
    cin >> get_name;
    char *name = new char[strlen(get_name)];
    strcpy(name, get_name);// do not use name = get_name
    //because name will get get_name address it's in the stack
    //it is stupid idea.
    return name;
}
