#include <stdio.h>
#include <signal.h>
#include <stdlib.h>

#include <http_server/errcode.h>
#include <http_server/server.h>

HTTPServer server;

void sig_handler(int sig) {
    printf("Received signal %d\n", sig);
    int ret = HTTPServer_finalize(&server);
    printf("retcode: %d\nmsg: %s\n", ret, http_server_errcode_lookup[ret]);
    exit(0);
}

int main(void) {
    int ret = HTTPServer_initialize(&server, 8000);
    printf("retcode: %d\nmsg: %s\n", ret, http_server_errcode_lookup[ret]);
    signal(SIGABRT, sig_handler);
    signal(SIGFPE, sig_handler);
    signal(SIGILL, sig_handler);
    signal(SIGINT, sig_handler);
    signal(SIGSEGV, sig_handler);
    signal(SIGTERM, sig_handler);
    HTTPServer_start(&server);
    return 0;
}
