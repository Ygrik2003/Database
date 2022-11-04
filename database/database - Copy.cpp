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
#define OFFSET 100;

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
    Marks(int n, ...)
    {
        va_list factor;
        va_start(factor, n);
        if (marks != NULL) delete[] marks;
        marks = new BYTE[n];
        for (int i = 0; i < n; i++)
            marks[i] = va_arg(factor, BYTE);
        length = n;
    }

    Marks(const Marks& m)
    {
        if (this->marks != NULL) delete[] this->marks;
        marks = new BYTE[m.length];
        for (unsigned i = 0; i < m.length; i++)
            marks[i] = m.marks[i];
        length = m.length;
    }

    ~Marks()
    {
        if (marks != NULL) delete[] marks;
    }

    void addMark(int mark)
    {
        BYTE* temp = NULL;
        if (marks != NULL)
        {
            temp = marks;
            marks = new BYTE[length + 1];
            for (unsigned i = 0; i < length; i++)
                marks[i] = temp[i];
            delete[] temp;
        }
        else
        {
            length = 0;
            marks = new BYTE[1];
        }
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
        if (this->marks != NULL) delete[] this->marks;
        this->marks = new BYTE[length];
        for (unsigned i = 0; i < length; i++)
            this->marks[i] = marks[i];
    }


    unsigned length = 0;
    BYTE* marks = NULL;
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
        this->marks = s.marks;
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
        this->course = course;
        this->marks = new BYTE[marks.length];
        for (unsigned i = 0; i < marks.length; i++)
            this->marks[i] = marks[i];
    }

    unsigned id = -1;
    char firstName[BUFFER_SIZE]{ 0 };
    char secondName[BUFFER_SIZE]{ 0 };
    unsigned course = 0;
    BYTE *marks = NULL;
    bool isDeleted = 0;
};

class MarksCount
{
public:
    MarksCount() {}
    MarksCount(unsigned id, unsigned count);


    unsigned id = 0;
    unsigned count = 0;
    unsigned offset = 0;

};

struct StudentMarks
{
    StudentMarks() {}
    StudentMarks(Student student, int length, int offset)
    {
        this->student = student;
        marks = Marks(length, student.marks);
        this->offset = offset;
    }

    Student student;
    Marks marks;
    int offset = 0;
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
        int offset = 0;
        
            
        MarksCount marksCount(student.id, marks.length);

        
        offset = marksCount.offset;

        fseek(fileDatabaseStudents, student.id * sizeof(Student) + offset, SEEK_SET);
        fwrite((char*)(&student), sizeof(Student), 1, fileDatabaseStudents);

        fwrite(student.marks, sizeof(BYTE), marksCount.count, fileDatabaseStudents);
        
        fseek(fileDatabaseMarksCounts, student.id * sizeof(MarksCount), SEEK_SET);
        fwrite((char*)(&marksCount), sizeof(MarksCount), 1, fileDatabaseMarksCounts);

    }

    void addStudent(const char* firstName, const char* secondName, unsigned course, Marks marks)
    {

        fseek(fileDatabaseMarksCounts, 0, SEEK_END);
        fseek(fileDatabaseStudents, 0, SEEK_END);

        Student student(count, firstName, secondName, course, marks);
        MarksCount marksCount(count, marks.length);

        fwrite((char*)(&marksCount), sizeof(MarksCount), 1, fileDatabaseMarksCounts);
        fwrite((char*)(&student), sizeof(Student), 1, fileDatabaseStudents);

        fwrite(student.marks, sizeof(BYTE), marksCount.count, fileDatabaseStudents);




        count++;
    }

    void deleteStudent(unsigned id)
    {
        Student student;
        MarksCount marksCount;

        fseek(fileDatabaseMarksCounts, id * sizeof(MarksCount), SEEK_SET);
        
        fread(&marksCount, sizeof(MarksCount), 1, fileDatabaseMarksCounts);

        student.isDeleted = true;

        fseek(fileDatabaseStudents, id * sizeof(Student) + marksCount.offset, SEEK_SET);

        fwrite((char*)(&student), sizeof(Student), 1, fileDatabaseStudents);
    }

    StudentMarks getStudentFromID(unsigned id)
    {
        Student student;
        MarksCount marksCount;

        fseek(fileDatabaseMarksCounts, id * sizeof(MarksCount), SEEK_SET);
        fread(&marksCount, sizeof(MarksCount), 1, fileDatabaseMarksCounts);

        fseek(fileDatabaseStudents, id * sizeof(Student) + marksCount.offset, SEEK_SET);
        fread(&student, sizeof(Student), 1, fileDatabaseStudents);

        student.marks = new BYTE[marksCount.count];
            
        fread(&student.marks, sizeof(BYTE), marksCount.count, fileDatabaseStudents);


        return StudentMarks(student, marksCount.count, marksCount.offset);
    }
    /*
    StudentMarks getStudentFromName(const char* firstName, const char* secondName)
    {
        Student student;
        MarksCount marksCount;

        
        char firstNameF[BUFFER_SIZE], secondNameF[BUFFER_SIZE];
        for (unsigned id = 0; id < count; id++)
        {

            fseek(fileDatabaseStudents, id * sizeof(Student), SEEK_SET);
            fseek(fileDatabaseStudents, 4, SEEK_CUR);
            fread(&firstNameF, 1, BUFFER_SIZE, fileDatabaseStudents);
            fseek(fileDatabaseStudents, BUFFER_SIZE, SEEK_CUR);
            fread(&secondNameF, 1, BUFFER_SIZE, fileDatabaseStudents);

            if (!isEqualStr(firstName, firstNameF) && !isEqualStr(secondName, secondNameF))
            {
                fseek(fileDatabaseStudents, id * sizeof(Student), SEEK_SET);
                fseek(fileDatabaseMarksCounts, id * sizeof(MarksCount), SEEK_SET);
                break;
            }
        }

        fread(&marksCount, sizeof(MarksCount), 1, fileDatabaseMarksCounts);
        fread(&student, sizeof(Student), 1, fileDatabaseStudents);

        BYTE* marks = new BYTE[marksCount.count];

        for (unsigned i = 0; i < marksCount.count; i++)
            fread(&marks, sizeof(BYTE), marksCount.count, fileDatabaseStudents);

        student.marks = marks;

        return StudentMarks(student, marksCount.count, marksCount.offset);
    }
    */
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

MarksCount::MarksCount(unsigned id, unsigned count)
{
    this->id = id;
    this->count = count;
    StudentMarks student = db.getStudentFromID(id - 1);
    if (!id)
        offset = 0;
    else
        offset = student.offset + student.marks.length;
    offset = OFFSET;
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_DATABASE, szWindowClass, MAX_LOADSTRING);

    db.clearDB();

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
            leftText += buf;//
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
    for (unsigned i = 0; i < db.count; i++)
    {
        arrStudents[i] = db.getStudentFromID(i);
    }
    db.clearDB();
    for (unsigned i = 0; i < db.count; i++)
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
        CreateWindow("STATIC", "ID", WS_CHILD | WS_VISIBLE, 10, 10, 100, 20, hDlg, NULL, NULL, NULL);
        hEditID = CreateWindow("Edit", NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            120,
            10,
            100,
            20,
            hDlg, NULL, NULL, NULL);
        CreateWindow("STATIC", "Marks (sep = ' ')", WS_CHILD | WS_VISIBLE, 10, 40, 100, 20, hDlg, NULL, NULL, NULL);
        hEditMarks = CreateWindow("Edit", NULL,
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            120,
            40,
            100,
            20,
            hDlg, NULL, NULL, NULL);
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK)
        {
            char IDStudent[BUFFER_SIZE];
            char marks[BUFFER_SIZE];

            char buf[BUFFER_SIZE];

            GetWindowText(hEditID, IDStudent, BUFFER_SIZE);
            GetWindowText(hEditMarks, marks, BUFFER_SIZE);

            StudentMarks studentMarks = db.getStudentFromID(atoi(IDStudent));

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

            studentMarks.student.marks = studentMarks.marks.marks;
            
            db.rewriteStudent(studentMarks.student, studentMarks.marks);

            updateDatabaseText();

            EndDialog(hDlg, LOWORD(wParam));
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

            db.addStudent(firstName, secondName, atoi(course), Marks());


            EndDialog(hDlg, LOWORD(wParam));
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

            db.deleteStudent(atoi(IDStudent));


            EndDialog(hDlg, LOWORD(wParam));
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