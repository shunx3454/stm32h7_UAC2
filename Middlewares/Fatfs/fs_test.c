#include "fs_test.h"
#include "FreeRTOS.h"
#include "main.h"
#include "task.h"
#include <stdio.h>
#include <string.h>
#include "bord_config.h"

#define MKFS_BUFSIZE 10240U
BYTE mkfs_buf[MKFS_BUFSIZE] __ATTR_SDRAM __ALIGNED(4);

void fatfs_mout(FATFS *fs, BYTE *fs_name) {
  FRESULT res;
  DWORD fre_clust, tot_sect, fre_sect;
  FATFS *pfs;
  MKFS_PARM fsfm;

  fsfm.fmt = FM_EXFAT;
  fsfm.align = 2048;
  fsfm.au_size = 4096;
  fsfm.n_fat = 1;

  do {
    /* mount file system */
    res = f_mount(fs, fs_name, 1);
    if (res == FR_NO_FILESYSTEM) {
      printf("Creating file system ...\r\n");
      res = f_mkfs(fs_name, &fsfm, mkfs_buf, MKFS_BUFSIZE);
      if (res != FR_OK) {
        SYS_ERR("f_mkfs");
        break;
      }
      SYS_LOG("FS", "File system create ok!");
    } else if (res) {
      SYS_ERR("f_mount");
      break;
    }
    /* get file system free */
    /* Get volume information and free clusters of drive 1 */
    res = f_getfree(fs_name, &fre_clust, &pfs);
    if (res == FR_OK) {
      /* Get total sectors and free sectors */
      tot_sect = (pfs->n_fatent - 2) * pfs->csize;
      fre_sect = fre_clust * pfs->csize;

      /* Print the free space (assuming 512 bytes/sector) */
      printf("\r\n%10lu KiB total drive space.\r\n%10lu KiB available.\r\n", tot_sect / 2, fre_sect / 2);
    }
  } while (0);
}

void fatfs_test(void) {
  FRESULT res;
  UINT nbw, nbr;
  FIL *fl0, *fl1, *fl2;
  const char *text = "This is a test file\r\n";
  const char *charzhcn = "中文字符测试，今天天气真好！";
  BYTE errcode = 0;

  /* mem alloc */
  fl0 = (FIL *)pvPortMalloc(sizeof(FIL) * 3);
  fl1 = fl0 + 1;
  fl2 = fl1 + 1;

  do {
    /* file function test */
    res = f_open(fl0, "/SD/text1.txt", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
    if (res) {
      SYS_ERR("f_open");
      errcode = 1;
      break;
    }
    res = f_write(fl0, text, (UINT)strlen(text), (UINT *)&nbw);
    if (res) {
      SYS_ERR("f_write");
      errcode = 2;
      errcode |= 0x10;
      break;
    }
    f_close(fl0);

    /* Multi files opt */
    res = f_open(fl0, "/SD/MUSIC/MUSIC.txt", FA_READ);
    if (res) {
      SYS_ERR("f_open");
      errcode = 3;
      break;
    }
    res = f_open(fl1, "/SD/text1.txt", FA_OPEN_EXISTING | FA_WRITE);
    if (res) {
      SYS_ERR("f_open");
      errcode = 4;
      errcode |= 0x10;
      break;
    }

    memset(mkfs_buf, 0, MKFS_BUFSIZE);
    f_read(fl0, mkfs_buf, MKFS_BUFSIZE, &nbr);
    printf("/SD/MUSIC/MUSIC.txt: read %d bytes\n", nbr);
    f_lseek(fl1, f_size(fl1));
    f_write(fl1, mkfs_buf, nbr, &nbw);
    printf("/SD/text1.txt: append %d bytes\n", nbw);

    /* close files */
    f_close(fl0);
    f_close(fl1);

    /* chinese char test */
    res = f_open(fl2, "/SD/这是一个中文件名测试文件.txt", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
    if (res) {
      printf("中文文件名测试失败\r\n");
      errcode = 5;
      break;
    }
    res = f_write(fl2, charzhcn, strlen(charzhcn), &nbw);
    if (res) {
      printf("中文件名写入失败\r\n");
      errcode = 6;
      errcode |= 0x40;
      break;
    }
    printf("成功写入 %u 字节\r\n", strlen(charzhcn));
    f_close(fl2);

    printf("开始读取中文文件...\r\n");
    res = f_open(fl2, "/SD/这是一个中文件名测试文件.txt", FA_READ);
    if (res) {
      printf("打开文件失败\r\n");
      errcode = 7;
      break;
    }
    memset(mkfs_buf, 0, MKFS_BUFSIZE);
    res = f_read(fl2, mkfs_buf, MKFS_BUFSIZE, &nbr);
    if (res) {
      printf("读取文件失败\r\n");
      errcode = 8;
      errcode |= 0x40;
      break;
    }
    printf("成功读取 %u 字节 \r\n%s\r\n", nbr, mkfs_buf);
    f_close(fl2);
  } while (0);
  errcode & 0x10 ? f_close(fl0) : (void)(0);
  errcode & 0x40 ? f_close(fl2) : (void)(0);

  vPortFree(fl0);
  printf("fatfs_test errcode: %d\r\n", errcode);
}