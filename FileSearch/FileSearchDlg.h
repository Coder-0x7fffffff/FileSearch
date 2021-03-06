
// FileSearchDlg.h: 头文件
//

#pragma once


//头文件
#include <winioctl.h>
#include "trackFileNotifyDef.h"
#include "oper.h"
#include "wildCard.h"
#include "SearchFile.h"
#include "dialogAbout.h"
#include "dialogInstruct.h"

//托盘窗口
#define WM_SHOWWND (WM_USER +123)
#define WM_FIND_FINISH (WM_USER +125)

//太丑了！！！！两个大小变换警告
#pragma warning(disable:4312)

// CFileSearchDlg 对话框
class CFileSearchDlg : public CDialogEx
{
public:
	//监控文件变量
	ULONG fileNotifyId;
	//扫描磁盘变量
	SearchFile fileSearch[26];	//磁盘文件检索类数组
	CWinThread *searchThreadPtr[26], *findThreadPtr[26],*updataListThreadPtr;	//线程数组
	bool isSearch[26],isFind[26];	//状态数组
	std::vector<FileInfo> findInfo;	//查找的文件记录详细信息
	int lenOfEachPage;	//页面长度记录
	std::vector<char> nameOfDisks;	//NTFS盘记录
	//判断状态函数
	bool isInSearch();
	bool isInFind();
	//扫描磁盘个数
	void countNTFSDisk();
	//查找
	bool ifChange;	//是否在查找时候更改名字
	bool ifName;
	CString name;
	bool ifKind;	//是否启用类型筛选
	CString kind;
	bool ifSize;	//是否启用大小筛选
	ULONGLONG size;
	int sizeKind;
	//托盘
	NOTIFYICONDATA notifyIconData;
	//文件大小
	long getFileSizeFromPath(const wchar_t * filename);
// 构造
public:
	CFileSearchDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_FILESEARCH_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CEdit editControl;
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnEnChangeEdit();
	afx_msg void OnClose();
	CListCtrl listControl;
	afx_msg void OnLvnGetdispinfoList(NMHDR *pNMHDR, LRESULT *pResult);
	CStatic outLineControl;
protected:
	afx_msg LRESULT OnFilemodify(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnNMRClickList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnRclicklistOpen();
	afx_msg void OnRclicklistCopyFilename();
	afx_msg void OnRclicklistCopyPath();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
protected:
	afx_msg LRESULT OnShowwnd(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnIconInstruct();
	afx_msg void OnIconAbout();
	afx_msg void OnIconExit();
protected:
	afx_msg LRESULT OnFindFinish(WPARAM wParam, LPARAM lParam);
};
