#include <stdio.h>
#include <string>
#include <iostream>

using namespace std;

int main() {
    char src[40];
    char dest[100];

    char str3[] = "h a o";
    for (int i = 0; i < strlen(str3); i++) {
        cout << "str3-c:" << str3[i] << ":" << int(str3[i]) << endl;
    }

    memset(dest, '\0', sizeof(dest));
    string str = "hello\0world";
    char str2[] = "hello\0world";
    cout << "str_len:" << str.size() << endl;
    for (int i = 0; i < str.size(); i++) {
        cout << "i:" << i << str[i] << endl;
    }
    strcpy(src, str2);
    printf("最终的目标字符串： %s\n", src);

    return (0);
}