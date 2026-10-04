#include <sys/stat.h>
#include <cerrno>

extern "C" {

int _close(int) {
    return -1;
}

int _fstat(int, struct stat *st) {
    st->st_mode = S_IFCHR;
    return 0;
}

int _isatty(int) {
    return 1;
}

int _lseek(int, int, int) {
    return 0;
}

int _read(int, char *, int) {
    return 0;
}

int _write(int, char *, int len) {
    return len;
}

void _exit(int status) {
    (void)status;
    while (1) {}
}

int _kill(int, int) {
    errno = EINVAL;
    return -1;
}

int _getpid(void) {
    return 1;
}

} // extern "C"
