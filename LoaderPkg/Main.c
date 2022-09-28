#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/BaseMemoryLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>
#include <../src/frame_buffer.hpp>

//
// ELF typedef (spec here: https://uclibc.org/docs/elf-64-gen.pdf)
//
typedef void*              Elf64_Addr;
typedef unsigned long long Elf64_Off;
typedef unsigned short     Elf64_Half;
typedef unsigned int       Elf64_Word;
typedef unsigned long long Elf64_Xword;
#define PT_LOAD 1
// ELF header
typedef struct {
  char          a[24];
  Elf64_Addr    e_entry;     /* Entry point address */
  Elf64_Off     e_phoff;     /* Program header offset */
  char          b[14];
  Elf64_Half    e_phentsize; /* Size of program header entry */
  Elf64_Half    e_phnum;     /* Number of program header entries */
} Elf64_Ehdr;
// ELF Program header
typedef struct {
  Elf64_Word    p_type;      /* Type of segment */
  Elf64_Word    p_flags;     /* Segment attributes */
  Elf64_Off     p_offset;    /* Offset in file */
  Elf64_Addr    p_vaddr;     /* Virtual address in memory */
  Elf64_Addr    p_paddr;     /* Reserved */
  Elf64_Xword   p_filesz;    /* Size of segment in file */
  Elf64_Xword   p_memsz;     /* Size of segment in memory */
  Elf64_Xword   p_align;     /* Alignment of segment */
} Elf64_Phdr;


//
// Memory map typedef
//
typedef struct {
  unsigned long long map_size;
  void*              map_buffer;
  unsigned long long map_key;
  unsigned long long descriptor_size;
  unsigned int       descriptor_version;
} MemoryMap;

void Halt() {
  while (1) __asm__("hlt");
}

void GetMemoryMap(MemoryMap *map) {
  EFI_STATUS status;
  CHAR8 buf[4096*4];
  map->map_buffer = buf;
  map->map_size = sizeof(buf);
  status = gBS->GetMemoryMap(
    &map->map_size, (EFI_MEMORY_DESCRIPTOR*)map->map_buffer, &map->map_key,
    &map->descriptor_size, &map->descriptor_version);
  if (EFI_ERROR(status)) {
    Print(L"failed to get memory map: %r\n", status);
    if (status == EFI_BUFFER_TOO_SMALL) {
      Print(L"Given buffer size: %d bytes\n", sizeof(buf));
      Print(L"Needed buffer size: %d bytes\n", map->map_size);
    }
    Halt();
  }
}

void PrintMemoryMap(MemoryMap *map) {
  Print(L"Memory map copied.\n");
  Print(L"Memory map size: %d bytes\n", map->map_size);
  Print(L"Memory descriptor size: %d bytes\n", map->descriptor_size);
  EFI_MEMORY_DESCRIPTOR *dsc;
  for (int i = 0; i < map->map_size / map->descriptor_size; i++) {
    dsc = (EFI_MEMORY_DESCRIPTOR*)(map->map_buffer + i * map->descriptor_size);
    if (dsc->Type != EfiConventionalMemory) continue;
    Print(L"Memory descriptor %d, type: %d, p_start: %x, n_pages: %d\n"
      ,i, dsc->Type, dsc->PhysicalStart, dsc->NumberOfPages);
  }
}

//
// Open the root directory on the same volume as the code is read
//
void OpenRootDir(EFI_HANDLE image_handle, EFI_FILE_PROTOCOL **root_dir) {
  EFI_STATUS                      status;
  EFI_LOADED_IMAGE_PROTOCOL       *loaded_image;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *file_system;

  // Open the Loaded Image Protocol of the Image Handle (to get its Device Handle)
  status = gBS->OpenProtocol(
    image_handle,
    &gEfiLoadedImageProtocolGuid,
    (void**)&loaded_image,
    image_handle,
    NULL,
    EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL
  );
  if (EFI_ERROR(status)) {
    Print(L"failed to open loaded image protocol: %r\n", status);
    Halt();
  }
  Print(L"EFI loaded image protocol opened.\n");

  // Open the Simple File System Protocol of the Device Handle obtained above
  // You can get the File Protocol instance using OpenVolume provided by the SFSP.
  status = gBS->OpenProtocol(
    loaded_image->DeviceHandle,
    &gEfiSimpleFileSystemProtocolGuid,
    (void**)&file_system,
    image_handle,
    NULL,
    EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL
  );
  if (EFI_ERROR(status)) {
    Print(L"failed to open simple file system protocol: %r\n", status);
    Halt();
  }
  Print(L"EFI simple file system protocol opened.\n");

  status = file_system->OpenVolume(file_system, root_dir);
  if (EFI_ERROR(status)) {
    Print(L"failed to open volume: %r\n", status);
    Halt();
  }
  Print(L"Root dir opened.\n");
}

/*
 * Open the specified file in the given directory
 * and read the data into buffer
*/
void OpenAndReadFile(
  EFI_FILE_PROTOCOL *root_dir, CHAR16 *file_name, VOID **file_buf_ptr) {
  EFI_STATUS status;
  EFI_FILE_PROTOCOL *file;

  // Open file in read mode
  status = root_dir->Open(
    root_dir, &file, file_name, EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR(status)) {
    Print(L"failed to open file: %r\n", status);
    Halt();
  }
  Print(L"File opened.\n");

  // Get file info to look up the size of file and allocate enough memory
  UINTN file_info_size = sizeof(EFI_FILE_INFO) + 11 * sizeof(CHAR16);
  CHAR8 file_info_buf[file_info_size];
  status = file->GetInfo(
    file, &gEfiFileInfoGuid, &file_info_size, &file_info_buf);
  if (EFI_ERROR(status)) {
    Print(L"failed to get file info: %r\n", status);
    Halt();
  }

  // Allocate memory and read out the data into the space
  UINTN file_size = ((EFI_FILE_INFO*)file_info_buf)->FileSize;
  status = gBS->AllocatePool(EfiLoaderData, file_size, file_buf_ptr);
  if (EFI_ERROR(status)) {
    Print(L"failed to allocate pool: %r\n", status);
    Halt();
  }
  status = file->Read(file, &file_size, *file_buf_ptr);
  if (EFI_ERROR(status)) {
    Print(L"failed to read file: %r\n", status);
    Halt();
  }
  Print(L"File read.\n");

  // Close file
  status = file->Close(file);
  if (EFI_ERROR(status)) {
    Print(L"failed to close file: %r\n", status);
    Halt();
  }
  Print(L"File closed.\n");
}

//
// Load ELF-formatted EXEC file
// 
void LoadELF(void *file_buf) {
  EFI_STATUS status;
  Elf64_Ehdr *ehdr = (Elf64_Ehdr*)file_buf;
  Print(L"entry: %x\n", ehdr->e_entry);
  Print(L"phoff: %d\n", ehdr->e_phoff);
  Print(L"phentsize: %d\n", ehdr->e_phentsize);
  Print(L"phnum: %d\n", ehdr->e_phnum);

  // Loop for program headers, find LOAD type segments and copy them to proper address
  // Set 0 in specified memory space that doesn't have corresponding file contents
  // It becomes an error if the specified memory space is not available
  Elf64_Phdr phdr;
  for (int i = 0; i < ehdr->e_phnum; i++) {
    phdr = *(Elf64_Phdr*)(file_buf + ehdr->e_phoff + i * ehdr->e_phentsize);
    if ((int)phdr.p_type != PT_LOAD) continue;
    Print(L"PT_LOAD type segment %d, v_addr: %x, memsz: %d\n", i, phdr.p_vaddr, phdr.p_memsz);

    status = gBS->AllocatePages(
      AllocateAddress, EfiLoaderData,
      (phdr.p_memsz + 4095) / 4096, (EFI_PHYSICAL_ADDRESS*)&phdr.p_vaddr);
    if (EFI_ERROR(status)) {
      Print(L"failed to allocate pages: %r\n", status);
      Halt();
    }
    Print(L"Pages allocated for segment %d.\n", i);
    CopyMem((void*)phdr.p_vaddr, (void*)(file_buf + phdr.p_offset), phdr.p_filesz);
    if (phdr.p_filesz < phdr.p_memsz) {
      SetMem((void*)(phdr.p_vaddr + phdr.p_filesz), phdr.p_memsz - phdr.p_filesz, 0);
    }
    Print(L"File copied.\n");
  }
}

void GetFrameBuffer(FrameBuffer *frame_buffer) {
  EFI_STATUS status;
  EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
  status = gBS->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (void**)&gop);
  if (EFI_ERROR(status)) {
    Print(L"failed to locate graphics output protocol: %r\n", status);
    Halt();
  }
  FrameBuffer _frame_buffer = {
    (unsigned int*)gop->Mode->FrameBufferBase, gop->Mode->FrameBufferSize,
    gop->Mode->Info->HorizontalResolution, gop->Mode->Info->VerticalResolution,
    (PixelFormat)gop->Mode->Info->PixelFormat, gop->Mode->Info->PixelsPerScanLine};
  *frame_buffer = _frame_buffer;
}

/*
 * Entry point of the UEFI application
 * This will be called by the UEFI firmware on platforms with parameters shown below:
 *
 * @param image_handle   UEFI handle of this UEFI application
 * @param *system_table  Pointer to UEFI system table which contains pointers to
 *                       some devices, services and configuration tables
*/
EFI_STATUS EFIAPI UefiMain(
  EFI_HANDLE       image_handle,
  EFI_SYSTEM_TABLE *system_table
) {
  Print(L"Bootloader entry point loaded.\n");
  EFI_STATUS status;
  MemoryMap map;
  EFI_FILE_PROTOCOL *root_dir;
  VOID *file_buf;
  FrameBuffer frame_buffer;

  GetMemoryMap(&map);
  OpenRootDir(image_handle, &root_dir);
  OpenAndReadFile(root_dir, L"kernel.elf", &file_buf);
  LoadELF(file_buf);
  GetFrameBuffer(&frame_buffer);

  GetMemoryMap(&map);
  status = gBS->ExitBootServices(image_handle, map.map_key);
  if (EFI_ERROR(status)) {
    Print(L"failed to exit boot services: %r\n", status);
    Halt();
  }

  typedef void EntryPoint(FrameBuffer *frame_buffer);
  UINTN entry_point_address = *(UINTN*)(file_buf + 24); // ehdr->e_entry
  EntryPoint* entry_point = (EntryPoint*)entry_point_address;
  entry_point(&frame_buffer);

  return EFI_SUCCESS;
}
