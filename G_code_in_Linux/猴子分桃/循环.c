#include <stdio.h>

int main()
{

    int i = 6;
    while(1)
    {
        int temp = i;
        int j = 0;
        while(j < 5)
        {
            if((temp-1)%5==0)
            {
                temp = (temp-1)/5*4;
            }
            else
            {
                break;
            }
            ++j;
        }
        if(j==5)
        {
            printf("num is %d\n",i);
            break;
        }
        i+=5;
    }
    return 0;
}
