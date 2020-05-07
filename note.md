##20200507
###接收get请求

no known conversion from 'void (int)' to 'void * _Nullable (* _Nonnull)(void * _Nullable)' for 3rd argument

原因就是C语言编译器允许隐含性的将一个通用指针转换为任意类型的指针，包括const *而C＋＋不允许将const 转换为非const*，所以出错。

所以要按照符合函数声明的本来面目修改出错函数的形参输入与返回值

https://blog.csdn.net/searchwang/article/details/30036129
``
int pthread_create(pthread_t _Nullable * _Nonnull __restrict,
		const pthread_attr_t * _Nullable __restrict,
		void * _Nullable (* _Nonnull)(void * _Nullable),
		void * _Nullable __restrict);
``
warning: cast to 'void *' from smaller integer type 'int' [-Wint-to-void-pointer-cast]

把
``
if (pthread_create(&newthread, NULL, accept_request, (void *) client_sock) != 0) {
    perror("pthread_create");
}
``
改成
``
void *tmp = (void *) (long)client_sock;
if (pthread_create(&newthread, NULL, accept_request, tmp) != 0) {
    perror("pthread_create");
}
``
原因未知。

