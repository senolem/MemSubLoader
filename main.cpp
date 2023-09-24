#ifndef UNICODE
#define UNICODE
#endif
#define _WIN32_WINNT 0x0501

#include <windows.h>
#include <iostream>
#include <shlwapi.h>
#include <fstream>
#include <string>
#include <vector>

#define GAME 1
#define TRANSLATION 2
#define START 3

HWND game_text = NULL;
HWND file_text = NULL;
HWND subtitles = NULL;
wchar_t gamePath[MAX_PATH] = {};
wchar_t filePath[MAX_PATH] = {};

//Find address containing audio ID
void findAddress(uintptr_t &address, int offset, HANDLE hProcess)
{
    SIZE_T bytesRead;
    if (ReadProcessMemory(hProcess, (LPCVOID)address, &address, 4, &bytesRead) )
            address += offset;
}

//Used when the game starts
void game_start(PROCESS_INFORMATION pi)
{
    SIZE_T bytesRead;
    int audID;
    uintptr_t baseaddress;
    uintptr_t addressToRead;
    std::vector <std::wstring> Text;

    std::wstring ws( filePath );
    std::string SfilePath( ws.begin(), ws.end() );
    //Opens file containing base address, offsets and text for subtitles
    std::wifstream subfile;
    subfile.open(SfilePath);
    int num;
    int offset[6];
    if(subfile.is_open() && !subfile.eof())
    {
        subfile>>baseaddress;
        subfile>>num;
        for(int i = 0; i<num;i++)
           subfile>>offset[i];
        while(getline(subfile, ws))
        {
            Text.push_back(ws);
        }
    }
    else
        SetWindowText(subtitles,L"File failed to open");
    subfile.close();

    int lastID = 0;
    DWORD Width = 0;
    DWORD Height = 0;
    while (WaitForSingleObject( pi.hProcess, 0 ) == WAIT_TIMEOUT)
    {
        //Read memory of audioID's address
        if (ReadProcessMemory(pi.hProcess, (LPCVOID)addressToRead, &audID, sizeof(audID), &bytesRead) && lastID!=audID && audID <Text.size() && audID >0)
        {
            SetWindowText(subtitles,Text[audID].c_str());
            lastID=audID;
        }
        //process messages, otherwise the software will freeze
        MSG msg = { };
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        addressToRead = baseaddress;
        for( int i = 0; i < num; i++)
            findAddress(addressToRead,offset[i],pi.hProcess);
        //If width and height of the screen changed, resize and reposition the subtitles window
        if(Width != GetSystemMetrics(SM_CXSCREEN) && Height != GetSystemMetrics(SM_CYSCREEN))
        {
            Width = GetSystemMetrics(SM_CXSCREEN);
            Height = GetSystemMetrics(SM_CYSCREEN);
            SetWindowPos(subtitles,HWND_TOPMOST, 0,Height-100,Width,100,0);
        }

    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    DestroyWindow(subtitles);
}

//Open File Explorer
bool OpenFileExplorer(HWND hwnd, wchar_t* filePath, int filePathSize, int button)
{
    OPENFILENAME ofn = { sizeof(OPENFILENAME) };
    ofn.hwndOwner = hwnd;
    if(button == GAME)
        ofn.lpstrFilter = L"Executable Files (*.exe)\0*.exe\0All Files (*.*)\0*.*\0";
    else
        ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = filePathSize;
    ofn.Flags = OFN_FILEMUSTEXIST;
    if (GetOpenFileName(&ofn))
        return true;
    else
        return false;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case GAME:
            if (OpenFileExplorer(hwnd, gamePath, MAX_PATH, LOWORD(wParam)))
                SetWindowText(game_text, PathFindFileName(gamePath));
        break;

        case TRANSLATION:
            if (OpenFileExplorer(hwnd, filePath, MAX_PATH, LOWORD(wParam)))
                SetWindowText(file_text, PathFindFileName(filePath));
        break;

        case START:
        {
            //When both game and file was selected
            if(gamePath[0] != NULL && filePath[0] != NULL)
            {
                STARTUPINFO si;
                PROCESS_INFORMATION pi;
                ZeroMemory( &si, sizeof(si) );
                si.cb = sizeof(si);
                ZeroMemory( &pi, sizeof(pi) );
                //Opens the game
                CreateProcess(gamePath, NULL,NULL,NULL,FALSE,0,NULL, NULL, &si,&pi);
                WaitForInputIdle(pi.hProcess, INFINITE);
                //Creates subtitles window
                subtitles = CreateWindowEx(WS_EX_LAYERED|WS_EX_TRANSPARENT,L"STATIC", L"", WS_VISIBLE|WS_POPUP , 50, 100, 640, 100, hwnd, NULL, NULL, NULL);
                SetLayeredWindowAttributes(subtitles, RGB(255, 255, 255), 128, LWA_ALPHA);
                SetWindowText(subtitles, L"Loading...");
                game_start(pi);
            }
            else
                MessageBox(hwnd, L"You must select both game and file", L"Warning", MB_ICONINFORMATION);
            }
        break;
        }
    break;
    case WM_DESTROY:
        PostQuitMessage(0);
    break;

    case WM_CREATE:
    {
        HFONT hFont = CreateFont(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
        game_text   = CreateWindow(L"STATIC", L"No Game", WS_CHILD | WS_VISIBLE, 50, 100, 150, 20, hwnd, NULL, NULL, NULL);
        file_text   = CreateWindow(L"STATIC", L"No File", WS_CHILD | WS_VISIBLE, 440, 100, 150, 20, hwnd, NULL, NULL, NULL);
        SendMessage(game_text, WM_SETFONT, (WPARAM)hFont, TRUE);
        SendMessage(file_text, WM_SETFONT, (WPARAM)hFont, TRUE);
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
        EndPaint(hwnd, &ps);
    }
    break;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[]  = L"Sample Window Class";

    WNDCLASS wc      = { };
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    GetWindowRect(hDesktop, &desktop);
    //Creates main window and positions it at the middle of the screen
    HWND hwnd = CreateWindowEx(0,CLASS_NAME,L"MemSubLoader",WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,desktop.right/2 - 320,desktop.bottom/2 - 120,640,240,NULL,NULL,hInstance,NULL);
    if (hwnd == NULL)
        return 0;

    //Creates three buttons
    CreateWindow(L"BUTTON", L"Choose Game", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 50, 50, 150, 50, hwnd, (HMENU)GAME, hInstance, NULL);
    CreateWindow(L"BUTTON", L"Choose File", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 440, 50, 150, 50, hwnd, (HMENU)TRANSLATION, hInstance, NULL);
    CreateWindow(L"BUTTON", L"Start", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 245, 150, 150, 50, hwnd, (HMENU)START, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);

    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}
