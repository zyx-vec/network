#include        <time.h>
#include        <stdio.h>
#include        <stdlib.h>
#include        <signal.h>
#include        <unistd.h>
#include        <sys/select.h>

#define BUFFSIZE 80

void err_sys(const char *p_error);

void sig_alrm(int signo)
{
    char s[] = "receive";

    psignal(signo, s);

    return;
}

int main(int argc, char **argv) {
    int             maxfdp1;
    fd_set          rset;
    sigset_t        sigmask;
    ssize_t         nread;
    char            buf[BUFFSIZE];
    int ret = 0;

    sigset_t sigset;
    struct sigaction act;

    // set SIGALRM signal handler
    act.sa_handler = sig_alrm;
    if (sigemptyset(&act.sa_mask) == -1)       
    err_sys("sigemptyset");
    act.sa_flags = 0;
    if (sigaction(SIGALRM, &act, NULL) == -1)
    err_sys("sigaction");

    // initialize signal set and addition SIGALRM into sigset
    if (sigemptyset(&sigset) == -1)
        err_sys("sigemptyet");
    if (sigaddset(&sigset, SIGALRM) == -1)
        err_sys("sigaddset");
    
    alarm(1);   // 1 second later, deliver SIG_ALARM to this process.

    FD_ZERO(&rset);
    FD_SET(STDIN_FILENO, &rset);
    maxfdp1 = STDIN_FILENO + 1;
    ret = select(maxfdp1, &rset, NULL, NULL, NULL);
//        ret = pselect(maxfdp1, &rset, NULL, NULL, NULL, &sigset);
    if (ret <= 0)
    {
        printf("%x %d/n",ret,ret);
            err_sys("pselect error");
    }

    if (FD_ISSET(STDIN_FILENO, &rset))
    {
            if ((nread = read(STDIN_FILENO, buf, BUFFSIZE)) == -1)
                    err_sys("read error");
            if (write(STDOUT_FILENO, buf, nread) != nread)
                    err_sys("write error");
    }

    exit(0);
}

void err_sys(const char *p_error)
{
        perror(p_error);

        exit(1);
}