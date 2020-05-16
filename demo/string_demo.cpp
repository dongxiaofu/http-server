#include <iostream>
#include <vector>

using namespace std;

int main()
{
	string string1 = "ABC";
	cout <<string1.c_str()<<endl;
    vector<char> body_from_client;
	cout<<body_from_client.data()<<endl;
    string str = "AB";
	for(int i=0;i<str.size();i++){
		cout << str[i] << endl;
	}
	char string[str.size()];
	for(int i = 0; i < str.size(); i++){
	    string[i] = str[i];
	}
	for(int i = 0; i < str.size(); i++){
	    cout << string[i] << endl;
	}
	int arr[2] = {1,2};
	for(int i = 0; i < 2; i++){
	    cout << arr[i];
	}
	int *ptr = arr;
	for(int i = 0; i < 2; i++){
	    cout << ptr[i];
	}
	return 0;
}
