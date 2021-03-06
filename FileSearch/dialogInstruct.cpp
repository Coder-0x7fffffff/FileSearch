// dialogInstruct.cpp: 实现文件
//

#include "stdafx.h"
#include "FileSearch.h"
#include "dialogInstruct.h"
#include "afxdialogex.h"


// dialogInstruct 对话框

IMPLEMENT_DYNAMIC(dialogInstruct, CDialogEx)

dialogInstruct::dialogInstruct(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOGINSTRUCT, pParent)
{

}

dialogInstruct::~dialogInstruct()
{
}

void dialogInstruct::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATICINSTRUCT, instructStatic);
}


BEGIN_MESSAGE_MAP(dialogInstruct, CDialogEx)
END_MESSAGE_MAP()


// dialogInstruct 消息处理程序


BOOL dialogInstruct::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  在此添加额外的初始化
	instructStatic.SetWindowTextW(TEXT("注意事项:\n\t1.重启后打开软件系统会恢复USN需要等待, 重启软件则不会\n\t2.最小化即最小化到托盘\n\t3.关闭即直接退出程序\n使用说明 : \n\t1.直接输入文件名即可自动检索\n\t2.右击列表可以进行对应操作\n\t3.左键双击托盘图标显示主窗口\n\t4.右击托盘图标显示操作菜单\n关于搜索 : \n\t1.关键字使用(不区分大小写)\n\t\t1 > 关键字使用|隔开\n\t\t2 > size关键字：size + 比较符 + 大小(整数) + 单位(默认为B) eg: size > 100kb\n\t\t3 > size的比较符 > < >= <= = <>(不等于)\n\t\t4 > type关键字：type:+类型后缀 eg : type:png\n\t2.通配符\n\t\t1 > 通配符即嵌入搜索文本中\n\t\t2 > *通配符：可以代替0个或者多个字符\n\t\t3 > ? 通配符：替代一个字符\n\t\t4 > eg: xi?m*i 匹配 : xiami、xiamiii 不匹配 : xiaami、xiam\n"));
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}
