class FcgiRequestType {
public:
    const int FCGI_BEGIN_REQUEST = 1;
    const int FCGI_ABORT_REQUEST = 2;
    const int FCGI_END_REQUEST = 3;
    const int FCGI_PARAMS = 4;
    const int FCGI_STDIN = 5;
    const int FCGI_STDOUT = 6;
    const int FCGI_STDERR = 7;
    const int FCGI_DATA = 8;
    const int FCGI_GET_VALUES = 9;
    const int FCGI_GET_VALUES_RESULT = 10;
};