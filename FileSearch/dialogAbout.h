#pragma once


// dialogAbout 对话框

class dialogAbout : public CDialogEx
{
	DECLARE_DYNAMIC(dialogAbout)

public:
	dialogAbout(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~dialogAbout();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOGABOUT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	CStatic urlControl;
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnStnClickedStaticurl();
	CStatic icoControl;
	HICON icoHandle;
	virtual BOOL OnInitDialog();
};
