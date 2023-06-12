#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

extern size_t events_read(void *buf, size_t offset, size_t len);
extern size_t serial_write(const void *buf, size_t offset, size_t len);
extern size_t dispinfo_read(void *buf, size_t offset, size_t len);
extern size_t fb_write(const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_EVENT, FD_DISPINFO, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}



/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
  [FD_EVENT]  = {"/dev/events", 0, 0, events_read, invalid_write},
  [FD_DISPINFO] = {"/proc/dispinfo", 64, 0, dispinfo_read,invalid_write},
  [FD_FB]     = {"/dev/fb", 0, 0, invalid_read, fb_write},
#include "files.h"
};

void init_fs() {
  // TODO: initialize the size of /dev/fb
  AM_GPU_CONFIG_T gconf = io_read(AM_GPU_CONFIG);
  file_table[FD_FB].size = gconf.width * gconf.height * 4;
}

//begin
//进行一些预处理
extern size_t ramdisk_read(void *, size_t, size_t);
extern size_t ramdisk_write(const void*, size_t, size_t);
const int FD_SIZE = sizeof(file_table) / sizeof(file_table[0]);


int fs_open(const char *path){
  if(strcmp(path, file_table[FD_EVENT].name) == 0) return FD_EVENT;
  if(strcmp(path, file_table[FD_DISPINFO].name) == 0) return FD_DISPINFO;
  if(strcmp(path, file_table[FD_FB].name) == 0) return FD_FB;
  	for (int i = FD_FB + 1; i < FD_SIZE; i++) {
    if (strcmp(path, file_table[i].name) == 0) {
      file_table[i].open_offset = 0;
      return i;
    }
  }
  return -1;
}

size_t fs_read(int fd,void *buf,size_t len){
  // //处理count
  // if(fd>FD_FB && (file_table[fd].open_offset+count >= file_table[fd].size)){
  //   count=file_table[fd].size-file_table[fd].open_offset;
  //   if(count<0)count=0;
  // }
  // count = file_table[fd].read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, count);
  // file_table[fd].open_offset += count;
  // return count;

  Finfo *info = &file_table[fd];
  size_t real_len;
  if (info->read){
    real_len = info->read(buf, info->open_offset, len);
  }
  else {
    real_len = info->open_offset + len <= info->size ?
    len : info->size - info->open_offset;
    ramdisk_read(buf, info->disk_offset + info->open_offset, real_len);
  }
  info->open_offset += real_len;
  return real_len;

  // size_t actual_len = 0;
  // if (file_table[fd].open_offset + len > file_table[fd].size) {
  //   actual_len = file_table[fd].size - file_table[fd].open_offset;
  // }
  // else {
  //   actual_len = len;
  // }

  // if (file_table[fd].read) {
  //   file_table[fd].read(buf, file_table[fd].open_offset, actual_len);
  // }
  // else {
  //   ramdisk_read(buf, file_table[fd].open_offset, actual_len);
  // }
  // file_table[fd].open_offset += actual_len;
  // return actual_len;
}

size_t fs_lseek(int fd, size_t offset, int whence){
  switch(whence) {
    case SEEK_SET: file_table[fd].open_offset = offset; break;
    case SEEK_CUR: file_table[fd].open_offset += offset; break;
    case SEEK_END: file_table[fd].open_offset = file_table[fd].size + offset; break;
    default: assert(0);
  }

  return file_table[fd].open_offset;
}

size_t fs_write(int fd,const void *buf,size_t len){
  // if(fd>FD_FB && (file_table[fd].open_offset+count >= file_table[fd].size)){
  //   count=file_table[fd].size-file_table[fd].open_offset;
  //   if(count<0)count=0;
  // }
  // count = file_table[fd].write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, count);
  // file_table[fd].open_offset += count;
  // return count;

  Finfo *info = &file_table[fd];
  size_t real_len;
  
  if (info->write){
    real_len = info->write(buf, info->open_offset, len);
  }
  else {
    assert(info->open_offset + len <= info->size);
    ramdisk_write(buf, info->disk_offset + info->open_offset, len);
    real_len = len;
  }
  info->open_offset += real_len;
  return real_len;
}

int fs_close(int fd){
  file_table[fd].open_offset=0;
  return 0;
}