
// FileSearchDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "FileSearch.h"
#include "FileSearchDlg.h"
#include "afxdialogex.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CFileSearchDlg 对话框

struct operArgv
{
	CFileSearchDlg *dialog;
	char volumeName;
};

operArgv in[26];

UINT searchTread(LPVOID volume)
{
	operArgv *oper = (operArgv*)volume;
	CFileSearchDlg *dlg = oper->dialog;
	dlg->isSearch[oper->volumeName - 'A'] = true;
	dlg->outLineControl.SetWindowTextW(TEXT("正在搜索......"));
	dlg->fileSearch[oper->volumeName - 'A'].searchAllFile();
	dlg->isSearch[oper->volumeName - 'A'] = false;
	//搜索完成操作
	if (!dlg->isInSearch())
	{
		dlg->outLineControl.SetWindowTextW(TEXT("搜索完成"));
		//搜索完成即显示
		dlg->OnEnChangeEdit();
	}
	return 0;
}

UINT findThread(LPVOID volume)
{
	operArgv *oper = (operArgv*)volume;
	CFileSearchDlg *dlg = oper->dialog;
	dlg->isFind[oper->volumeName - 'A'] = true;
	dlg->outLineControl.SetWindowTextW(TEXT("正在查找......"));
	CString str = dlg->name;
	dlg->fileSearch[oper->volumeName - 'A'].resetFind();//重置查找
	std::vector<FindResult> &vec = dlg->fileSearch[oper->volumeName - 'A'].findFileFromName(str);
	dlg->isFind[oper->volumeName - 'A'] = false;
	if (!dlg->isInFind())
	{
		dlg->outLineControl.SetWindowTextW(TEXT("正在整理......"));
		unsigned long long len = 0;
		for (int i = 0; i < dlg->nameOfDisks.size(); i++)
		{
			std::vector<FindResult> &vec = dlg->fileSearch[dlg->nameOfDisks[i] - 'A'].getFindResult();
			//第二次筛选
			int j = 0;
			if (dlg->ifSize || dlg->ifKind)
			{
				DWORD fileSize;
				bool ifRemove = false;
				while (j < vec.size())
				{
					FileInfo info = dlg->fileSearch[dlg->nameOfDisks[i] - 'A'].getFileInfo(vec[j]);
					fileSize = dlg->getFileSizeFromPath(info.path);
					ifRemove = false;
					//size <= >= <> < > =
					if (dlg->ifSize)
					{
						switch (dlg->sizeKind)
						{
						case 0:
							if (!(fileSize <= dlg->size))
								ifRemove = true;
							break;
						case 1:
							if (!(fileSize >= dlg->size))
								ifRemove = true;
							break;
						case 2:
							if (!(fileSize != dlg->size))
								ifRemove = true;
							break;
						case 3:
							if (!(fileSize < dlg->size))
								ifRemove = true;
							break;
						case 4:
							if (!(fileSize > dlg->size))
								ifRemove = true;
							break;
						case 5:
							if (!(fileSize == dlg->size))
								ifRemove = true;
							break;
						}
					}
					//kind
					if (!ifRemove &&  dlg->ifKind)
					{
						CString subStr = info.fileName.Right(dlg->kind.GetLength());
						if (subStr != dlg->kind)
							ifRemove = true;
					}
					if (ifRemove && vec.size()>j)
					{
						vec.erase(vec.begin() + j);
					}
					else j++;
				}
			}

			len += vec.size();
		}
		dlg->ifSize = false;
		dlg->ifKind = false;
		CString out;
		out.Format(TEXT("完成,共:%d项"),len);
		dlg->outLineControl.SetWindowTextW(out);
		dlg->listControl.SetItemCount((int)len);
		dlg->PostMessageW(WM_FIND_FINISH, NULL, NULL);
	}
	return 0;
}

long CFileSearchDlg::getFileSizeFromPath(const wchar_t * filename)//CString转换必须const 否则var to var!
{
	GET_FILEEX_INFO_LEVELS levelId = GET_FILEEX_INFO_LEVELS(0);
	WIN32_FILE_ATTRIBUTE_DATA fileInformation;
	//获取文件属性
	BOOL fileGet = GetFileAttributesEx(filename, levelId, &fileInformation);
	if (fileGet)
	{
		unsigned long fileSizeLow = fileInformation.nFileSizeLow;
		unsigned long fileSizeHigh = fileInformation.nFileSizeHigh;
		ULARGE_INTEGER sizeOfFile;
		sizeOfFile.HighPart = fileSizeHigh;
		sizeOfFile.LowPart = fileSizeLow;
		return sizeOfFile.LowPart;
	}
	return 0;
}

bool CFileSearchDlg::isInSearch()
{
	bool isInSearch = false;
	for (int i = 0; i < nameOfDisks.size(); i++)
	{
		if (isSearch[nameOfDisks[i] - 'A'])
		{
			isInSearch = true;
			break;
		}
	}
	return isInSearch;
}

bool CFileSearchDlg::isInFind()
{
	bool isInFind = false;
	for (int i = 0; i < nameOfDisks.size(); i++)
	{
		if (isFind[nameOfDisks[i] - 'A'])
		{
			isInFind = true;
			break;
		}
	}
	return isInFind;
}

void CFileSearchDlg::countNTFSDisk()
{
	char diskName[] = ("A:\\");
	for (int i = 0; i < 26; i++)	//固定26毕竟字母只有26个
	{
		diskName[0] = 'A' + i;
		char szVolumeNameBuf[MAX_PATH] = { 0 };
		DWORD dwVolumeSerialNum;
		DWORD dwMaxComponentLength;
		DWORD dwSysFlags;
		char szFileSystemBuf[MAX_PATH] = { 0 };
		DWORD dwFileSystemBuf = MAX_PATH;
		GetVolumeInformationA(diskName,	//根目录名称
			szVolumeNameBuf,			//卷名称
			MAX_PATH,					//卷名称长度
			&dwVolumeSerialNum,			//磁盘序列号
			&dwMaxComponentLength,		//磁盘最大支持的文件名称长度
			&dwSysFlags,				//磁盘属性
			szFileSystemBuf,			//磁盘类型
			MAX_PATH					//磁盘类型长度
		);
		if (!memcmp("NTFS", szFileSystemBuf, sizeof("NTFS")))
		{
			//记录
			nameOfDisks.push_back('A' + i);
		}
	}
}


CFileSearchDlg::CFileSearchDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_FILESEARCH_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFileSearchDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT, editControl);
	DDX_Control(pDX, IDC_LIST, listControl);
	DDX_Control(pDX, IDC_OUTPUTLINE, outLineControl);
}

BEGIN_MESSAGE_MAP(CFileSearchDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_EN_CHANGE(IDC_EDIT, &CFileSearchDlg::OnEnChangeEdit)
	ON_WM_CLOSE()
ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST, &CFileSearchDlg::OnLvnGetdispinfoList)
ON_MESSAGE(WM_FILEMODIFY, &CFileSearchDlg::OnFilemodify)
ON_NOTIFY(NM_RCLICK, IDC_LIST, &CFileSearchDlg::OnNMRClickList)
ON_COMMAND(ID_RCLICKLIST_OPEN, &CFileSearchDlg::OnRclicklistOpen)
ON_COMMAND(ID_RCLICKLIST_COPY_FILENAME, &CFileSearchDlg::OnRclicklistCopyFilename)
ON_COMMAND(ID_RCLICKLIST_COPY_PATH, &CFileSearchDlg::OnRclicklistCopyPath)
ON_BN_CLICKED(IDC_BUTTON3, &CFileSearchDlg::OnBnClickedButton3)
ON_BN_CLICKED(IDC_BUTTON2, &CFileSearchDlg::OnBnClickedButton2)
ON_WM_SYSCOMMAND()
ON_MESSAGE(WM_SHOWWND, &CFileSearchDlg::OnShowwnd)
ON_COMMAND(ID_ICON_INSTRUCT, &CFileSearchDlg::OnIconInstruct)
ON_COMMAND(ID_ICON_ABOUT, &CFileSearchDlg::OnIconAbout)
ON_COMMAND(ID_ICON_EXIT, &CFileSearchDlg::OnIconExit)
ON_MESSAGE(WM_FIND_FINISH, &CFileSearchDlg::OnFindFinish)
END_MESSAGE_MAP()


// CFileSearchDlg 消息处理程序

BOOL CFileSearchDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	//监听函数
	SHChangeNotifyEntry pshcne = { 0 };
	pshcne.pidl = NULL;
	pshcne.fRecursive = TRUE;//让通知消息在目录树上递归
	fileNotifyId = SHChangeNotifyRegister(m_hWnd, SHCNRF_ShellLevel, SHCNE_ALLEVENTS, WM_FILEMODIFY, 1, &pshcne);
	//搜索磁盘
	countNTFSDisk();
	std::vector<char> list;
	for (int i = 0; i < nameOfDisks.size(); i++)
	{
		list.clear();
		list.push_back(nameOfDisks[i]);
		in[nameOfDisks[i] - 'A'].dialog = this;
		in[nameOfDisks[i] - 'A'].volumeName = nameOfDisks[i];
		fileSearch[nameOfDisks[i] - 'A'].setNTFSDisk(list);
		searchThreadPtr[nameOfDisks[i] - 'A'] = AfxBeginThread(searchTread, &in[nameOfDisks[i] - 'A']);
	}
	//列表框设置
	listControl.SetExtendedStyle(listControl.GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
	CRect rect;
	listControl.GetWindowRect(&rect);
	listControl.InsertColumn(0, TEXT("文件名"), LVCFMT_LEFT, rect.Width()*2 / 7);
	listControl.InsertColumn(1, TEXT("大小"), LVCFMT_LEFT, rect.Width() * 1 / 7);
	listControl.InsertColumn(2, TEXT("路径"), LVCFMT_LEFT, rect.Width() * 4 / 7);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CFileSearchDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CFileSearchDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



BOOL CFileSearchDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类

	//屏蔽退出
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_ESCAPE)
		return TRUE;
	if (pMsg->message == WM_KEYDOWN && pMsg->wParam == VK_RETURN && pMsg->wParam)
		return TRUE;
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CFileSearchDlg::OnEnChangeEdit()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。
	ifName = ifSize = ifKind = false;
	bool setName = false;
	bool setSize = false;
	bool setKind = false;
	editControl.GetWindowText(name);
	CStringArray strArray;
	CString sizeStr[] = { CString("size<="),CString("size>="),CString("size<>"),CString("size<"),CString("size>"),CString("size=") };
	splitCString(name, CString("|"), strArray);
	for (int i = 0; i < strArray.GetSize(); i++)
	{
		//size <= >= <> < > =
		CString lowStr = strArray[i].MakeLower();
		if (!setSize)
		{
			for (int j = 0; j < 6; j++)
			{
				if (sizeStr[j] == lowStr.Left(sizeStr[j].GetLength()))
				{
					sizeKind = j;
					ifSize = true;
					break;
				}
			}
			if (ifSize)
			{
				ULONGLONG dw = 0;
				//不使用默认单位
				if (lowStr.Right(1) == CString("b"))
					if (lowStr.Right(2).Left(1) == CString("g"))
						dw = 1024 * 1024 * 1024;
					else if (lowStr.Right(2).Left(1) == CString("m"))
						dw = 1024 * 1024;
					else if (lowStr.Right(2).Left(1) == CString("k"))
						dw = 1024;
					else//b
						dw = 1;
				else if (lowStr.Right(1) == CString("g"))
					dw = 1024 * 1024 * 1024;
				else if (lowStr.Right(1) == CString("m"))
					dw = 1024 * 1024;
				else if (lowStr.Right(1) == CString("k"))
					dw = 1024;
				//单位字符长度
				int dwLen = 0;
				if (dw == 1)
					dwLen = 1;
				else if (dw > 0)
				{
					if (lowStr.Right(1) == CString("b"))
						dwLen = 2;
					else dwLen = 1;
				}
				//默认单位
				if (dw == 0)
					dw = 1;
				//获取数字
				CString num = lowStr.Right(lowStr.GetLength() - sizeStr[sizeKind].GetLength());
				num = num.Left(num.GetLength() - dwLen);
				size = _ttoi64(num)*dw;
				setSize = true;
				continue;
			}
		}
		//kind
		if (!setKind && !ifKind && lowStr.Left(CString("type:").GetLength()) == CString("type:"))
		{
			kind = lowStr.Right(lowStr.GetLength() - CString("type:").GetLength());
			kind = CString(".") + kind;
			ifKind = true;
			setKind = true;
			continue;
		}
		//name
		if (!setName && !ifName)
		{
			name = strArray[i];
			ifName = true;
			setName = true;
			continue;
		}
	}
	if (!isInSearch() && !isInFind())
	{
		ifChange = false;
		for (int i = 0; i < nameOfDisks.size(); i++)
		{
			findThreadPtr[nameOfDisks[i] - 'A'] = AfxBeginThread(findThread, &in[nameOfDisks[i] - 'A']);
		}
	}
	else
	{
		ifChange = true;
	}
	// TODO:  在此添加控件通知处理程序代码
}


void CFileSearchDlg::OnClose()
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	//关闭线程
	for (int i = 0; i < nameOfDisks.size(); i++)
	{
		if (isSearch[nameOfDisks[i] - 'A'])
			TerminateThread(searchThreadPtr[nameOfDisks[i] - 'A']->m_hThread, 0);
	}
	for (int i = 0; i < nameOfDisks.size(); i++)
	{
		if (isFind[nameOfDisks[i] - 'A'])
			TerminateThread(findThreadPtr[nameOfDisks[i] - 'A']->m_hThread, 0);
	}
	//关闭文件监控
	SHChangeNotifyDeregister(fileNotifyId);
	CDialogEx::OnClose();
}

/*
（1）LVIF_TEXT   必须填充 pszText
（2）LVIF_IMAGE  必须填充 iImage
（3）LVIF_INDENT 必须填充 iIndent
（4）LVIF_PARAM  必须填充 lParam
（5）LVIF_STATE  必须填充 state
*/

void CFileSearchDlg::OnLvnGetdispinfoList(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	if (isInFind() || isInSearch())
		return;
	if (pDispInfo->item.mask & LVIF_TEXT)
	{
		unsigned long long item = pDispInfo->item.iItem;
		unsigned long long len = 0;
		char disk = '\0';
		for (int i = 0; i < nameOfDisks.size(); i++)
		{
			if (len + fileSearch[nameOfDisks[i] - 'A'].getFindResult().size() < item+1)
				len += (unsigned long long)fileSearch[nameOfDisks[i] - 'A'].getFindResult().size();
			else
			{
				disk = nameOfDisks[i];
				break;
			}
		}
		std::vector<FindResult> &vec = fileSearch[disk - 'A'].getFindResult();
		unsigned long long inItem = item - len;
		FileInfo res = fileSearch[disk - 'A'].getFileInfo(vec[inItem]);
		CString out;
		if (res.fileName == "*")
		{
			out = "已删除";
			res.fileName = "已删除";
			res.path = "已删除";
		}
		else
		{
			//文件大小
			out.Format(TEXT("%d Byte"), getFileSizeFromPath(res.path));
		}
		switch (pDispInfo->item.iSubItem)
		{
		case 0://文件名
			lstrcpy(pDispInfo->item.pszText,res.fileName);
			break;
		case 1://大小
			lstrcpy(pDispInfo->item.pszText, out);
			break;
		case 2://路径
			lstrcpy(pDispInfo->item.pszText, res.path);
			break;
		}
	}
}

afx_msg LRESULT CFileSearchDlg::OnFilemodify(WPARAM wParam, LPARAM lParam)
{
	FileChangeInfo *fciPtr = (FileChangeInfo*)wParam;
	WCHAR path[MAX_PATH],path2[MAX_PATH]; 
	SHGetPathFromIDList((LPCITEMIDLIST)fciPtr->item1, path);
	SHGetPathFromIDList((LPCITEMIDLIST)fciPtr->item2, path2);//四个DWORD 1 3代表文件名
	WCHAR diskName = path[0];
	WCHAR diskName2 = path2[0];
	switch (lParam)
	{
	case SHCNE_MKDIR:	//创建目录
	case SHCNE_CREATE:	//创建文件
		if (diskName >= 'A' && diskName <= 'Z')
			fileSearch[diskName - 'A'].upDataFile(lParam, path, path2);
		break;
	case SHCNE_RMDIR:	//删除目录
	case SHCNE_DELETE:	//删除文件
		if (diskName >= 'A' && diskName <= 'Z')
			fileSearch[diskName - 'A'].upDataFile(lParam, path, path2);
		break;
	case SHCNE_RENAMEFOLDER:	//文件夹重命名
	case SHCNE_RENAMEITEM:		//文件重命名
		if (diskName >= 'A' && diskName <= 'Z' && diskName2 >= 'A' && diskName2 <= 'Z')
			fileSearch[diskName - 'A'].upDataFile(lParam, path, path2);
		break;
	}
	OnEnChangeEdit();
	return 0;
}

void CFileSearchDlg::OnNMRClickList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;
	if (listControl.GetSelectedCount() <= 0)
		return;
	CMenu menu,*pMenu;
	menu.LoadMenuW(IDR_MENURCLICKLIST);
	pMenu = menu.GetSubMenu(0);
	CPoint pos;
	GetCursorPos(&pos);
	pMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,pos.x ,pos.y, this);
}


void CFileSearchDlg::OnRclicklistOpen()
{
	// TODO: 在此添加命令处理程序代码
	POSITION pos = listControl.GetFirstSelectedItemPosition();
	while (pos)
	{
		int index = -1;
		index = listControl.GetNextSelectedItem(pos);
		if (index >= 0 && listControl.GetItemCount() > index)
		{
			CString str = listControl.GetItemText(index, 2);
			ShellExecute(NULL,TEXT("open"),str,NULL,NULL, SW_SHOW);
		}
	}
}


void CFileSearchDlg::OnRclicklistCopyFilename()
{
	// TODO: 在此添加命令处理程序代码
	POSITION pos = listControl.GetFirstSelectedItemPosition();
	while (pos)
	{
		int index = -1;
		index = listControl.GetNextSelectedItem(pos);
		if (index >= 0 && listControl.GetItemCount() > index)
		{
			CString str = listControl.GetItemText(index, 0);
			if (::OpenClipboard(m_hWnd))
			{
				EmptyClipboard();
				HANDLE hData = ::GlobalAlloc(GMEM_MOVEABLE, str.GetLength() * 2 + 2);
				LPWSTR pData = (LPWSTR)::GlobalLock(hData);
				lstrcpy(pData, str);
				GlobalUnlock(hData);
				SetClipboardData(CF_UNICODETEXT, hData);
				CloseClipboard();
			}
		}
	}
}


void CFileSearchDlg::OnRclicklistCopyPath()
{
	// TODO: 在此添加命令处理程序代码
	POSITION pos = listControl.GetFirstSelectedItemPosition();
	while (pos)
	{
		int index = -1;
		index = listControl.GetNextSelectedItem(pos);
		if (index >= 0 && listControl.GetItemCount() > index)
		{
			CString str = listControl.GetItemText(index, 2);
			if (::OpenClipboard(m_hWnd))
			{
				EmptyClipboard();
				HANDLE hData = ::GlobalAlloc(GMEM_MOVEABLE, str.GetLength() * 2 + 2);
				LPWSTR pData = (LPWSTR)::GlobalLock(hData);
				lstrcpy(pData, str);
				GlobalUnlock(hData);
				SetClipboardData(CF_UNICODETEXT, hData);
				CloseClipboard();
			}
		}
	}
}


void CFileSearchDlg::OnBnClickedButton3()
{
	// TODO: 在此添加控件通知处理程序代码
	dialogAbout dlg;
	dlg.DoModal();
}


void CFileSearchDlg::OnBnClickedButton2()
{
	// TODO: 在此添加控件通知处理程序代码
	dialogInstruct dlg;
	dlg.DoModal();
}


void CFileSearchDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nID == SC_MINIMIZE) // 最小化窗口
	{
		notifyIconData.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
		notifyIconData.hWnd = this->m_hWnd;
		notifyIconData.uID = IDR_MAINFRAME;
		notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		notifyIconData.uCallbackMessage = WM_SHOWWND; // 自定义的消息名称 
		notifyIconData.hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
		lstrcpy(notifyIconData.szTip, _T("文件搜索工具")); // 信息提示条
		Shell_NotifyIcon(NIM_ADD, &notifyIconData); // 在托盘区添加图标 
		ShowWindow(SW_HIDE); // 隐藏主窗口 
		return;
	}
	CDialogEx::OnSysCommand(nID, lParam);
}


afx_msg LRESULT CFileSearchDlg::OnShowwnd(WPARAM wParam, LPARAM lParam)
{
	if (wParam != IDR_MAINFRAME)
		return 1;
	if (WM_LBUTTONDBLCLK == lParam)
	{
		this->ShowWindow(SW_SHOWNORMAL);
		this->SetForegroundWindow();
		Shell_NotifyIcon(NIM_DELETE, &notifyIconData);//删除托盘图标
	}
	if (lParam == WM_RBUTTONDOWN)
	{
		//弹出托盘菜单
		CMenu menu;
		menu.LoadMenu(IDR_MENUICON);
		CMenu *pMenu = menu.GetSubMenu(0);
		CPoint pnt;
		GetCursorPos(&pnt);
		SetForegroundWindow();
		pMenu->TrackPopupMenu(TPM_RIGHTBUTTON, pnt.x, pnt.y, this);
	}
	return 0;
}

void CFileSearchDlg::OnIconInstruct()
{
	// TODO: 在此添加命令处理程序代码
	dialogInstruct dlg;
	dlg.DoModal();
}


void CFileSearchDlg::OnIconAbout()
{
	// TODO: 在此添加命令处理程序代码
	dialogAbout dlg;
	dlg.DoModal();
}


void CFileSearchDlg::OnIconExit()
{
	// TODO: 在此添加命令处理程序代码
	OnClose();
	PostQuitMessage(0);
}


afx_msg LRESULT CFileSearchDlg::OnFindFinish(WPARAM wParam, LPARAM lParam)
{
	if (ifChange)//在查找或者搜索期间进行查找
	{
		OnEnChangeEdit();
	}
	return 0;
}
