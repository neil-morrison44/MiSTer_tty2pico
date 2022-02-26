#include "boards/pimoroni_tiny2040.h"
#include "pico/stdlib.h"
#include "ff.h"
#include "stdio.h"
#include "stdlib.h"

// BYTE Buff[4096];
FATFS fs; /* File system object for each logical drive */
// FIL File; /* File object */
// DIR Dir;  /* Directory object */
// FILINFO Finfo;

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

  // mount file system
  FRESULT res = f_mount(&fs, "", 1);
  if (FR_OK == res) {
    printf("f_mount succeeded! \n");
  } else {
    printf("f_mount failed, result: %d. \n", res);
  }

  // if filesystem mount failed due to no filesystem, attempt to make it
  if (FR_NO_FILESYSTEM == res) {
    printf("No filesystem present. Attempting to make file system..  \n");
    uint8_t *work_buffer = malloc(FF_MAX_SS);
    if (!work_buffer) {
      printf(
          "Unable to allocate f_mkfs work buffer. File system not created. \n");
    } else {
      // make the file system
      res = f_mkfs("", 0, work_buffer, FF_MAX_SS);
      if (FR_OK != res) {
        printf("f_mkfs failed, result: %d. \n",
               res);  // fs make failure
      } else {
        printf("f_mkfs succeeded! \n");  // fs make success
        // retry mount
        res = f_mount(&fs, "", 1);
        if (FR_OK == res) {
          printf("f_mount succeeded!");
        } else {
          printf("f_mount failed, result: %d. \n", res);
        }
      }

      free(work_buffer);
    }
  }

  loop();

  return 0;
}
