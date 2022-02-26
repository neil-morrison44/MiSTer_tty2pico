#include "boards/pimoroni_tiny2040.h"
#include "pico/stdlib.h"
#include "flash_io.h"
#include "ff.h"
#include "stdio.h"

BYTE Buff[4096];
FATFS FatFs; /* File system object for each logical drive */
FIL File;    /* File object */
DIR Dir;     /* Directory object */
FILINFO Finfo;

void makeFileSystem() { f_mkfs("", 0, Buff, sizeof Buff); }

void loop() {
  while (true) {
    printf("hello! \n");
    sleep_ms(1000 * 20);
  }
}

int main() {
  stdio_init_all();
  sleep_ms(5000);
  printf("first hello! \n");
  sleep_ms(1000);
  printf("second hello! '\n");

  FRESULT res;
  UINT bw;
  printf("hello? \n");

  res = f_mkfs("", 0, Buff, sizeof Buff);
  if (res != 0) {
    printf("failed to f_mkfs \n");
  }
  res = f_mount(&FatFs, "", 0);
  if (res != 0) {
    printf("failed to mount \n");
  }
  res = f_open(&File, "hello.txt", FA_CREATE_NEW | FA_WRITE);
  if (res != 0) {
    printf("failed to open \n");
  }
  res = f_write(&File, "Hello, World!\r\n", 15, &bw);
  if (res != 0) {
    printf("failed to write \n");
  }
  res = f_close(&File);
  if (res != 0) {
    printf("failed to close \n");
  }
  res = f_mount(0, "", 0);
  if (res != 0) {
    printf("failed to unmount \n");
  }
  sleep_ms(1000);
  res = f_mount(&FatFs, "", 0);
  if (res != 0) {
    printf("failed to remount \n");
  }
  char line[100]; /* Line buffer */

  res = f_open(&File, "hello.txt", FA_READ);
  if (res != 0) {
    printf("failed to reopen \n");
  }
  while (f_gets(line, sizeof line, &File)) {
    printf(line);
    printf("\n");
  }

  loop();

  return 0;
}
