class FcgiRequestType {
public:
    static const int FCGI_BEGIN_REQUEST = 1;
    static const int FCGI_ABORT_REQUEST = 2;
    static const int FCGI_END_REQUEST = 3;
    static const int FCGI_PARAMS = 4;
    static const int FCGI_STDIN = 5;
    static const int FCGI_STDOUT = 6;
    static const int FCGI_STDERR = 7;
    static const int FCGI_DATA = 8;
    static const int FCGI_GET_VALUES = 9;
    static const int FCGI_GET_VALUES_RESULT = 10;
};