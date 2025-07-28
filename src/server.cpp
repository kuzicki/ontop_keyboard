// #include "server.h"
// #include <ShlObj.h>
// #include <string>
//
// SOCKET g_listenSocket = INVALID_SOCKET;
// bool g_serverRunning = false;
// std::thread g_serverThread;
//
// static std::wstring current_folder_path;
//
//
// enum class ChooseFileStatus {
//   Success = 0,
//   OpenDialogNotFound,
//   NoFileNameEditControl,
//   FailedToOpen
// };
//
// ChooseFileStatus chooseFile(const std::wstring &filePath);
//
// void serverThread(std::function<void()> action) {
//   WSADATA wsaData;
//   if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
//     return;
//   }
//
//   g_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
//   if (g_listenSocket == INVALID_SOCKET) {
//     WSACleanup();
//     return;
//   }
//
//   sockaddr_in serverAddr{};
//   serverAddr.sin_family = AF_INET;
//   serverAddr.sin_addr.s_addr = INADDR_ANY;
//   serverAddr.sin_port = htons(4949);
//
//   if (bind(g_listenSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) ==
//       SOCKET_ERROR) {
//     closesocket(g_listenSocket);
//     WSACleanup();
//     return;
//   }
//
//   if (listen(g_listenSocket, SOMAXCONN) == SOCKET_ERROR) {
//     closesocket(g_listenSocket);
//     WSACleanup();
//     return;
//   }
//
//   g_serverRunning = true;
//   while (g_serverRunning) {
//     SOCKET client = accept(g_listenSocket, nullptr, nullptr);
//     if (client == INVALID_SOCKET)
//       continue;
//     std::string fullRequest;
//     char tempBuffer[1024];
//     while (true) {
//       int bytesRead = recv(client, tempBuffer, sizeof(tempBuffer) - 1, 0);
//       if (bytesRead <= 0)
//         break;
//       tempBuffer[bytesRead] = '\0';
//       fullRequest += tempBuffer;
//       if (fullRequest.find("\r\n\r\n") != std::string::npos)
//         break;
//     }
//
//     if (strstr(tempBuffer, "GET /do-something") != nullptr) {
//       action();
//
//       const char *response = "HTTP/1.1 200 OK\r\n"
//                              "Content-Type: text/plain\r\n"
//                              "Access-Control-Allow-Origin: *\r\n"
//                              "\r\n"
//                              "Success";
//       send(client, response, (int)strlen(response), 0);
//     } else {
//       const char *notFound = "HTTP/1.1 404 Not Found\r\n\r\n";
//       send(client, notFound, (int)strlen(notFound), 0);
//     }
//     closesocket(client);
//   }
//
//   closesocket(g_listenSocket);
//   WSACleanup();
// }
//
// void pickFolder() {
//   std::wstring folderPath;
//
//   if (SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED |
//                                             COINIT_DISABLE_OLE1DDE))) {
//     BROWSEINFOW bi = {0};
//     bi.hwndOwner = nullptr;
//     bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
//     bi.lpszTitle = L"Select a folder:";
//
//     LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
//
//     if (pidl != NULL) {
//       wchar_t path[MAX_PATH];
//       if (SHGetPathFromIDListW(pidl, path)) {
//         folderPath = path;
//       }
//
//       CoTaskMemFree(pidl);
//     }
//
//     CoUninitialize();
//   }
//   current_folder_path = std::move(folderPath);
// }
//
// void uploadNextFile() {
// 	chooseFile(); 
// }
//
// HWND findOpenFileDialog() {
//   HWND hWnd = nullptr;
//   for (int i = 0; i < 50 && !hWnd; ++i) {
//     // Find any top‑level window whose title is exactly "Open"
//     hWnd = FindWindowW(nullptr, L"File upload");
//     std::this_thread::sleep_for(std::chrono::milliseconds(100));
//   }
//   return hWnd;
// }
//
// // Walk down: Dialog → ComboBoxEx32 → ComboBox → Edit
// HWND findFileNameEdit(HWND hDialog) {
//   // 1) ComboBoxEx32
//   HWND hComboEx = FindWindowExW(hDialog, nullptr, L"ComboBoxEx32", nullptr);
//   if (!hComboEx)
//     return nullptr;
//
//   // 2) ComboBox
//   HWND hCombo = FindWindowExW(hComboEx, nullptr, L"ComboBox", nullptr);
//   if (!hCombo)
//     return nullptr;
//
//   // 3) Edit inside ComboBox
//   HWND hEdit = FindWindowExW(hCombo, nullptr, L"Edit", nullptr);
//   return hEdit;
// }
//
// ChooseFileStatus chooseFile(const std::wstring &filePath) {
//   // 1) wait for the dialog
//   // HWND hDialog = //findOpenFileDialog();
//   HWND hDialog = FindWindowExW(nullptr, nullptr, L"#32770", nullptr);
//   if (!hDialog) {
//     return ChooseFileStatus::OpenDialogNotFound;
//   }
//
//   // 2) find File name edit box
//   HWND hEdit = findFileNameEdit(hDialog);
//   if (!hEdit) {
//     return ChooseFileStatus::NoFileNameEditControl;
//   }
//
//   // 3) set the text
//   SendMessageW(hEdit, WM_SETTEXT, 0, (LPARAM)filePath.c_str());
//
//   // 4) find & click the Open button
//   //    (text is locale‑dependent; could be "&Open" or "Open")
//   HWND hOpenBtn = FindWindowExW(hDialog, nullptr, L"Button", L"&Open");
//   if (!hOpenBtn)
//     hOpenBtn = FindWindowExW(hDialog, nullptr, L"Button", L"Open");
//   if (!hOpenBtn) {
//     return ChooseFileStatus::FailedToOpen;
//   }
//   SendMessageW(hOpenBtn, BM_CLICK, 0, 0);
// }
