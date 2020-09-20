#pragma once
#include <vector>
#include <unordered_map>
#include <set>
#include <winioctl.h>
#include "trackFileNotifyDef.h"
#include "wildCard.h"
#include "oper.h"

//文件结构 pfrn->file_info
struct FileIden
{
	unsigned long long parentFileReferenceNum;
	CString fileNmae;
};

//查找结果
struct FindResult
{
	unsigned long long rn;
	char volumeName;
	FindResult(unsigned long long rn,char volumeName)
	{
		this->rn = rn;
		this->volumeName = volumeName;
	}
};

//文件信息
struct FileInfo
{
	CString fileName;
	CString path;
	FileInfo(CString fileName, CString path)
	{
		this->fileName = fileName;
		this->path = path;
	}
	FileInfo()
	{
		fileName = CString("");
		path = CString("");
	}
};

//CString作为map的key 重载hash
struct CStringHash
{
	size_t operator () (const CString &s) const
	{
		CStringA ss(s);
		return std::hash<std::string>()(static_cast<LPCSTR>(ss));
	}
};

typedef std::unordered_multimap<unsigned long long, FileIden> hashRN2FI;
typedef std::unordered_multimap<CString, unsigned long long, CStringHash> hashFN2RN;

class SearchFile
{
private:
	//NTFS磁盘记录
	std::vector<char> nameOfDisks;
	//文件遍历变量
	HANDLE hDisk[26];													//磁盘handle
	USN_JOURNAL_DATA usnJournalData[26];								//USN的Info
	LONGLONG fileCount[26];
	//hash_map
	hashRN2FI fileMap[26];												//rn映射文件信息
	hashFN2RN fileMap2[26];												//文件名映射rn
	//文件名列表
	std::set<CString> fileNameSet[26];
	//查找临时量 --> 减少复制
	std::vector<FindResult> findResult;									//查找结果
	std::vector<FindResult> fullFindResult;								//操作临时查找结果
	//内部操作
	void countNTFSDisk();												//扫描NTFS磁盘有哪些
	bool createUSN(char volumeName);									//开启USN记录
	bool closeUSN(char volumeName);										//关闭USN记录
	bool openDisk(char volumeName);										//打开磁盘
	bool closeDisk(char volumeName);									//关闭磁盘
	bool getUSNInfo(char volumeName);									//获取USN的Info
	void searchFileOfDisk(char volumeName);								//查找所有在这个磁盘上的文件
	//内部全名查找
	std::vector<FindResult>& findFileFromFullName(CString name);		//查找文件的rn
public:
	SearchFile();
	~SearchFile();
	void setNTFSDisk(std::vector<char> list);
	std::vector<FindResult>& getFindResult();
	void searchAllFile();												//遍历系统所有文件
	CString getPath(unsigned long long rn, char volumeName);			//查找文件路径
	std::vector<FindResult>& findFileFromName(CString name);			//查找文件的rn
	void resetFind();													//重置
	FileInfo getFileInfo(FindResult res);								//查找文件信息
	void upDataFile(LPARAM lParam,WCHAR *path,WCHAR *path2);
};