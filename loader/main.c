#define IN
#define EFIAPI
#define EFI_SUCCESS 0

typedef unsigned short CHAR16;
typedef unsigned long UINT32;
typedef unsigned long long UINT64;
typedef unsigned long long UINTN;
typedef UINTN EFI_STATUS;
typedef void *EFI_HANDLE;
typedef void *EFI_EVENT;


typedef struct {
  UINT64 Signature;
  UINT32 Revision;
  UINT32 HeaderSize;
  UINT32 CRC32;
  UINT32 Reserved;
} EFI_TABLE_HEADER;

typedef EFI_STATUS (EFIAPI *EFI_INPUT_RESET)();
typedef EFI_STATUS (EFIAPI *EFI_INPUT_READ_KEY)();
typedef struct {
  EFI_INPUT_RESET    Reset;
  EFI_INPUT_READ_KEY ReadKeyStroke;
  EFI_EVENT          WaitForKey;
} EFI_SIMPLE_TEXT_INPUT_PROTOCOL;

struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;
typedef EFI_STATUS (EFIAPI *EFI_TEXT_RESET)();
typedef EFI_STATUS (EFIAPI *EFI_TEXT_STRING)(
  IN struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This,
  IN CHAR16                                  *String
);
typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
  EFI_TEXT_RESET  Reset;
  EFI_TEXT_STRING OutputString;
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;


typedef struct {
  EFI_TABLE_HEADER Hdr;
  CHAR16                          *FirmwareVendor;
  UINT32                          FirmwareRevision;
  EFI_HANDLE                      ConsoleInHandle;
  EFI_SIMPLE_TEXT_INPUT_PROTOCOL  *ConIn;
  EFI_HANDLE                      ConsoleOutHandle;
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
