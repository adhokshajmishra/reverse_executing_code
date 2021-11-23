//
// Created by adhokshajmishra on 22/4/20.
//

#include <iostream>
#include <unistd.h>
#include <cstring>
#include <sys/ptrace.h>
#include <sys/syscall.h>
#include <sys/wait.h>
#include <sys/user.h>

#define SYS_CUSTOM_write 10000

void print_custom(char *str) {
    syscall(SYS_CUSTOM_write, str, 1, strlen(str));
}

void tracee() {
    ptrace(PTRACE_TRACEME, 0, 0, 0);
    raise(SIGCONT);//to make it actually stop
    std::cout << "Try attaching debugger to child process :P" << std::endl;
    for (int i = 0; i < 10; i++) {
        //std::cout << "Try debugging me please:)" << std::endl;
        print_custom("A");
    }
}

void tracer(pid_t child_pid) {
    int status;
    waitpid(child_pid, &status, 0);
    if (!WIFSTOPPED(status)) {
        std::cerr<< "Incorrect state." << std::endl;
        return;
    }
    ptrace(PTRACE_SETOPTIONS, child_pid, 0, PTRACE_O_EXITKILL);
    struct user_regs_struct regs;
    while (WIFSTOPPED(status)) {
        ptrace(PTRACE_SYSCALL, child_pid, 0, 0);
        waitpid(child_pid, &status, 0);
        ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
        if (regs.orig_rax == SYS_CUSTOM_write) {
            //printf("CUSTOM_write found, patched.\n");
            regs.orig_rax = SYS_write;
            //swap arg1 & arg2
            unsigned long long int orig_rdi = regs.rdi;
            regs.rdi = regs.rsi;
            regs.rsi = orig_rdi;

            ptrace(PTRACE_SETREGS, child_pid, 0, &regs);
        }
        ptrace(PTRACE_SYSCALL, child_pid, 0, 0);
        waitpid(child_pid, &status, 0);
    }
}

int main() {
    std::cout << "Linux Anti-debugging Demo" << std::endl;
    pid_t child_pid = fork();
    if (child_pid < 0) {
        printf("Fork failed.\n");
        return 1;
    }
    if (child_pid == 0) {
        tracee();
    } else {
        tracer(child_pid);
    }
    return 0;
}
