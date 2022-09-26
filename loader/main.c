#define IN
#define EFIAPI
#define EFI_SUCCESS 0

typedef unsigned short CHAR16;
typedef unsigned long UINT32;
typedef unsigned long long UINT64;
typedef unsigned long long UINTN;
typedef UINTN EFI_STATUS;
typedef void *EFI_HANDLE;


struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;
typedef EFI_STATUS (EFIAPI *EFI_TEXT_STRING)(
  IN struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
  IN CHAR16                                  *String
);
typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
  void            *a;
  EFI_TEXT_STRING OutputString;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;


typedef struct {
  char a[60];
  EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
} EFI_SYSTEM_TABLE;


// Entry point
EFI_STATUS EFIAPI EfiMain(
  IN EFI_HANDLE       ImageHandle,
  IN EFI_SYSTEM_TABLE *SystemTable
) {
  SystemTable->ConOut->OutputString(SystemTable->ConOut, L"Hello World!\n");
  while(1);
  return EFI_SUCCESS;
}
