#include <fstream>

int main(int argc, char const *argv[]) {
    int val = 1;
    std::ofstream out("out.bin", std::ios::binary);
    //out << val; //不能用 << 输出二进制
    out.write(reinterpret_cast<const char *>(&val), sizeof(val));
    out.close();

    return 0;
}