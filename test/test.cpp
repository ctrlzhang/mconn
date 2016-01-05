#include <iostream>
#include <string>
#include <string.h>
using namespace std;

struct test
{
    char name[20];
    char desc[20];
};

int main()
{
    test t;
    memset(t.name, 0, 20);
    memset(t.desc, 0, 20);

    strcpy(t.name, "111");
    strcpy(t.desc, "222");

    cout<<sizeof(t)<<endl;

    return 0;
}
