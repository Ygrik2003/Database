// lab3main1.cpp : Определяет точку входа для приложения.
//

#include <windows.h>
#include "framework.h"
#include "database.h"
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <string>

#define MAX_LOADSTRING 64
#define BUFFER_SIZE 32
#define NAME_OF_DATABASE_STUDENTS_FILE "Students.bin"
#define NAME_OF_DATABASE_MARKS_FILE "Marks.bin"

const unsigned MAX_COUNT_MARKS = 100;

HINSTANCE hInst;
WCHAR szTitle[MAX_LOADSTRING];
WCHAR szWindowClass[MAX_LOADSTRING];


ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    MenuProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    AddMarks(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    AddStudent(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    DeleteStudent(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    OptimizeDatabase(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    ClearDatabase(HWND, UINT, WPARAM, LPARAM);


bool isEqualStr(const char* firstS, const char* secondS)
{
    int i = 0;
    while (i < BUFFER_SIZE)
    {
        if (firstS[i] != secondS[i])
            return false;
    }
    return true;
}

struct Marks
{
    Marks() {}

    /*
    Marks(int n, ...)
    {
        int mark;
        va_list factor;
        va_start(factor, n);
        for (int i = 0; i < n; i++)
        {
            mark = va_arg(factor, BYTE);
            if (mark < 0 || mark > 10)
                throw std::exception("Marks are wrong");
            marks[i] = mark;
        }
        length = n;
    }
    */

    Marks(int n, BYTE *marks)
    {
        int mark;
        for (int i = 0; i < n; i++)
        {
            mark = marks[i];
            if (mark < 0 || mark > 10)
                throw std::exception("Marks are wrong");
            marks[i] = mark;
        }
        length = n;
    }

    Marks(const Marks& m)
    {
        for (unsigned i = 0; i < m.length; i++)
            marks[i] = m.marks[i];
        length = m.length;
    }

    void addMark(int mark)
    {
        if (mark < 0 || mark > 10)
            throw std::exception("Marks are wrong");
        marks[length] = mark;
        length++;
    }

    BYTE& operator[](int i)
    {
        return marks[i];
    }

    void operator=(Marks marks)
    {
        this->length = marks.length;
        for (unsigned i = 0; i < length; i++)
            this->marks[i] = marks[i];
    }


    unsigned length = 0;
    BYTE marks[MAX_COUNT_MARKS]{0};
};

struct Student
{
    Student() {};
    Student(const Student& s)
    {
        this->id = s.id;
        for (int i = 0; i < BUFFER_SIZE; i++)
        {
            this->firstName[i] = s.firstName[i];
            this->secondName[i] = s.secondName[i];
        }
        this->course = s.course;
        for (int i = 0; i < MAX_COUNT_MARKS; i++)
            this->marks[i] = s.marks[i];
        this->isDeleted = s.isDeleted;

    }
    Student(unsigned id, const char* firstName, const char* secondName, unsigned course, Marks marks)
    {
        this->id = id;
        for (int i = 0; i < BUFFER_SIZE; i++)
        {
            this->firstName[i] = firstName[i];
            this->secondName[i] = secondName[i];
        }

        if (course < 1 || course > 6)
            throw std::exception("Course is wrong");
        this->course = course;
        for (unsigned i = 0; i < marks.length; i++)
            this->marks[i] = marks[i];
    }

    unsigned id = -1;
    char firstName[BUFFER_SIZE]{ 0 };
    char secondName[BUFFER_SIZE]{ 0 };
    unsigned course = 0;
    BYTE marks[MAX_COUNT_MARKS]{ 0 };
    bool isDeleted = 0;
};

class MarksCount
{
public:
    MarksCount() {}
    MarksCount(unsigned id, unsigned count) 
    {
        this->id = id;
        this->count = count;
    }

    unsigned id = 0;
    unsigned count = 0;

};

struct StudentMarks
{
    StudentMarks() {}
    StudentMarks(Student student, int length)
    {
        this->student = student;
        marks = Marks(length, student.marks);
    }

    Student student;
    Marks marks;
};

struct Database
{
    Database()
    {
        fopen_s(&fileDatabaseStudents, NAME_OF_DATABASE_STUDENTS_FILE, "r+b");
        fopen_s(&fileDatabaseMarksCounts, NAME_OF_DATABASE_MARKS_FILE, "r+b");

        if (fileDatabaseStudents && fileDatabaseMarksCounts)
        {

            fseek(fileDatabaseMarksCounts, 0, SEEK_END);
            fseek(fileDatabaseStudents, 0, SEEK_END);

            count = ftell(fileDatabaseMarksCounts) / sizeof(MarksCount);

        }
        else
        {
            throw std::exception("FileError: Cant open files");
        }
        
    }
    Database(const Database& db)
    {
        this->fileDatabaseMarksCounts = db.fileDatabaseMarksCounts;
        this->fileDatabaseStudents = db.fileDatabaseStudents;
        this->count = db.count;
    }

    ~Database()
    {
        if (fileDatabaseStudents && fileDatabaseMarksCounts)
        {
            fclose(fileDatabaseStudents);
            fclose(fileDatabaseMarksCounts);
        }
    }

    void rewriteStudent(Student student, Marks marks)
    {
        MarksCount marksCount(student.id, marks.length);

        

        fseek(fileDatabaseStudents, student.id * sizeof(Student), SEEK_SET);
        fwrite((char*)(&student), sizeof(Student), 1, fileDatabaseStudents);

        
        fseek(fileDatabaseMarksCounts, student.id * sizeof(MarksCount), SEEK_SET);
        fwrite((char*)(&marksCount), sizeof(MarksCount), 1, fileDatabaseMarksCounts);

    }

    void addStudent(const char* firstName, const char* secondName, unsigned course, Marks marks)
    {
        if (course < 1 || course > 6)
            throw std::exception("Course is wrong");


        fseek(fileDatabaseMarksCounts, 0, SEEK_END);
        fseek(fileDatabaseStudents, 0, SEEK_END);

        Student student(count, firstName, secondName, course, marks);
        MarksCount marksCount(count, marks.length);

        fwrite((char*)(&marksCount), sizeof(MarksCount), 1, fileDatabaseMarksCounts);
        fwrite((char*)(&student), sizeof(Student), 1, fileDatabaseStudents);

        count++;
    }

    void deleteStudent(unsigned id)
    {
        if (id >= count)
            throw std::exception("ID is wrong");

        Student student;
        MarksCount marksCount;

        fseek(fileDatabaseMarksCounts, id * sizeof(MarksCount), SEEK_SET);
        
        fread(&marksCount, sizeof(MarksCount), 1, fileDatabaseMarksCounts);

        student.isDeleted = true;

        fseek(fileDatabaseStudents, id * sizeof(Student), SEEK_SET);

        fwrite((char*)(&student), sizeof(Student), 1, fileDatabaseStudents);
    }

    StudentMarks getStudentFromID(unsigned id)
    {
        if (id >= count)
            throw std::exception("ID is wrong");
        Student student;
        MarksCount marksCount;

        fseek(fileDatabaseMarksCounts, id * sizeof(MarksCount), SEEK_SET);
        fread(&marksCount, sizeof(MarksCount), 1, fileDatabaseMarksCounts);

        fseek(fileDatabaseStudents, id * sizeof(Student), SEEK_SET);
        fread(&student, sizeof(Student), 1, fileDatabaseStudents);

        return StudentMarks(student, marksCount.count);
    }

    void clearDB()
    {
        if (fileDatabaseStudents && fileDatabaseMarksCounts)
        {
            fclose(fileDatabaseStudents);
            fclose(fileDatabaseMarksCounts);
        }
        
        fopen_s(&fileDatabaseStudents, NAME_OF_DATABASE_STUDENTS_FILE, "wb");
        fopen_s(&fileDatabaseMarksCounts, NAME_OF_DATABASE_MARKS_FILE, "wb");

        if (fileDatabaseStudents && fileDatabaseMarksCounts)
        {
            fclose(fileDatabaseStudents);
            fclose(fileDatabaseMarksCounts);
        }

        fopen_s(&fileDatabaseStudents, NAME_OF_DATABASE_STUDENTS_FILE, "r+b");
        fopen_s(&fileDatabaseMarksCounts, NAME_OF_DATABASE_MARKS_FILE, "r+b");
    
        if (!fileDatabaseStudents || !fileDatabaseMarksCounts)
        {
            throw std::exception("FileError: Cant open files");
        }
            
        count = 0;
    }

    FILE* fileDatabaseStudents{};
    FILE* fileDatabaseMarksCounts{};
    unsigned count = 0;
};

Database db;



int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DATABASE, szWindowClass, MAX_LOADSTRING);

    
    MyRegisterClass(hInstance);
    
    
    if (!InitInstance(hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DATABASE));

    MSG msg;


    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int)msg.wParam;
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
    hInst = hInstance;

    HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd)
    {
        return FALSE;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = MenuProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DATABASE));
    wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_DATABASE);
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

HWND hEditLeft, hEditRight;

void updateDatabaseText()
{
    std::string leftText;
    std::string rightText;

    StudentMarks student;

    for (unsigned i = 0; i < db.count; i++)
    {
        student = db.getStudentFromID(i);
        if (!student.student.isDeleted)
        {
            char buf[BUFFER_SIZE]{0};
            
            leftText += "ID: ";
            _itoa_s(student.student.id, buf, 10);
            leftText += buf;
            leftText += '\n';

            leftText += "First name: ";
            leftText += student.student.firstName;
            leftText += '\n';

            leftText += "Second name: ";
            leftText += student.student.secondName;
            leftText += '\n';


            leftText += "Course: ";
            _itoa_s(student.student.course, buf, 10);
            leftText += buf;
            leftText += '\n';


            leftText += "Marks: ";

            for (unsigned j = 0; j < student.marks.length; j++)
            {
                _itoa_s(student.student.marks[j], buf, 10);
                leftText += buf;
                leftText += " ";
            }
            leftText += '\n';
            leftText += '\n';


            rightText += "ID: ";
            _itoa_s(student.student.id, buf, 10);
            rightText += buf;
            rightText += '\n';

            rightText += "Count marks: ";
            _itoa_s(student.marks.length, buf, 10);
            rightText += buf;
            rightText += '\n';
            rightText += '\n';


        }
    }
    SetWindowText(hEditLeft, leftText.c_str());
    SetWindowText(hEditRight, rightText.c_str());

}

void OptimizeDatabase()
{
    StudentMarks* arrStudents = new StudentMarks[db.count];
    int count = db.count;
    for (unsigned i = 0; i < count; i++)
    {
        arrStudents[i] = db.getStudentFromID(i);
    }
    db.clearDB();
    for (unsigned i = 0; i < count; i++)
    {
        if (!arrStudents[i].student.isDeleted)
            db.addStudent(arrStudents[i].student.firstName, arrStudents[i].student.secondName, arrStudents[i].student.course, arrStudents[i].marks);

    }
    updateDatabaseText();
}

LRESULT MenuProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    static RECT rectWindow;
    static Student student;
    static MarksCount marksCount;
    switch (msg)
    {
    case WM_CREATE:
        GetWindowRect(hWnd, &rectWindow);

        hEditLeft = CreateWindow("Static", NULL, 
            WS_VSCROLL | WS_CHILD | WS_VISIBLE | WS_BORDER,
            10,
            10,
            (int)(0.3 * (rectWindow.right - rectWindow.left)),
            (rectWindow.bottom - rectWindow.top - 60) - 2 * 10,
            hWnd, NULL, NULL, NULL); 
        hEditRight = CreateWindow("Static", NULL, 
            WS_VSCROLL | WS_CHILD | WS_VISIBLE | WS_BORDER,
            (int)(0.7 * (rectWindow.right - rectWindow.left)) - 2 * 10,
            10,
            (int)(0.3 * (rectWindow.right - rectWindow.left)),
            (rectWindow.bottom - rectWindow.top - 60) - 2 * 10,
            hWnd, NULL, NULL, NULL);
            updateDatabaseText();
        break;
    case WM_SIZE:
        GetWindowRect(hWnd, &rectWindow);
        
        SetWindowPos(hEditLeft, nullptr, 
            10,
            10,
            (int)(0.3 * (rectWindow.right - rectWindow.left)), 
            (rectWindow.bottom - rectWindow.top - 60) - 2 * 10, 
            NULL);

        SetWindowPos(hEditRight, nullptr,
            (int)(0.7 * (rectWindow.right - rectWindow.left)) - 2 * 10,
            10,
            (int)(0.3 * (rectWindow.right - rectWindow.left)),
            (rectWindow.bottom - rectWindow.top - 60) - 2 * 10,
            NULL);
        break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDC_ADDMARKS:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ADDMARKS), hWnd, AddMarks);
            break;
        case IDC_ADDSTUDENT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ADDSTUDENT), hWnd, AddStudent);
            break; 
        case IDC_DELETESTUDENT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_DELETESTUDENT), hWnd, DeleteStudent);
            break;
        case IDC_CLEARDATABASE:
            db.clearDB();
            updateDatabaseText();
            break;
        case IDC_OPTIMIZEDATABASE:
            OptimizeDatabase();

            break;
        case IDC_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
        }
    }
    break;
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}


INT_PTR CALLBACK About(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (msg)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


INT_PTR CALLBACK AddMarks(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    static HWND hEditID, hEditMarks;
    switch (msg)
    {
    case WM_INITDIALOG:
        CreateWindow("STATIC", "ID", WS_CHILD | WS_VISIBLE, 10, 10, 140, 20, hDlg, NULL, NULL, NULL);
        hEditID = CreateWindow("Edit", NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            160,
            10,
            100,
            20,
            hDlg, NULL, NULL, NULL);
        CreateWindow("STATIC", "Marks (sep = ' ')", WS_CHILD | WS_VISIBLE, 10, 40, 140, 20, hDlg, NULL, NULL, NULL);
        hEditMarks = CreateWindow("Edit", NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            160,
            40,
            100,
            20,
            hDlg, NULL, NULL, NULL);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            try 
            {
                char IDStudent[BUFFER_SIZE];
                char marks[BUFFER_SIZE];

                char buf[BUFFER_SIZE];

                GetWindowText(hEditID, IDStudent, BUFFER_SIZE);
                GetWindowText(hEditMarks, marks, BUFFER_SIZE);

                
                try
                {
                    if (IDStudent[0] == 0)
                        throw std::exception("ID is wrong");
                    StudentMarks studentMarks = db.getStudentFromID(atoi(IDStudent));

                    int tempLength = studentMarks.marks.length;

                    int i = 0, j = 0;
                    while (marks[i] != 0)
                    {
                        if (marks[i] != ' ')
                        {
                            buf[j] = marks[i];
                            j++;
                        }
                        else
                        {
                            buf[j] = 0;
                            studentMarks.marks.addMark(atoi(buf));
                            j = 0;
                        }
                        i++;
                    }
                    if (j != 0)
                    {
                        buf[j] = 0;
                        studentMarks.marks.addMark(atoi(buf));
                    }

                    for (int k = tempLength; k < studentMarks.marks.length; k++)
                        studentMarks.student.marks[k] = studentMarks.marks.marks[k];
            
                    db.rewriteStudent(studentMarks.student, studentMarks.marks);

                    updateDatabaseText();

                    EndDialog(hDlg, LOWORD(wParam));
                }
                catch (std::exception e)
                {
                    MessageBox(
                        NULL,
                        e.what(),
                        "Error",
                        MB_ICONEXCLAMATION | MB_OK
                    );
                }
            }
            catch (std::exception& e)
            {
                MessageBox(
                    NULL,
                    e.what(),
                    "Error",
                    MB_ICONEXCLAMATION | MB_OK
                );
            }
            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    case WM_CLOSE:
        EndDialog(hDlg, LOWORD(wParam));
        return (INT_PTR)TRUE;
    }

    return (INT_PTR)FALSE;
}


INT_PTR CALLBACK AddStudent(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    static HWND hEditFirstName, hEditSecondName, hEditCourse;
    switch (msg)
    {
    case WM_INITDIALOG:
        CreateWindow("STATIC", "First name student", WS_CHILD | WS_VISIBLE, 10, 10, 100, 20, hDlg, NULL, NULL, NULL);
        hEditFirstName = CreateWindow("Edit", NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            120,
            10,
            100,
            20,
            hDlg, NULL, NULL, NULL);
        CreateWindow("STATIC", "Second name student", WS_CHILD | WS_VISIBLE, 10, 40, 100, 20, hDlg, NULL, NULL, NULL);
        hEditSecondName = CreateWindow("Edit", NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            120,
            40,
            100,
            20,
            hDlg, NULL, NULL, NULL);
        CreateWindow("STATIC", "Course student", WS_CHILD | WS_VISIBLE, 10, 70, 100, 20, hDlg, NULL, NULL, NULL);
        hEditCourse = CreateWindow("Edit", NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            120,
            70,
            100,
            20,
            hDlg, NULL, NULL, NULL);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            char firstName[BUFFER_SIZE];
            char secondName[BUFFER_SIZE];
            char course[BUFFER_SIZE];

            GetWindowText(hEditFirstName, firstName, BUFFER_SIZE);
            GetWindowText(hEditSecondName, secondName, BUFFER_SIZE);
            GetWindowText(hEditCourse, course, BUFFER_SIZE);

            if (firstName[0] == 0 || secondName[0] == 0)
            {
                MessageBox(
                    NULL,
                    "Name is wrong",
                    "Error",
                    MB_ICONEXCLAMATION | MB_OK
                );
                return (INT_PTR)TRUE;
            }

            try {
                db.addStudent(firstName, secondName, atoi(course), Marks());
                EndDialog(hDlg, LOWORD(wParam));
            }
            catch (std::exception& e)
            {
                MessageBox(
                    NULL,
                    e.what(),
                    "Error",
                    MB_ICONEXCLAMATION | MB_OK
                );
            }


            
            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    case WM_CLOSE:
        EndDialog(hDlg, LOWORD(wParam));
        return (INT_PTR)TRUE;
    }
    updateDatabaseText();

    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK DeleteStudent(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    static HWND hEditID;
    switch (msg)
    {
    case WM_INITDIALOG:
        CreateWindow("STATIC", "ID student", WS_CHILD | WS_VISIBLE, 10, 10, 100, 20, hDlg, NULL, NULL, NULL);
        hEditID = CreateWindow("Edit", NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            120,
            10,
            100,
            20,
            hDlg, NULL, NULL, NULL);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            char IDStudent[BUFFER_SIZE];


            GetWindowText(hEditID, IDStudent, BUFFER_SIZE);

            try 
            {
                db.deleteStudent(atoi(IDStudent));
                EndDialog(hDlg, LOWORD(wParam));
            }
            catch (std::exception e)
            {
                MessageBox(
                    NULL,
                    e.what(),
                    "Error",
                    MB_ICONEXCLAMATION | MB_OK
                );
            }


            return (INT_PTR)TRUE;
        }
        if (LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    case WM_CLOSE:
        EndDialog(hDlg, LOWORD(wParam));
        return (INT_PTR)TRUE;
    }
    updateDatabaseText();

    return (INT_PTR)FALSE;
}