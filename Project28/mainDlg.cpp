#include "mainDlg.h"

mainDlg* mainDlg::ptr = NULL;
bool isSave = false;
bool fFitsr = true;
int numberCases = 0;
std::vector<int> timerArray;

std::vector<HANDLE> threadHandles;
struct ThreadParams {
	int timer;
	int index;
	HWND hList3;
};
ThreadParams* paramList = new ThreadParams;

mainDlg::mainDlg(void) {
	ptr = this;
	pNID = new NOTIFYICONDATA;
}

mainDlg::~mainDlg(void) {
	delete pNID;
}

BOOL mainDlg::Cls_OnInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam) {
	HINSTANCE hInst = GetModuleHandle(NULL);

	hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDB_PNG1));

	pNID->hIcon = hIcon;

	for (int i = 0; i < numberOfButtons; i++) {
		hButtons[i] = GetDlgItem(hwnd, IdsButtons[i]);
	}

	hList1 = GetDlgItem(hwnd, IDC_LIST1);
	hList2 = GetDlgItem(hwnd, IDC_LIST2);
	hList3 = GetDlgItem(hwnd, IDC_LIST3);
	hEdit1 = GetDlgItem(hwnd, IDC_EDIT1);
	hEdit2 = GetDlgItem(hwnd, IDC_EDIT2);
	
	_TCHAR buff[25] = _T("");

	

	return TRUE;
}

void mainDlg::Cls_OnClose(HWND hwnd) {
	EndDialog(hwnd, 0);
}

DWORD WINAPI Thread_timer(LPVOID lp) {
	ThreadParams* params = (ThreadParams*)lp;

	int timer = params->timer;
	int index = params->index;
	HWND hList3 = params->hList3;

	HANDLE hTimer = CreateWaitableTimer(NULL, TRUE, NULL);
	SYSTEMTIME st;

	GetLocalTime(&st);

	st.wHour = timer / 3600;
	st.wMinute = (timer % 3600) / 60;
	st.wSecond = timer % 60;

	FILETIME ft;
	SystemTimeToFileTime(&st, &ft);
	LocalFileTimeToFileTime(&ft, &ft);
	SetWaitableTimer(hTimer, (LARGE_INTEGER*)&ft, 0, NULL, NULL, FALSE);
	if (WaitForSingleObject(hTimer, INFINITE) == WAIT_OBJECT_0) {
		MessageBox(0, _T("FFFFF"), 0, 0);
		SendMessage(hList3, LB_DELETESTRING, index, 0);
		SendMessage(hList3, LB_INSERTSTRING, index, LPARAM(_T("Time is up!")));
	}

	return 0;
}

void mainDlg::Cls_OnCommand(HWND hwnd, int id, HWND hwndCtl, UINT codeNotify) {
	const int max_size = 250;

	_TCHAR buffTask[max_size] = _T("");
	_TCHAR buffTime[max_size] = _T("");

	if (id == IDC_BUTTON1) {
		GetWindowText(hEdit1, buffTask, max_size);
		GetWindowText(hEdit2, buffTime, max_size);

		if (_tcsclen(buffTask) == 0 || _tcsclen(buffTime) == 0) {
			MessageBox(hwnd, _T("Вы заполнили не все поля!"), _T("Информация"), MB_OK);
			return;
		}

		if (isTimeFormat(buffTime)) {
			MessageBox(hwnd, _T("Введите в формате: часы:минуты:секунды!"), _T("Информация"), MB_OK);
			return;
		}


		int tempTimer = FormatTime(GetArrayFormatTime(buffTime));
		timerArray.push_back(tempTimer);
		int index = isUpdate(&timerArray[0], timerArray.size());

		paramList->timer = timerArray[index];
		paramList->index = numberCases++;
		paramList->hList3 = hList3;

		if (index != -1 || fFitsr) {
			fFitsr = false;
			HANDLE hThread = CreateThread(NULL, 0, Thread_timer, paramList, 0, NULL);
			threadHandles.push_back(hThread);

			timerArray.erase(timerArray.begin() + index);
		}
		SendMessage(hList1, LB_ADDSTRING, 0, LPARAM(buffTask));
		SendMessage(hList2, LB_ADDSTRING, 0, LPARAM(buffTime));
		SendMessage(hList3, LB_ADDSTRING, 0, LPARAM(_T("Not done")));
		SetWindowText(hEdit1, _T(""));
		SetWindowText(hEdit2, _T(""));
	}
	else if (id == IDC_BUTTON2) {
		int index = SendMessage(hList1, LB_GETCURSEL, 0, 0);
		SendMessage(hList3, LB_DELETESTRING, index, 0);
		SendMessage(hList3, LB_INSERTSTRING, index, LPARAM(_T("Done")));
		updateSelected();
	}
	else if (id == IDC_LIST1 && codeNotify == LBN_SELCHANGE || id == IDC_LIST2 && codeNotify == LBN_SELCHANGE || id == IDC_LIST3 && codeNotify == LBN_SELCHANGE) {
		updateSelected();
		EnableWindow(hButtons[3], TRUE);
		EnableWindow(hButtons[2], TRUE);
		EnableWindow(hButtons[1], TRUE);
	}
	else if (id == IDC_BUTTON3) {
		int index = SendMessage(hList1, LB_GETCURSEL, 0, 0);
		SendMessage(hList1, LB_DELETESTRING, index, 0);
		SendMessage(hList2, LB_DELETESTRING, index, 0);
		SendMessage(hList3, LB_DELETESTRING, index, 0);
	}
	else if (id == IDC_BUTTON4 && !isSave) {
		int index = SendMessage(hList1, LB_GETCURSEL, 0, 0);
		SendMessage(hList1, LB_GETTEXT, index, LPARAM(buffTask));
		SendMessage(hList2, LB_GETTEXT, index, LPARAM(buffTime));
		SetWindowText(hEdit1, buffTask);
		SetWindowText(hEdit2, buffTime);
		SetWindowText(hButtons[3], _T("Save"));
		EnableWindow(hButtons[0], FALSE);		

		isSave = true;
	}
	else if (id == IDC_BUTTON4 && isSave) {
		int index = SendMessage(hList1, LB_GETCURSEL, 0, 0);
		GetWindowText(hEdit1, buffTask, max_size);
		GetWindowText(hEdit2, buffTime, max_size);

		if (_tcsclen(buffTask) == 0 || _tcsclen(buffTime) == 0) {
			MessageBox(hwnd, _T("Вы заполнили не все поля!"), _T("Информация"), MB_OK);
			return;
		}

		if (isTimeFormat(buffTime)) {
			MessageBox(hwnd, _T("Введите в формате: часы:минуты:секунды!"), _T("Информация"), MB_OK);
			return;
		}

		SendMessage(hList1, LB_DELETESTRING, index, 0);
		SendMessage(hList2, LB_DELETESTRING, index, 0);
		SendMessage(hList3, LB_DELETESTRING, index, 0);
		SendMessage(hList1, LB_INSERTSTRING, index, LPARAM(buffTask));
		SendMessage(hList2, LB_INSERTSTRING, index, LPARAM(buffTime));
		SendMessage(hList3, LB_INSERTSTRING, index, LPARAM(_T("Not done")));
		SetWindowText(hButtons[3], _T("Edit"));
		SetWindowText(hEdit1, _T(""));
		SetWindowText(hEdit2, _T(""));

		isSave = false;
		updateSelected();
		EnableWindow(hButtons[0], TRUE);

	}
	else if (id == IDC_BUTTON5) {
		EndDialog(hwnd, 0);
	}
}

bool mainDlg::isTimeFormat(_TCHAR* text) {
	const int lengthText = _tcsclen(text);
	if (lengthText >= 9 || lengthText < 8)
		return true;
	if (text[2] != _T(':') || text[5] != _T(':'))
		return true;

	_TCHAR temp[3] = _T("");

	for (int i = 0; i < lengthText; i++) {
		if (isalpha(text[i])) { 
			return true;
		}
		if (text[i] != _T(':')) {
			temp[0] = text[i];
			temp[1] = text[i + 1];
			temp[2] = _T('\0');

			int number = _tstoi(temp);

			if (i == 0 && (number > 23 || number <= 0))
				return true;
			else if (i == 3 && (number > 59 || number < 0))
				return true;
			else if (i == 6 && (number > 59 || number < 0))
				return true;
		}
	}

	return false;
}

int mainDlg::isUpdate(const int* arrayTime, int size, int cols) {
	if (size == 1)
		return 0;

	_TCHAR buff[25];
	int minVal = 0;

	for (int i = 0; i < size; i++) {
		if (arrayTime[i] < arrayTime[minVal])
			minVal = i;
	}

	if (minVal == 0)
		return minVal;

	wsprintf(buff, _T("%d"), minVal);
	MessageBox(0, buff, 0, 0);

	return minVal;
}


int mainDlg::FormatTime(int* timerArray) {	
	return timerArray[0] * 3600 + timerArray[1] * 60 + timerArray[2];
}

void mainDlg::updateSelected() {
	int index = SendMessage(hList1, LB_GETCURSEL, 0, 0);
	SendMessage(hList1, LB_SETCURSEL, index, 0);
	SendMessage(hList2, LB_SETCURSEL, index, 0);
	SendMessage(hList3, LB_SETCURSEL, index, 0);
}

void mainDlg::updateTimer(int& timer) {
	HANDLE hTimer = CreateWaitableTimer(NULL, TRUE, NULL);
	SYSTEMTIME st;

	st.wHour = timer / 3600;
	st.wMinute = (timer % 3600) / 60;
	st.wSecond = timer % 60;

	FILETIME ft;
	SystemTimeToFileTime(&st, &ft); 
	LocalFileTimeToFileTime(&ft, &ft); 
	SetWaitableTimer(hTimer, (LARGE_INTEGER*)&ft, 0, NULL, NULL, FALSE); 

	MessageBox(0, _T("DO"), 0, 0);

	if (WaitForSingleObject(hTimer, INFINITE) == WAIT_OBJECT_0) {
		MessageBox(0, _T("FFFFF"), 0, 0);
	}
}

int* mainDlg::GetArrayFormatTime(_TCHAR* text) {
	const int lengthText = _tcsclen(text);
	_TCHAR temp[3] = _T("");
	int* rez = new int[3];
	
	for (int i = 0; i < lengthText; i++) {
		if (text[i] != _T(':')) {
			temp[0] = text[i];
			temp[1] = text[i + 1];
			temp[2] = _T('\0');

			int number = _tstoi(temp);

			if (i == 0) 
				rez[0] = number;
			else if (i == 3) 
				rez[1] = number;
			else if (i == 6)
				rez[2] = number;
		}
	}

	return rez;
}

BOOL CALLBACK mainDlg::DlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
		HANDLE_MSG(hwnd, WM_CLOSE, ptr->Cls_OnClose);
		HANDLE_MSG(hwnd, WM_INITDIALOG, ptr->Cls_OnInitDialog);
		HANDLE_MSG(hwnd, WM_COMMAND, ptr->Cls_OnCommand);
	}

	return FALSE;
}