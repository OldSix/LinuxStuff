#include <stdio.h>

int count = 5;
int foo(int n)
{
    if(count == 0)
        return 1;
    
    if((n-1)%5==0)
    {
        --count;
        return foo((n-1)/5*4);
    }
    else
    {
        count = 5;
        return -1;
    }
}

int main()
{
    int i=6;
    while(1)
    {
        int ret = foo(i);
        if(ret == 1)
        {
            printf("num is %d\n", i);
            break;
        }
        i+=5;
    }
    return 0;
}
