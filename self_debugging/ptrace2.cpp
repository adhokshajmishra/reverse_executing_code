//
// Created by adhokshajmishra on 22/4/20.
//
#include <iostream>
#include <sys/ptrace.h>

int main()
{
    int offset = 0;

    if (ptrace(PTRACE_TRACEME, 0, 1, 0) == 0)
    {
        offset = 2;
    }

    if (ptrace(PTRACE_TRACEME, 0, 1, 0) == -1)
    {
        offset = offset * 3;
    }

    if (offset == 2 * 3)
    {
        std::cout << "normal execution" << std::endl;
    }
    else
    {
        std::cout<< "don't trace me !!" << std::endl;
    }

    return 0;
}