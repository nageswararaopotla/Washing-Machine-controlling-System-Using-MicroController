#include <xc.h>
#include"main.h"
void __interrupt() isr(void)
{
    extern unsigned char min,sec;
    static unsigned int count=0;
    if(TMR2IF==1)
    {
        if(++count==1250)
        {
            count=0;
            if(sec>0)
                sec--;
            else if(sec==0 &&min>0)
            {
                min--;
                sec=59;
            }
        }
        TMR2IF=0;
    }
}

