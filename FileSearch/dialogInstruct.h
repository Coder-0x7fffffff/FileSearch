#pragma once


// dialogInstruct 对话框

class dialogInstruct : public CDialogEx
{
	DECLARE_DYNAMIC(dialogInstruct)

public:
	dialogInstruct(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~dialogInstruct();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOGINSTRUCT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CStatic instructStatic;
	virtual BOOL OnInitDialog();
};
