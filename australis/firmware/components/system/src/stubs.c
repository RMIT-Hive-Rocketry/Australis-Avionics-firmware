/***********************************************************************************
 * @file        stubs.c                                                            *
 * @author      Matt Ricci                                                         *
 ***********************************************************************************/

extern int __io_putchar(int ch) __attribute__((weak));
extern int __io_getchar(void) __attribute__((weak));

__attribute__((weak)) int _read(int file, char *ptr, int len) {
  (void)file;
  for (int i = 0; i < len; i++) {
    *ptr++ = __io_getchar();
  }
  return len;
}

__attribute__((weak)) int _write(int file, char *ptr, int len) {
  (void)file;
  for (int i = 0; i < len; i++) {
    __io_putchar(*ptr++);
  }
  return len;
}

int _getpid(void) {
  return 1;
}

int _kill(int pid, int sig) {
  (void)pid;
  (void)sig;
  return -1;
}

void _exit(int status) {
  _kill(status, -1);
  while (1) {}
}

int _close(int file) {
  (void)file;
  return -1;
}

int _fstat(int file) {
  (void)file;
  return 0;
}

int _isatty(int file) {
  (void)file;
  return 1;
}

int _lseek(int file, int ptr, int dir) {
  (void)file;
  (void)ptr;
  (void)dir;
  return 0;
}
