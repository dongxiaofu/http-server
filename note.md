##20200507
###接收get请求

1.`no known conversion from 'void (int)' to 'void * _Nullable (* _Nonnull)(void * _Nullable)' for 3rd argument`

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

4.从`socket`读取实体主体数据卡住

```
string read_body(int socket_fd) {
    std::cout << "start read body" << std::endl;
    string body;
//    char data[1024];
    char c;
    while (recv(socket_fd, &c, 1, 0) != -1) {
        body += c;
    }
    std::cout << body;

//    while (recv(socket_fd, data, sizeof(data), 0) != -1) {
//        std::cout << data;
//        body += data;
//    }

    return body;
}
```

并非完全卡住，而是在读取完所有实体主体数据后，`recv`卡住，持续数秒后，进程退出。

接收不到数据了，为什么不主动退出循环而是卡住等待客户端发送数据呢？

之前打日志时出错，使我对该段代码的执行状况有错误的理解。

卡住一段时间后，进程意外退出，`Process finished with exit code 11`。

对比`Content-Length`值与实际接收到的数据的大小比较，若两者相等，主动退出循环。

5.停止服务器后马上启动，为什么无法提供`http`服务？

10多秒后再启动服务器，就正常了。

6.用浏览器提交一次表单，POST请求，却产生三次POST请求

这是为什么？

使用postman提交请求，只产生一次POST请求。

服务端逐个字符接收客户端数据，没有问题。一次性接收多个字符，也没有问题。

逐个接收字符

```
char c;
while (recv(socket_fd, &c, 1, 0) != -1) {
    body += c;
    if (body.size() == content_length) {
        break;
    }
}
```

一次性接收多个字符

```
string body;
char data[1024];
while (recv(socket_fd, data, sizeof(char)*1024, 0) != -1) {
    body += data;
    if (body.size() == content_length) {
        break;
    }
}
```

7.接收上传文件的表单提交的数据

数据

```
POST /server.html HTTP/1.1
Host: 127.0.0.1
Connection: keep-alive
Content-Length: 291042
Pragma: no-cache
Cache-Control: no-cache
Upgrade-Insecure-Requests: 1
Origin: http://127.0.0.1
Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryPL3JKVDDvW4BBCCH
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/81.0.4044.129 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9
Sec-Fetch-Site: same-origin
Sec-Fetch-Mode: navigate
Sec-Fetch-User: ?1
Sec-Fetch-Dest: document
Referer: http://127.0.0.1/form.html
Accept-Encoding: gzip, deflate, br
Accept-Language: zh-CN,zh;q=0.9,en-US;q=0.8,en;q=0.7,ja;q=0.6
------WebKitFormBoundaryPL3JKVDDvW4BBCCH
Content-Disposition: form-data; name="title"

穿越时空的恋爱
------WebKitFormBoundaryPL3JKVDDvW4BBCCH
Content-Disposition: form-data; name="age"

彻底撒富家大室发的说法
------WebKitFormBoundaryPL3JKVDDvW4BBCCH
Content-Disposition: form-data; name="pic"; filename="9-1602061H626.jpg"
Content-Type: image/jpeg

<图片数据，一段乱码>
------WebKitFormBoundaryPL3JKVDDvW4BBCCH--
```

神奇，实体主体与请求行之间的分隔符不是`\r\n`。不是这样的。在服务端，我没有输出换行符。

表单

```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>表单</title>
</head>
<body>
<form action="./server.html" method="post">
    <p><input type="text" name="title"></p>
    <p><input type="text" name="age"></p>
<!--    <p><input type="file" name="pic"></p>-->
    <p><input type="submit"></p>
</form>

</body>
</html>
```

服务端接收到的数据

```
POST /server.html HTTP/1.1
Host: 127.0.0.1
Connection: keep-alive
Content-Length: 173
Pragma: no-cache
Cache-Control: no-cache
Upgrade-Insecure-Requests: 1
Origin: http://127.0.0.1
Content-Type: application/x-www-form-urlencoded
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/81.0.4044.129 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9
Sec-Fetch-Site: same-origin
Sec-Fetch-Mode: navigate
Sec-Fetch-User: ?1
Sec-Fetch-Dest: document
Referer: http://127.0.0.1/form.html
Accept-Encoding: gzip, deflate, br
Accept-Language: zh-CN,zh;q=0.9,en-US;q=0.8,en;q=0.7,ja;q=0.6

title=%E7%A9%BF%E8%B6%8A%E6%97%B6%E7%A9%BA%E7%9A%84%E6%81%8B%E7%88%B1&age=%E5%BD%BB%E5%BA%95%E6%92%92%E5%AF%8C%E5%AE%B6%E5%A4%A7%E5%AE%A4%E5%8F%91%E7%9A%84%E8%AF%B4%E6%B3%95
```

表单

```
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>表单</title>
</head>
<body>
<form action="./server.html" method="post" enctype="multipart/form-data">
    <p><input type="text" name="title"></p>
    <p><input type="text" name="age"></p>
<!--    <p><input type="file" name="pic"></p>-->
    <p><input type="submit"></p>
</form>

</body>
</html>
```

服务端接收到的数据

```
POST /server.html HTTP/1.1
Host: 127.0.0.1
Connection: keep-alive
Content-Length: 280
Pragma: no-cache
Cache-Control: no-cache
Upgrade-Insecure-Requests: 1
Origin: http://127.0.0.1
Content-Type: multipart/form-data; boundary=----WebKitFormBoundaryn8OE4vY8st2mRIJC
User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_1) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/81.0.4044.129 Safari/537.36
Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.9
Sec-Fetch-Site: same-origin
Sec-Fetch-Mode: navigate
Sec-Fetch-User: ?1
Sec-Fetch-Dest: document
Referer: http://127.0.0.1/form.html
Accept-Encoding: gzip, deflate, br
Accept-Language: zh-CN,zh;q=0.9,en-US;q=0.8,en;q=0.7,ja;q=0.6

------WebKitFormBoundaryn8OE4vY8st2mRIJC
Content-Disposition: form-data; name="title"

穿越时空的恋爱
------WebKitFormBoundaryn8OE4vY8st2mRIJC
Content-Disposition: form-data; name="age"

彻底撒富家大室发的说法
------WebKitFormBoundaryn8OE4vY8st2mRIJC--
==============================body start=====================
------WebKitFormBoundaryn8OE4vY8st2mRIJC
Content-Disposition: form-data; name="title"

穿越时空的恋爱
------WebKitFormBoundaryn8OE4vY8st2mRIJC
Content-Disposition: form-data; name="age"

彻底撒富家大室发的说法
------WebKitFormBoundaryn8OE4vY8st2mRIJC--

==============================body end=====================

```

不解析POST请求数据，而是将这些数据转发给php等处理。

## 实现fpm协议
就是把动态请求转发给php。

半年前实现过fpm协议，现在，忘记了百分之九十五。我不相信，理解了东西，就不会忘记，下次做就很容易。自己几个月前写的东西，时间久了，再次接触，有时看不懂。

一定要做好笔记，不要相信记忆力。

需要把请求行在内的所有数据转发给php，另外，需要传递数个从请求行内才能获取的数据。

工作量主要在实现fpm协议，对照着原来的java代码翻译。

### 实现
#### 回忆
建立连接到php的tcp连接。将数据传递给php，然后从php读取结果A。A就是http服务器需要的结果。

传递给php的数据是一个数据包，构成：请求开始，请求参数，请求参数结束符，请求数据主体，请求结束标志符。

以上是我看半年前的java代码实现后回忆内容。后面，我会根据java代码写出完全符合代码的文档。
#### 实际实现

## 疑问
### 向静态页面或非处理POST请求的页面发送POST请求
会返回什么？



## todo
1. 实现`Connection: keep-alive`
1. 实现`http2`
1. 实现`https`
1. 实现`fpm`
1. 兼容`nginx`配置
1. 实现`nginx`的工作机制：master进程接收请求，work进程处理请求
1. 实现access_log
1. 实现error_log
1. 实现负载均衡
1. 提高并发量





