// dialogAbout.cpp: 实现文件
//

#include "stdafx.h"
#include "FileSearch.h"
#include "dialogAbout.h"
#include "afxdialogex.h"


// dialogAbout 对话框

IMPLEMENT_DYNAMIC(dialogAbout, CDialogEx)

dialogAbout::dialogAbout(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOGABOUT, pParent)
{
	icoHandle = AfxGetApp()->LoadIconW(IDR_MAINFRAME);
}

dialogAbout::~dialogAbout()
{
}

void dialogAbout::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATICURL, urlControl);
	DDX_Control(pDX, IDC_PIC, icoControl);
}


BEGIN_MESSAGE_MAP(dialogAbout, CDialogEx)
	ON_WM_CREATE()
	ON_STN_CLICKED(IDC_STATICURL, &dialogAbout::OnStnClickedStaticurl)
END_MESSAGE_MAP()


// dialogAbout 消息处理程序


int dialogAbout::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	// TODO:  在此添加您专用的创建代码

	return 0;
}


void dialogAbout::OnStnClickedStaticurl()
{
	// TODO: 在此添加控件通知处理程序代码
	ShellExecute(NULL, TEXT("open"), TEXT("https://github.com/Coder-0x7fffffff"), NULL, NULL, SW_SHOW);
}


BOOL dialogAbout::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	icoControl.SetIcon(icoHandle);
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
