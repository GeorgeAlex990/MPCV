#include <windows.h>
#include <string>
#include <sstream>

#pragma comment(lib, "user32.lib")

// Global variables
HINSTANCE hInst;
HWND hSensorLabels[4];
HANDLE hSerial;

// Function declarations
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitSerialPort();
void CloseSerialPort();
void SendCommand(const std::string& command);
DWORD WINAPI SerialThread(LPVOID lpParam);
void ParseAndDisplayData(const std::string& data);

// Custom message identifier for updating sensor labels
#define WM_UPDATE_SENSOR_LABELS (WM_USER + 1)

// Sensor data structure
struct SensorData {
    int sensorValues[4];
};

SensorData gSensorData; // Global sensor data

// Entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    hInst = hInstance;

    // Register the window class
    const char CLASS_NAME[] = "SensorControlWindowClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInst;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClass(&wc);

    // Create the main window
    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        "HC-SR04 Sensor Interface",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 250,
        NULL,
        NULL,
        hInst,
        NULL
    );

    if (!hwnd) {
        MessageBox(NULL, "Failed to create window.", "Error", MB_OK | MB_ICONERROR);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    // Initialize serial communication
    InitSerialPort();

    // Start the serial reading thread
    HANDLE hThread = CreateThread(NULL, 0, SerialThread, hwnd, 0, NULL);

    // Message loop
    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Cleanup
    CloseHandle(hThread);
    CloseSerialPort();

    return 0;
}

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hButtons[4];
    static const char* buttonLabels[] = { "Forward", "Backward", "Left", "Right" };
    static const int buttonCommands[] = { 1, 2, 3, 4 };

    switch (uMsg) {
    case WM_CREATE: {
        // Create sensor labels
        for (int i = 0; i < 4; ++i) {
            std::string label = "Sensor " + std::to_string(i + 1) + ": ---";
            hSensorLabels[i] = CreateWindow(
                "STATIC", label.c_str(),
                WS_CHILD | WS_VISIBLE,
                20, 20 + i * 30, 200, 20,
                hwnd, NULL, hInst, NULL
            );
        }

        // Create control buttons
        for (int i = 0; i < 4; ++i) {
            hButtons[i] = CreateWindow(
                "BUTTON", buttonLabels[i],
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                20 + i * 90, 150, 80, 30,
                hwnd, (HMENU)(buttonCommands[i]),
                hInst, NULL
            );
        }

        // Set focus to the window to capture keyboard input
        SetFocus(hwnd);
        break;
    }
    case WM_COMMAND: {
        int command = LOWORD(wParam);
        switch (command) {
        case 1: SendCommand("F"); break; // Forward
        case 2: SendCommand("B"); break; // Backward
        case 3: SendCommand("L"); break; // Left
        case 4: SendCommand("R"); break; // Right
        }
        break;
    }
    case WM_KEYDOWN: {
        switch (wParam) {
        case 'W':
        case 'w':
            SendCommand("F"); // Forward
            break;
        case 'S':
        case 's':
            SendCommand("B"); // Backward
            break;
        case 'A':
        case 'a':
            SendCommand("L"); // Left
            break;
        case 'D':
        case 'd':
            SendCommand("R"); // Right
            break;
        }
        break;
    }
    case WM_UPDATE_SENSOR_LABELS: {
        // Update sensor labels with the new data
        for (int i = 0; i < 4; ++i) {
            std::string label = "Sensor " + std::to_string(i + 1) + ": " + std::to_string(gSensorData.sensorValues[i]);
            SetWindowText(hSensorLabels[i], label.c_str());
        }
        break;
    }
    case WM_SETFOCUS: {
        // Ensure the window continues to receive keyboard input
        break;
    }
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

// Initialize serial port
void InitSerialPort() {
    hSerial = CreateFile(
        "COM3",          // Replace with your COM port
        GENERIC_READ | GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (hSerial == INVALID_HANDLE_VALUE) {
        MessageBox(NULL, "Failed to open COM port.", "Error", MB_OK | MB_ICONERROR);
        exit(1);
    }

    DCB dcbSerialParams = { 0 };
    dcbSerialParams.DCBlength = sizeof(dcbSerialParams);

    // Get current serial port settings
    if (!GetCommState(hSerial, &dcbSerialParams)) {
        MessageBox(NULL, "Failed to get COM port state.", "Error", MB_OK | MB_ICONERROR);
        CloseHandle(hSerial);
        exit(1);
    }

    // Configure serial port parameters
    dcbSerialParams.BaudRate = CBR_9600;  // Match this with your microcontroller
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity   = NOPARITY;

    // Apply the settings
    if (!SetCommState(hSerial, &dcbSerialParams)) {
        MessageBox(NULL, "Failed to set COM port state.", "Error", MB_OK | MB_ICONERROR);
        CloseHandle(hSerial);
        exit(1);
    }

    // Set timeouts
    COMMTIMEOUTS timeouts = { 0 };
    timeouts.ReadIntervalTimeout         = 50;
    timeouts.ReadTotalTimeoutConstant    = 50;
    timeouts.ReadTotalTimeoutMultiplier  = 10;
    timeouts.WriteTotalTimeoutConstant   = 50;
    timeouts.WriteTotalTimeoutMultiplier = 10;
    SetCommTimeouts(hSerial, &timeouts);
}

// Read from the serial port
DWORD WINAPI SerialThread(LPVOID lpParam) {
    HWND hwnd = (HWND)lpParam;
    char buffer[128];
    DWORD bytesRead;

    while (true) {
        if (ReadFile(hSerial, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0) {
            buffer[bytesRead] = '\0'; // Null-terminate the buffer
            std::string data(buffer);
            // Parse data and update sensor values
            ParseAndDisplayData(data);

            // Notify the main thread to update the UI
            PostMessage(hwnd, WM_UPDATE_SENSOR_LABELS, 0, 0);
        }
        Sleep(100); // Adjust the sleep duration as needed
    }
    return 0;
}

// Parse incoming data and update sensor values
void ParseAndDisplayData(const std::string& data) {
    // Expected data format: "S1:123;S2:456;S3:789;S4:101;"
    std::istringstream ss(data);
    std::string token;

    while (std::getline(ss, token, ';')) {
        if (token.find("S") != std::string::npos) {
            int sensorIndex = token[1] - '1'; // Extract sensor index (0-3)
            std::string valueStr = token.substr(3); // Extract value after 'Sx:'
            int value = std::stoi(valueStr);

            if (sensorIndex >= 0 && sensorIndex < 4) {
                // Update global sensor data
                gSensorData.sensorValues[sensorIndex] = value;
            }
        }
    }
}

// Send command to microcontroller
void SendCommand(const std::string& command) {
    DWORD bytesWritten;
    if (!WriteFile(hSerial, command.c_str(), command.length(), &bytesWritten, NULL)) {
        MessageBox(NULL, "Failed to send command.", "Error", MB_OK | MB_ICONERROR);
    }
}

// Close the serial port
void CloseSerialPort() {
    CloseHandle(hSerial);
}
