#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>

void Halt(void) {
  while (1) __asm__("hlt");
}

//
// Open the root directory on the same volume as the code is read
//
EFI_STATUS OpenRootDir(EFI_HANDLE image_handle, EFI_FILE_PROTOCOL **root_dir) {
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
    return status;
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
    return status;
  }
  Print(L"EFI simple file system protocol opened.\n");

  return file_system->OpenVolume(file_system, root_dir);
}

EFI_STATUS ReadFile(EFI_FILE_PROTOCOL *file, VOID **file_buf_ptr) {
  EFI_STATUS status;

  UINTN file_info_size = sizeof(EFI_FILE_INFO) + 11 * sizeof(CHAR16);
  CHAR8 file_info_buf[file_info_size];
  status = file->GetInfo(
    file, &gEfiFileInfoGuid, &file_info_size, &file_info_buf);
  if (EFI_ERROR(status)) {
    return status;
  }

  UINTN file_size = ((EFI_FILE_INFO*)file_info_buf)->FileSize;
  status = gBS->AllocatePool(EfiLoaderData, file_size, file_buf_ptr);
  if (EFI_ERROR(status)) {
    return status;
  }

  return file->Read(file, &file_size, *file_buf_ptr);
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


  // Open root directory
  EFI_FILE_PROTOCOL *root_dir;
  status = OpenRootDir(image_handle, &root_dir);
  if (EFI_ERROR(status)) {
    Print(L"failed to open root directory: %r\n", status);
    Halt();
  }
  Print(L"Root dir opened.\n");


  // Open and read kernel file
  EFI_FILE_PROTOCOL *file;
  VOID              *file_buf;
  status = root_dir->Open(
    root_dir, &file, L"kernel.elf", EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR(status)) {
    Print(L"failed to open file: %r\n", status);
    Halt();
  }
  Print(L"File opened.\n");
  status = ReadFile(file, &file_buf);
  if (EFI_ERROR(status)) {
    Print(L"failed to read file: %r\n", status);
    Halt();
  }
  Print(L"File read.\n");
  status = file->Close(file);
  if (EFI_ERROR(status)) {
    Print(L"failed to close file: %r\n", status);
    Halt();
  }
  Print(L"File closed.\n");


  // Allocate memory for kernel


  // Get memory map
  CHAR8 map[4096*4];
  UINTN map_size=sizeof(map), map_key, descriptor_size;
  UINT32 descriptor_version;
  status = gBS->GetMemoryMap(
    &map_size, (EFI_MEMORY_DESCRIPTOR*)map, &map_key,
    &descriptor_size, &descriptor_version);
  if (EFI_ERROR(status)) {
    Print(L"failed to get memory map: %r\n", status);
    if (status == EFI_BUFFER_TOO_SMALL) {
      Print(L"Needed buffer size: %d bytes\n", map_size);
    }
    Halt();
  }
  Print(L"Memory map copied.\n");
  Print(L"Memory map size: %d bytes\n", map_size);
  Print(L"Memory descriptor size: %d bytes\n", descriptor_size);

  Print(L"Hello, AIOS!\n");
  while(1);
  return EFI_SUCCESS;
}
