#include <Uefi.h>
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>

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


  // Open kernel file
  EFI_FILE_PROTOCOL *file;
  status = root_dir->Open(
    root_dir, &file, L"kernel.elf", EFI_FILE_MODE_READ, 0);
  if (EFI_ERROR(status)) {
    Print(L"failed to open file: %r\n", status);
    Halt();
  }
  Print(L"File opened.\n");
  status = file->Close(file);
  if (EFI_ERROR(status)) {
    Print(L"failed to close file: %r\n", status);
    Halt();
  }
  Print(L"File closed.\n");


  Print(L"Hello, AIOS!\n");
  while(1);
  return EFI_SUCCESS;
}
