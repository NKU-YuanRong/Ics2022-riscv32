#include <proc.h>
#include <elf.h>
#include <fs.h>

#ifdef __LP64__
# define Elf_Ehdr Elf64_Ehdr
# define Elf_Phdr Elf64_Phdr
#else
# define Elf_Ehdr Elf32_Ehdr
# define Elf_Phdr Elf32_Phdr
#endif
extern size_t ramdisk_read(void *buf, size_t offset, size_t len);
extern size_t ramdisk_write(const void *buf, size_t offset, size_t len);
extern size_t get_ramdisk_size();
static uintptr_t loader(PCB *pcb, const char *filename) {
  Elf32_Ehdr header;
	int fd = fs_open(filename);
	fs_read(fd, &header, sizeof(Elf32_Ehdr));
  Log("header.e_ident=%x,it should be 0x464c457f",*(uint32_t *)header.e_ident );
  assert(*(uint32_t*)header.e_ident==0x464c457f);

  Elf32_Phdr Phdr;
  for (int i = 0; i < header.e_phnum; i++) {
    fs_lseek(fd, header.e_phoff + i*header.e_phentsize, SEEK_SET);
    fs_read(fd, &Phdr, sizeof(Phdr));
    if (Phdr.p_type == PT_LOAD) {
      fs_lseek(fd, Phdr.p_offset, SEEK_SET);
      fs_read(fd, (void*)Phdr.p_vaddr, Phdr.p_filesz);
      for(unsigned int i = Phdr.p_filesz; i < Phdr.p_memsz;i++){
          ((char*)Phdr.p_vaddr)[i] = 0;
      }
    }
  }
	return header.e_entry;
}

void naive_uload(PCB *pcb, const char *filename) {
  uintptr_t entry = loader(pcb, filename);
  Log("Jump to entry = %p", entry);
  ((void(*)())entry) ();
}

