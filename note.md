##20200507
###接收get请求

1.no known conversion from 'void (int)' to 'void * _Nullable (* _Nonnull)(void * _Nullable)' for 3rd argument

原因就是C语言编译器允许隐含性的将一个通用指针转换为任意类型的指针，包括const *而C＋＋不允许将const 转换为非const*，所以出错。

所以要按照符合函数声明的本来面目修改出错函数的形参输入与返回值

https://blog.csdn.net/searchwang/article/details/30036129

```
int pthread_create(pthread_t _Nullable * _Nonnull __restrict,
		const pthread_attr_t * _Nullable __restrict,
		void * _Nullable (* _Nonnull)(void * _Nullable),
		void * _Nullable __restrict);
```
warning: cast to 'void *' from smaller integer type 'int' [-Wint-to-void-pointer-cast]

把

```
if (pthread_create(&newthread, NULL, accept_request, (void *) client_sock) != 0) {
    perror("pthread_create");
}
```

改成

```
void *tmp = (void *) (long)client_sock;
if (pthread_create(&newthread, NULL, accept_request, tmp) != 0) {
    perror("pthread_create");
}
```

原因未知。

2.读取图片，读取的数据残缺，原因未知。耗时许久。

调试非常低效。

图片不能显示，浏览器一直处于加载状态=======>Content-Length大于发送给浏览器的数据大小。

`ifstream` 读取的图片文件数据大小，小于图片文件实际大小。

读取图片文件，是否使用循环。不使用循环，只能读取到一部分数据。

使用循环后，读取到的数据总是小于实际数据。

在使用循环和不使用循环上面调试好久。

又在是否使用二进制模式读取数据这个方向耗时许久。

最后，再次搜索*c++读取数据*，尝试一个例子后，解决了问题。

一个c++用法，耗时五六个小时，太低效了。

每次调试，要拟好思路，每个思路，只需验证两三次，就排除。无脑重复，实在不可取。

解决了。需要正确地读取图片文件。错误原因，读取的图片文件数据丢失，与实际文件大小不一致，导致Content-Length大于向浏览器返回的数据。

正确读取图片文件的代码：

```
PICTURE read_picture(string file_path) {
    string data;
    using namespace std;
    ifstream is(file_path, ios::in);
//    ifstream is(file_path, ios::in | ios::binary);  // ok
    // 2. 计算图片长度
    is.seekg(0, is.end);
    int len = is.tellg();
    is.seekg(0, ios::beg);
    stringstream buffer;
    buffer << is.rdbuf();
    PICTURE picture = {len, len, buffer.str()};
    // 到此，图片已经成功的被读取到内存（buffer）中
    return picture;
}
```

3.显示图片的网页不能正常显示

```
<!--<p>显示图片</p>-->
<img src="weixiao_nvhai-001.jpg">
```

图片网络请求一直处于 pending 状态。

解决：建立的tcp连接数量，导致整个问题。

包含图片的网页A，发起三个http请求，一个是请求网页本身，一个是请求favicon.ion，另一个是请求图片。前两个请求正常，第三个请求一直是`pending`状态。

修改`Connection`，值为`keep-alive`或'close`，都不起作用。

关闭发送`http`数据的`tcp socket`后，初次打开网页A，正常。再次打开任何网页，已经不能连接到`http`服务器。

在关闭`socket`前，休眠一小会儿，竟然正常了。

这种方法，一定存在很大的瓶颈，并发量不会很高。

想方设法提高`http`服务器的处理速度，这种方法却需要休眠一段时间。

有更好的方法吗？

找到这个方法，纯粹是突发奇想地猜想。

我怀疑过是浏览器的问题，很快排除这中可能。

怀疑过是`http`协议中的响应行中的`Connection`的问题，测试后排除这种可能。

开始怀疑服务端`socket`监听数量的问题，`listen`的第二个参数，排除。

怀疑服务端`pthread_create`创建线程的数量有限制，排除。

我实现的这个方法，一个tcp只能处理一个http事务，`Connection:keep-alive`并没有实现，不是真正的`http/1.1`。




