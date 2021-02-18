#include <iostream>
#include <fstream>
#include <string>
using namespace std;

int main(int argc, char const *argv[])
{
    string buf;

    while (cin >> buf)
    {
        string res("kProp");
        bool needUpper = true;
        for (char ch : buf)
        {
            if (ch == '-')
                needUpper = true;
            else if (needUpper)
            {
                res.push_back(toupper(ch));
                needUpper = false;
            }
            else
                res.push_back(ch);
        }

        cout << "    { CSSPropertyName::" << res << ", \t\"" << buf << "\"\t }, " << endl;
    }
    return 0;
}
