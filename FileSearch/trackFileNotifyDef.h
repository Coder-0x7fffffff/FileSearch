#pragma once
#include <Windows.h>
//�궨��
#define SHCNRF_InterruptLevel 0x0001 //�жϼ���Ϣ <- �ļ�ϵͳ
#define SHCNRF_ShellLevel 0x0002 //shell������Ϣ <- shell
#define SHCNRF_RecursiveInterrupt 0x1000 //����Ŀ¼�����ж��¼�
#define SHCNRF_NewDelivery 0x8000 //���յ�����Ϣʹ�ù����ڴ档
struct FileChangeInfo
{
	DWORD* item1; //�ɵ��ļ�������PIDL
	DWORD* item2; //�µ��ļ�������PIDL
	//DWORD item3; //�ɵ��ļ�������PIDL
	//DWORD item4; //�µ��ļ�������PIDL
};
//typedef struct tagFILECHANGENOTIFY 
//{
//	DWORD refCount;
//	FileChangeInfo fileChangeInfo;
//}FileChangeNotify;

//��Ϣ
#define WM_FILEMODIFY (WM_USER + 124)