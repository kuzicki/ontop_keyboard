#include "server.h"
#include "utils.h"
#include <ShlObj.h>
#include <filesystem>
#include <print>

SOCKET g_listenSocket = INVALID_SOCKET;
bool g_serverRunning = false;
std::thread g_serverThread;

std::vector<std::filesystem::path> g_fileQueue;
size_t g_fileIndex = 0;

static std::wstring current_folder_path;

enum class ChooseFileStatus {
  Success = 0,
  OpenDialogNotFound,
  NoFileNameEditControl,
  FailedToOpen
};

static std::string file_status_str(ChooseFileStatus &status) {
  switch (status) {
  case ChooseFileStatus::Success:
    return "success";
  case ChooseFileStatus::OpenDialogNotFound:
    return "open dialog wasn't found";
  case ChooseFileStatus::NoFileNameEditControl:
    return "filename edit control not found";
  case ChooseFileStatus::FailedToOpen:
    return "failed to click Open button";
  default:
    return "unknown status";
  }
}
ChooseFileStatus chooseFile(const std::wstring &filePath);

void serverThread() {
  WSADATA wsaData;
  if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
    return;
  }

  g_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (g_listenSocket == INVALID_SOCKET) {
    WSACleanup();
    return;
  }

  sockaddr_in serverAddr{};
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_addr.s_addr = INADDR_ANY;
  serverAddr.sin_port = htons(4949);

  if (bind(g_listenSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) ==
      SOCKET_ERROR) {
    closesocket(g_listenSocket);
    WSACleanup();
    return;
  }

  if (listen(g_listenSocket, SOMAXCONN) == SOCKET_ERROR) {
    closesocket(g_listenSocket);
    WSACleanup();
    return;
  }

  g_serverRunning = true;
  while (g_serverRunning) {
    SOCKET client = accept(g_listenSocket, nullptr, nullptr);
    if (client == INVALID_SOCKET)
      continue;
    std::string fullRequest;
    char tempBuffer[1024];
    while (true) {
      int bytesRead = recv(client, tempBuffer, sizeof(tempBuffer) - 1, 0);
      if (bytesRead <= 0)
        break;
      tempBuffer[bytesRead] = '\0';
      fullRequest += tempBuffer;
      if (fullRequest.find("\r\n\r\n") != std::string::npos)
        break;
    }

    std::print("[IN]: {}", fullRequest);

    if (strstr(tempBuffer, "GET /pick-folder") != nullptr) {
      pickFolder();
      const char *response = "HTTP/1.1 200 OK\r\n"
                             "Content-Type: text/plain\r\n"
                             "Access-Control-Allow-Origin: *\r\n"
                             "\r\n"
                             "Success";
      send(client, response, (int)strlen(response), 0);
    } else if (strstr(tempBuffer, "GET /next-file") != nullptr) {
      std::string fileUploadResponse = uploadNextFile();

      std::string response = "HTTP/1.1 200 OK\r\n"
                             "Content-Type: text/plain\r\n"
                             "Access-Control-Allow-Origin: *\r\n"
                             "\r\n" +
                             fileUploadResponse;
      std::print("[OUT]: {}", fileUploadResponse);
      send(client, response.c_str(), (int)response.length(), 0);

    } else {
      const char *notFound = "HTTP/1.1 404 Not Found\r\n\r\n";
      send(client, notFound, (int)strlen(notFound), 0);
    }
    closesocket(client);
  }

  closesocket(g_listenSocket);
  WSACleanup();
}

void initializeFileQueue() {
  g_fileQueue.clear();
  g_fileIndex = 0;

  if (!current_folder_path.empty()) {
    for (const auto &entry :
         std::filesystem::directory_iterator(current_folder_path)) {
      if (entry.is_regular_file()) {
        g_fileQueue.push_back(entry.path());
      }
    }
  }
}

void pickFolder() {
  std::wstring folderPath;

  if (SUCCEEDED(CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED |
                                            COINIT_DISABLE_OLE1DDE))) {
    auto bi = zero_init<BROWSEINFOW>();
    bi.hwndOwner = nullptr;
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    bi.lpszTitle = L"Select a folder:";

    LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);

    if (pidl != NULL) {
      wchar_t path[MAX_PATH];
      if (SHGetPathFromIDListW(pidl, path)) {
        folderPath = path;

        initializeFileQueue();
        current_folder_path = std::move(folderPath);
      }

      CoTaskMemFree(pidl);
    }

    CoUninitialize();
  } else {
  	current_folder_path = std::wstring(L"");
  	initializeFileQueue();
  }
}

std::string uploadNextFile() {
  std::string responseOut;

  if (current_folder_path.empty()) {
    return "No folder selected.\n";
  }

  if (g_fileIndex >= g_fileQueue.size()) {
    return "DONE\n";
  }

  const auto &path = g_fileQueue[g_fileIndex++];
  ChooseFileStatus status = chooseFile(path.wstring());

  if (status == ChooseFileStatus::Success) {
    responseOut += "Uploaded: " + path.string() + "\n";
  } else {
    responseOut += "Failed: " + path.string() + "\n";
    responseOut += file_status_str(status) + "\n";
  }

  if (g_fileIndex >= g_fileQueue.size()) {
    responseOut += "DONE\n";
  }

  return responseOut;
}

HWND findOpenFileDialog() {
  HWND hWnd = nullptr;
  for (int i = 0; i < 50 && !hWnd; ++i) {

    hWnd = FindWindowW(nullptr, L"File upload");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }
  return hWnd;
}

HWND findFileNameEdit(HWND hDialog) {
  HWND hComboEx = FindWindowExW(hDialog, nullptr, L"ComboBoxEx32", nullptr);
  if (!hComboEx)
    return nullptr;

  HWND hCombo = FindWindowExW(hComboEx, nullptr, L"ComboBox", nullptr);
  if (!hCombo)
    return nullptr;

  HWND hEdit = FindWindowExW(hCombo, nullptr, L"Edit", nullptr);
  return hEdit;
}

ChooseFileStatus chooseFile(const std::wstring &filePath) {
  HWND hDialog = FindWindowExW(nullptr, nullptr, L"#32770", nullptr);
  if (!hDialog) {
    return ChooseFileStatus::OpenDialogNotFound;
  }

  HWND hEdit = findFileNameEdit(hDialog);
  if (!hEdit) {
    return ChooseFileStatus::NoFileNameEditControl;
  }

  SendMessageW(hEdit, WM_SETTEXT, 0, (LPARAM)filePath.c_str());


  HWND hOpenBtn = FindWindowExW(hDialog, nullptr, L"Button", L"&Open");
  if (!hOpenBtn)
    hOpenBtn = FindWindowExW(hDialog, nullptr, L"Button", L"Open");
  if (!hOpenBtn) {
    return ChooseFileStatus::FailedToOpen;
  }
  SendMessageW(hOpenBtn, BM_CLICK, 0, 0);
  return ChooseFileStatus::Success;
}
