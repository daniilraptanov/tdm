#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <CommCtrl.h>
#include <algorithm>

// Global variables
HWND hNameInput;
HWND hDateInput;
HWND hTaskList;
HWND hAddButton;
HWND hDeleteButton;
HWND hChangeStatusButton;
HWND hSortByNameButton;
HWND hSortByDateButton;

bool sortByNameAscending = true;
bool sortByDateAscending = true;

std::vector<std::string> tasks;
std::vector<std::string> dates;
std::vector<bool> completed;

// Function prototypes
void LoadTasks();
void SaveTasks();
void UpdateTaskList(bool changeSort);
void AddTask();
void DeleteTask();
void ChangeStatus();
void SortTasksByDate(bool ascending);
void SortTasksByName(bool ascending);

// Window procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        // Create name input
        hNameInput = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 10, 10, 280, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);
        // Create date input
        hDateInput = CreateWindowEx(0, DATETIMEPICK_CLASS, "", WS_BORDER | WS_CHILD | WS_VISIBLE | DTS_SHORTDATEFORMAT, 300, 10, 280, 20, hwnd, NULL, GetModuleHandle(NULL), NULL);

        // Create task list
        hTaskList = CreateWindowEx(WS_EX_CLIENTEDGE, "LISTBOX", "", WS_CHILD | WS_VISIBLE | LBS_STANDARD | WS_VSCROLL | LBS_NOTIFY, 10, 40, 580, 700, hwnd, NULL, GetModuleHandle(NULL), NULL);

        // Create buttons
        hAddButton = CreateWindowEx(0, "BUTTON", "Add", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 750, 190, 30, hwnd, (HMENU)1, NULL, NULL);
        hDeleteButton = CreateWindowEx(0, "BUTTON", "Delete", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 205, 775, 190, 30, hwnd, (HMENU)2, NULL, NULL);
        hChangeStatusButton = CreateWindowEx(0, "BUTTON", "Change Status", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 400, 750, 190, 30, hwnd, (HMENU)3, NULL, NULL);

        // Create sort buttons
        hSortByNameButton = CreateWindowEx(0, "BUTTON", "Sort by Name", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 10, 800, 190, 30, hwnd, (HMENU)4, NULL, NULL);
        hSortByDateButton = CreateWindowEx(0, "BUTTON", "Sort by Date", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, 400, 800, 190, 30, hwnd, (HMENU)5, NULL, NULL);

        // Load tasks from file
        LoadTasks();

        // Update task list
        UpdateTaskList(false);
    }
    break;

    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case 1:
            AddTask();
            break;
        case 2:
            DeleteTask();
            break;
        case 3:
            ChangeStatus();
            break;
        case 4:
            sortByNameAscending = !sortByNameAscending;
            SortTasksByName(sortByNameAscending);
            break;
        case 5:
            sortByDateAscending = !sortByDateAscending;
            SortTasksByDate(sortByDateAscending);
            break;
        default:
            break;
        }
    }
    break;

    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

// Main function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    // Register window class
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = 0;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDC_ARROW);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = "ToDoApp";
    wc.hIconSm = LoadIcon(NULL, IDC_ARROW);

    if (!RegisterClassEx(&wc))
    {
        MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Create window
    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        "ToDoApp",
        "ToDo Application",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        620,
        900,
        NULL, NULL, hInstance, NULL);

    if (hwnd == NULL)
    {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}

// Load tasks from file
void LoadTasks()
{
    std::ifstream file("tasks.txt");
    if (file.is_open())
    {
        std::string line;
        while (std::getline(file, line))
        {
            std::stringstream ss(line);
            std::string task;
            std::string date;
            std::string status;
            std::getline(ss, task, '|');
            std::getline(ss, date, '|');
            std::getline(ss, status, '|');
            tasks.push_back(task);
            dates.push_back(date);
            completed.push_back(status == "1");
        }
        file.close();
    }
}

// Save tasks to file
void SaveTasks()
{
    std::ofstream file("tasks.txt");
    if (file.is_open())
    {
        for (size_t i = 0; i < tasks.size(); i++)
        {
            file << tasks[i] << "|" << dates[i] << "|" << completed[i] << "\n";
        }
        file.close();
    }
}

// Update task list
void UpdateTaskList(bool changeSorting)
{
    // Clear task list
    SendMessage(hTaskList, LB_RESETCONTENT, 0, 0);

    // Add tasks to task list
    for (size_t i = 0; i < tasks.size(); i++)
    {
        std::string task = tasks[i];
        std::string date = dates[i];
        bool isCompleted = completed[i];

        // Check if task is overdue
        SYSTEMTIME st;
        GetLocalTime(&st);
        int year = st.wYear;
        int month = st.wMonth;
        int day = st.wDay;
        std::stringstream ss(date);
        std::string token;
        std::getline(ss, token, '/');
        int taskYear = std::stoi(token);
        std::getline(ss, token, '/');
        int taskMonth = std::stoi(token);
        std::getline(ss, token, '/');
        int taskDay = std::stoi(token);
        bool isOverdue = false;
        if (taskYear < year)
            isOverdue = true;
        else if (taskYear == year && taskMonth < month)
            isOverdue = true;
        else if (taskYear == year && taskMonth == month && taskDay < day)
            isOverdue = true;

        // Format task string
        date += " " + task + " ";
        if (isOverdue)
            date += " - overdue";
        if (isCompleted)
            date += " - completed";
        else
            date += " - not completed";

        // Add task to list
        SendMessage(hTaskList, !changeSorting ? LB_ADDSTRING : LB_INSERTSTRING, 0, (LPARAM)date.c_str());
    }
}

// Add task
void AddTask()
{
    // Get name input
    char name[256];
    GetWindowText(hNameInput, name, sizeof(name));

    // Get date input
    char date[256];
    SYSTEMTIME st;
    DateTime_GetSystemtime(hDateInput, &st);
    sprintf_s(date, "%d/%d/%d", st.wYear, st.wMonth, st.wDay);

    // Check if input fields are not empty
    if (strlen(name) > 0 && strlen(date) > 0)
    {
        // Add new task
        tasks.push_back(name);
        dates.push_back(date);
        completed.push_back(false);

        // Save tasks to file
        SaveTasks();

        // Update task list
        UpdateTaskList(false);

        // Clear input fields
        SetWindowText(hNameInput, "");
        DateTime_SetSystemtime(hDateInput, GDT_NONE, NULL);
    }
}

// Delete task
void DeleteTask()
{
    // Get selected index
    int index = SendMessage(hTaskList, LB_GETCURSEL, 0, 0);

    // Check if an item is selected
    if (index != LB_ERR)
    {
        // Delete selected task
        tasks.erase(tasks.begin() + index);
        dates.erase(dates.begin() + index);
        completed.erase(completed.begin() + index);

        // Save tasks to file
        SaveTasks();

        // Update task list
        UpdateTaskList(false);
    }
}

// Change status of selected task
void ChangeStatus()
{
    // Get selected index
    int index = SendMessage(hTaskList, LB_GETCURSEL, 0, 0);

    // Check if an item is selected
    if (index != LB_ERR)
    {
        // Change status of selected task
        completed[index] = !completed[index];

        // Save tasks to file
        SaveTasks();

        // Update task list
        UpdateTaskList(false);
    }
}

// Sort tasks by name length
void SortTasksByName(bool ascending)
{
    SendMessage(hSortByNameButton, BM_SETSTATE, !sortByNameAscending, 0); // Set new state of the sort button

    // Sort tasks
    for (size_t i = 0; i < tasks.size(); i++)
    {
        for (size_t j = i + 1; j < tasks.size(); j++)
        {
            if (ascending ? tasks[i] > tasks[j] : tasks[i] < tasks[j])
            {
                std::swap(tasks[i], tasks[j]);
                std::swap(dates[i], dates[j]);
                bool temp = completed[i];
                completed[i] = completed[j];
                completed[j] = temp;
            }
        }
    }

    // Save tasks to file
    SaveTasks();

    // Update task list
    UpdateTaskList(true);
}

// Sort tasks by date
void SortTasksByDate(bool ascending)
{
    SendMessage(hSortByDateButton, BM_SETSTATE, !sortByDateAscending, 0); // Set new state of the sort button

    // Sort tasks
    for (size_t i = 0; i < dates.size(); i++)
    {
        for (size_t j = i + 1; j < dates.size(); j++)
        {
            std::stringstream ss1(dates[i]);
            std::string token1;
            std::getline(ss1, token1, '/');
            int year1 = std::stoi(token1);
            std::getline(ss1, token1, '/');
            int month1 = std::stoi(token1);
            std::getline(ss1, token1, '/');
            int day1 = std::stoi(token1);

            std::stringstream ss2(dates[j]);
            std::string token2;
            std::getline(ss2, token2, '/');
            int year2 = std::stoi(token2);
            std::getline(ss2, token2, '/');
            int month2 = std::stoi(token2);
            std::getline(ss2, token2, '/');
            int day2 = std::stoi(token2);

            if (ascending ? year1 > year2 || (year1 == year2 && month1 > month2) || (year1 == year2 && month1 == month2 && day1 > day2) : year1 < year2 || (year1 == year2 && month1 < month2) || (year1 == year2 && month1 == month2 && day1 < day2))
            {
                std::swap(tasks[i], tasks[j]);
                std::swap(dates[i], dates[j]);
                bool temp = completed[i];
                completed[i] = completed[j];
                completed[j] = temp;
            }
        }
    }

    // Save tasks to file
    SaveTasks();

    // Update task list
    UpdateTaskList(true);
}

