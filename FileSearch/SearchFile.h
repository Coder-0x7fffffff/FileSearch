#pragma once
#include <vector>
#include <unordered_map>
#include <set>
#include <winioctl.h>
#include "trackFileNotifyDef.h"
#include "wildCard.h"
#include "oper.h"

//�ļ��ṹ pfrn->file_info
struct FileIden
{
	unsigned long long parentFileReferenceNum;
	CString fileNmae;
};

//���ҽ��
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

//�ļ���Ϣ
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

//CString��Ϊmap��key ����hash
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
	//NTFS���̼�¼
	std::vector<char> nameOfDisks;
	//�ļ���������
	HANDLE hDisk[26];													//����handle
	USN_JOURNAL_DATA usnJournalData[26];								//USN��Info
	LONGLONG fileCount[26];
	//hash_map
	hashRN2FI fileMap[26];												//rnӳ���ļ���Ϣ
	hashFN2RN fileMap2[26];												//�ļ���ӳ��rn
	//�ļ����б�
	std::set<CString> fileNameSet[26];
	//������ʱ�� --> ���ٸ���
	std::vector<FindResult> findResult;									//���ҽ��
	std::vector<FindResult> fullFindResult;								//������ʱ���ҽ��
	//�ڲ�����
	void countNTFSDisk();												//ɨ��NTFS��������Щ
	bool createUSN(char volumeName);									//����USN��¼
	bool closeUSN(char volumeName);										//�ر�USN��¼
	bool openDisk(char volumeName);										//�򿪴���
	bool closeDisk(char volumeName);									//�رմ���
	bool getUSNInfo(char volumeName);									//��ȡUSN��Info
	void searchFileOfDisk(char volumeName);								//������������������ϵ��ļ�
	//�ڲ�ȫ������
	std::vector<FindResult>& findFileFromFullName(CString name);		//�����ļ���rn
public:
	SearchFile();
	~SearchFile();
	void setNTFSDisk(std::vector<char> list);
	std::vector<FindResult>& getFindResult();
	void searchAllFile();												//����ϵͳ�����ļ�
	CString getPath(unsigned long long rn, char volumeName);			//�����ļ�·��
	std::vector<FindResult>& findFileFromName(CString name);			//�����ļ���rn
	void resetFind();													//����
	FileInfo getFileInfo(FindResult res);								//�����ļ���Ϣ
	void upDataFile(LPARAM lParam,WCHAR *path,WCHAR *path2);
};