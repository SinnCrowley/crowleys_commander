#ifndef WINDOWS_FILE_OPERATIONS_H
#define WINDOWS_FILE_OPERATIONS_H

#include <windows.h>      // Standard include
#include <Shellapi.h>     // Included for shell constants such as FO_* values
#include <shlobj.h>       // Required for necessary shell dependencies
#include <strsafe.h>      // Including StringCch* helpers

class WindowsFileOperations {
public:
    HRESULT CreateAndInitializeFileOperation(REFIID riid, void **ppv);
    HRESULT CreateSampleFolders(IShellItem *psiSampleRoot, IShellItem **ppsiSampleSrc, IShellItem **ppsiSampleDst);
    HRESULT CreateSampleFiles(IShellItem *psiFolder);
    HRESULT DeleteSampleFiles(IShellItem *psiSrc, IShellItem *psiDst);
    HRESULT CopySingleFile(IShellItem *psiSrc, IShellItem *psiDst);
    HRESULT CreateShellItemArrayOfSampleFiles(IShellItem *psiSrc, REFIID riid, void **ppv);
    HRESULT CopyMultipleFiles(IShellItem *psiSrc, IShellItem *psiDst);
};

#endif // WINDOWS_FILE_OPERATIONS_H
