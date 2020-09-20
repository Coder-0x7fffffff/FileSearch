#pragma once
#include <Windows.h>
//宏定义
#define SHCNRF_InterruptLevel 0x0001 //中断级消息 <- 文件系统
#define SHCNRF_ShellLevel 0x0002 //shell级别消息 <- shell
#define SHCNRF_RecursiveInterrupt 0x1000 //接受目录所有中断事件
#define SHCNRF_NewDelivery 0x8000 //接收到的消息使用共享内存。
struct FileChangeInfo
{
	DWORD* item1; //旧的文件名或者PIDL
	DWORD* item2; //新的文件名或者PIDL
	//DWORD item3; //旧的文件名或者PIDL
	//DWORD item4; //新的文件名或者PIDL
};
//typedef struct tagFILECHANGENOTIFY 
//{
//	DWORD refCount;
//	FileChangeInfo fileChangeInfo;
//}FileChangeNotify;

//消息
#define WM_FILEMODIFY (WM_USER + 124)