//
// Created by adhokshajmishra on 22/4/20.
//
#include <iostream>
#include <sys/ptrace.h>

int main()
{
    if (ptrace(PTRACE_TRACEME, 0, 1, 0) == -1)
    {
        std::cout<< "don't trace me !!" << std::endl;
        return 1;
    }

    std::cout << "normal execution" << std::endl;
    return 0;
}
