
// LOGFONT_to_FontFileDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "LOGFONT_to_FontFile.h"
#include "LOGFONT_to_FontFileDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLOGFONT_to_FontFileDlg 对话框




CLOGFONT_to_FontFileDlg::CLOGFONT_to_FontFileDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLOGFONT_to_FontFileDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLOGFONT_to_FontFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SHOW_FONT_NAME, m_fontname);
	DDX_Control(pDX, IDC_FONT_FILE, m_fontfile);
	DDX_Control(pDX, IDC_EDIT_TARGET, m_targetpath);
}

BEGIN_MESSAGE_MAP(CLOGFONT_to_FontFileDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CHOOSE_FONT, &CLOGFONT_to_FontFileDlg::OnBnClickedChooseFont)
	ON_BN_CLICKED(IDC_SET_TARGET, &CLOGFONT_to_FontFileDlg::OnBnClickedSetTarget)
	ON_BN_CLICKED(IDC_COPY_FONT_FILE, &CLOGFONT_to_FontFileDlg::OnBnClickedCopyFontFile)
END_MESSAGE_MAP()


// CLOGFONT_to_FontFileDlg 消息处理程序

BOOL CLOGFONT_to_FontFileDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	
	//---------------------------------------------------
	// scan system fonts and generate a dictionary 
	// matching all fonts' names to it's file path
	//---------------------------------------------------
	bool result(generate_font_name_path_map());
	//---------------------------------------------------

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CLOGFONT_to_FontFileDlg::OnPaint()
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
HCURSOR CLOGFONT_to_FontFileDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CLOGFONT_to_FontFileDlg::OnBnClickedChooseFont()
{
	// open font dialog
	LOGFONT this_font;
	CFontDialog dlg(&this_font,  CF_NOVERTFONTS|CF_TTONLY| CF_SCREENFONTS, NULL);
	if (IDOK == dlg.DoModal())
	{
		// get logfont
		this_font = *(dlg.m_cf.lpLogFont);
		// get font name
		wstring font_name(get_font_name_from_logfont(this_font));
		m_fontname.SetWindowText(font_name.c_str());
		// get font path
		m_font_path = get_font_path_from_logfont(this_font);
		if (m_font_path.empty())
		{
			m_fontfile.SetWindowText(_T("Not Found"));
		}else
		{
			m_fontfile.SetWindowText(m_font_path.c_str());
		}
	}
}


void CLOGFONT_to_FontFileDlg::OnBnClickedSetTarget()
{
	// open choose path dialog
	wchar_t pszPath[MAX_PATH];  
	BROWSEINFO bi;   
	bi.hwndOwner      = this->GetSafeHwnd();  
	bi.pidlRoot       = NULL;  
	bi.pszDisplayName = NULL;   
	bi.lpszTitle      = TEXT("Please choose target folder:");   
	bi.ulFlags        = BIF_RETURNONLYFSDIRS | BIF_STATUSTEXT;  
	bi.lpfn           = NULL;   
	bi.lParam         = 0;  
	bi.iImage         = 0;   

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);  
	if (pidl == NULL)  
	{  
		return;  
	}  

	if (SHGetPathFromIDList(pidl, pszPath))  
	{  
		m_target_path = pszPath;
		m_targetpath.SetWindowText(pszPath);
	}
}


void CLOGFONT_to_FontFileDlg::OnBnClickedCopyFontFile()
{
	// is font path found
	if (m_target_path.empty()) return;
	// copy file to target folder
	copy_font_to_target_folder();
}

//--------------------------------------------------------------------------------------------------------------
// For Font Header Translate
// Ref: 
// http://www.microsoft.com/typography/otspec/otff.htm
// This is TTP file header
typedef struct _tagTT_OFFSET_TABLE{
	USHORT uMajorVersion;
	USHORT uMinorVersion;
	USHORT uNumOfTables;
	USHORT uSearchRange;
	USHORT uEntrySelector;
	USHORT uRangeShift;
}TT_OFFSET_TABLE;

//Tables in TTF file and there placement and name (tag)
typedef struct _tagTT_TABLE_DIRECTORY{
	char szTag[4]; //table name
	ULONG uCheckSum; //Check sum
	ULONG uOffset; //Offset from beginning of file
	ULONG uLength; //length of the table in bytes
}TT_TABLE_DIRECTORY;

//Header of names table
typedef struct _tagTT_NAME_TABLE_HEADER{
	USHORT uFSelector; //format selector. Always 0
	USHORT uNRCount; //Name Records count
	USHORT uStorageOffset; //Offset for strings storage, 
	//from start of the table
}TT_NAME_TABLE_HEADER;

//Record in names table
typedef struct _tagTT_NAME_RECORD{
	USHORT uPlatformID;
	USHORT uEncodingID;
	USHORT uLanguageID;
	USHORT uNameID;
	USHORT uStringLength;
	USHORT uStringOffset; //from start of storage area
}TT_NAME_RECORD;
//---------------------------------------------------
// For TTC
// TTC Header
typedef struct _tagTT_TTC_HEADER{
	char ttcTag[4];	// TrueType Collection ID string: 'ttcf'
	USHORT uMajorVersion;
	USHORT uMinorVersion;	// Version of the TTC Header (1.0 or 2.0)
	ULONG uNumFonts;	// Number of fonts in TTC
	//ULONG uOffsetTable[numFonts];	// Array of offsets to the OffsetTable for each font from the beginning of the file.
}TT_TTC_HEADER;

// TTF file is stored in Motorolla style(Big Endian), need these 
// 2 macros to rearrange bytes in variables retrieved from TrueType font file.
#define SWAPWORD(x) MAKEWORD(HIBYTE(x), LOBYTE(x))
#define SWAPLONG(x) MAKELONG(SWAPWORD(HIWORD(x)), SWAPWORD(LOWORD(x)))

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383
//--------------------------------------------------------------------------------------------------------------
bool CLOGFONT_to_FontFileDlg::generate_font_name_path_map()
{
	if (!get_all_font_file_in_registry()) return false;
	auto temp(m_font_name_path_dictionary);
	for (auto it = temp.begin(); it != temp.end(); ++it)
	{
		wstring full_path((*it).second);
		std::transform(full_path.begin(), full_path.end(), full_path.begin(), toupper);
		if (full_path.find(_T(".TTC")) != wstring::npos)
		{
			scan_TTC_file_for_name(full_path);
		}else
		{
			scan_TTF_OTF_file_for_name(full_path);
		}
	}
	return true;
}
//---------------------------------------------------
bool CLOGFONT_to_FontFileDlg::get_all_font_file_in_registry()
{
	//---------------------------------------------------
	// read registry, and record all known font name and path in dictionary
	//---------------------------------------------------
	unordered_map<wstring, wstring> temp_keyname_valuename_dic;
	// open registry of font_name / font_file_name
	HKEY hFontKey;
	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts"), 0, KEY_READ, &hFontKey))
	{
		RegCloseKey(hFontKey);
	}
	TCHAR    class_name[MAX_PATH] = TEXT("");	// buffer for class name   
	DWORD    class_name_size = MAX_PATH;		// size of class string   
	DWORD    num_subkey=0;						// number of subkeys   
	DWORD    longest_subkey_size;				// longest subkey size   
	DWORD    longest_class_string;				// longest class string   
	DWORD    num_key_value;						// number of values for key   
	DWORD    longest_value_name;				// longest value name   
	DWORD    longest_value_data;				// longest value data   
	DWORD    security_descriptor_size;			// size of security descriptor   
	FILETIME last_write_time;					// last write time

	// get the class name and the value count.
	if (ERROR_SUCCESS != RegQueryInfoKey(
		hFontKey,
		class_name,
		&class_name_size,
		NULL,
		&num_subkey,
		&longest_subkey_size,
		&longest_class_string,
		&num_key_value,
		&longest_value_name,
		&longest_value_data,
		&security_descriptor_size,
		&last_write_time))
	{
		RegCloseKey(hFontKey);
		return false;
	}
	// enumerate the key values
	for (DWORD i = 0; i < num_key_value; ++i)
	{
		DWORD key_name_size = MAX_VALUE_NAME;
		TCHAR  key_name[MAX_VALUE_NAME]; 
		key_name[0] = '\0';
		if (ERROR_SUCCESS != RegEnumValue(
			hFontKey, i,
			key_name,
			&key_name_size,
			NULL, NULL,
			NULL, NULL))
		{
			RegCloseKey(hFontKey);
			return false;
		}
		wstring temp(key_name);
		wstring font_name(temp.substr(0, temp.find('(')-1));

		DWORD bytes(0);
		DWORD type(0);
		if (ERROR_SUCCESS != RegQueryValueEx(
			hFontKey,
			key_name,
			NULL,
			&type,
			NULL,
			&bytes))
		{
			RegCloseKey(hFontKey);
			return false;
		}
		wchar_t* fname = new wchar_t[bytes];
		if (ERROR_SUCCESS != RegQueryValueEx(
			hFontKey,
			key_name,
			NULL,
			&type,
			(LPBYTE)fname,
			&bytes))
		{
			RegCloseKey(hFontKey);
			return false;
		}
		wstring file_name(fname);
		delete[] fname;
		fname = NULL;
		temp_keyname_valuename_dic.insert(make_pair(font_name, file_name));
	}
	RegCloseKey(hFontKey);
	//---------------------------------------------------
	// get folders path of font files
	//---------------------------------------------------
	HKEY hFontPathKey;
	// open registry of font file folder
	if (ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"), 0, KEY_READ, &hFontPathKey))
	{
		RegCloseKey(hFontPathKey);
		return false;
	}
	DWORD path_data_size;
	TCHAR path_data[MAX_VALUE_NAME];
	path_data[0] = '\0';
	if (ERROR_SUCCESS != RegQueryValueEx(hFontPathKey, TEXT("Fonts"), NULL, NULL, (LPBYTE)path_data, &path_data_size))
	{
		RegCloseKey(hFontPathKey);
		return false;
	}
	wstring font_path(path_data);
	RegCloseKey(hFontPathKey);
	//---------------------------------------------------
	// Put path and file name together, insert into dictionary
	//---------------------------------------------------
	for (auto it = temp_keyname_valuename_dic.begin(); it != temp_keyname_valuename_dic.end(); ++it)
	{
		wstring font_name = (*it).first;
		wstring file_name = (*it).second;
		wstring full_path(file_name);
		if (file_name.find(_T("\\")) == wstring::npos)	// exclude some of the file names that have full path
		{
			full_path = font_path + _T("\\") + file_name;
		}
		m_font_name_path_dictionary.insert(make_pair(font_name, full_path));
	}
	return true;
}
//---------------------------------------------------
void CLOGFONT_to_FontFileDlg::scan_TTF_OTF_file_for_name( wstring path )
{
	CFile f;
	if (f.Open(path.c_str(), CFile::modeRead|CFile::shareDenyWrite))
	{
		read_offset_table(f, path);
		f.Close();
	}
}
//---------------------------------------------------
void CLOGFONT_to_FontFileDlg::scan_TTC_file_for_name( wstring path )
{
	CFile f;
	if (f.Open(path.c_str(), CFile::modeRead|CFile::shareDenyWrite))
	{
		TT_TTC_HEADER ttTTCHeader;
		f.Read(&ttTTCHeader, sizeof(TT_TTC_HEADER));
		ttTTCHeader.uMajorVersion = SWAPWORD(ttTTCHeader.uMajorVersion);
		ttTTCHeader.uMinorVersion = SWAPWORD(ttTTCHeader.uMinorVersion);
		ttTTCHeader.uNumFonts = SWAPLONG(ttTTCHeader.uNumFonts);
		if (ttTTCHeader.uMajorVersion != 1 && ttTTCHeader.uMajorVersion != 2)
		{
			// not valid ttc file
			f.Close();
			return;
		}
		int num_fonts(ttTTCHeader.uNumFonts);
		unordered_set<ULONG> num_offsets;
		ULONG desoffset(0);
		for (int i = 0; i < num_fonts; ++i)
		{
			f.Read(&desoffset, sizeof(ULONG));
			desoffset = SWAPLONG(desoffset);
			if (desoffset != 0)
			{
				num_offsets.insert(desoffset);
			}
		}
		// check each font data inside one ttc.
		for (auto it = num_offsets.begin(); it != num_offsets.end(); ++it)
		{
			ULONG this_offset = *it;
			f.Seek(this_offset, CFile::begin);
			// read each offset table
			read_offset_table(f, path);
		}
		f.Close();
	}
}
//---------------------------------------------------
void CLOGFONT_to_FontFileDlg::read_offset_table( CFile& f , wstring path)
{
	TT_OFFSET_TABLE ttOffsetTable;
	f.Read(&ttOffsetTable, sizeof(TT_OFFSET_TABLE));

	ttOffsetTable.uNumOfTables = SWAPWORD(ttOffsetTable.uNumOfTables);
	ttOffsetTable.uMajorVersion = SWAPWORD(ttOffsetTable.uMajorVersion);
	ttOffsetTable.uMinorVersion = SWAPWORD(ttOffsetTable.uMinorVersion);

	TT_TABLE_DIRECTORY tblDir;
	BOOL bFound = FALSE;
	for (int i = 0; i < ttOffsetTable.uNumOfTables; ++i)
	{
		f.Read(&tblDir, sizeof(TT_TABLE_DIRECTORY));
		char des[5] = "\0";
		strncpy_s(des, tblDir.szTag, 4);
		string temp(des);
		wstring found_name;
		MultiByteToWideChar1(temp, found_name);
		std::transform(found_name.begin(), found_name.end(), found_name.begin(), toupper);
		if (found_name == wstring(_T("NAME")))
		{
			// we found names table. rearrange order and quit the loop
			bFound = TRUE;
			tblDir.uLength = SWAPLONG(tblDir.uLength);
			tblDir.uOffset = SWAPLONG(tblDir.uOffset);
			break;
		}
	}
	if (bFound)
	{
		f.Seek(tblDir.uOffset, CFile::begin);
		TT_NAME_TABLE_HEADER ttNTHeader;
		f.Read(&ttNTHeader, sizeof(TT_NAME_TABLE_HEADER));

		ttNTHeader.uNRCount = SWAPWORD(ttNTHeader.uNRCount);
		ttNTHeader.uStorageOffset = SWAPWORD(ttNTHeader.uStorageOffset);
		TT_NAME_RECORD ttRecord;
		bFound = FALSE;
		for(int i=0; i<ttNTHeader.uNRCount; i++)
		{
			wstring font_name_local;

			f.Read(&ttRecord, sizeof(TT_NAME_RECORD));
			ttRecord.uNameID = SWAPWORD(ttRecord.uNameID);
			ttRecord.uPlatformID = SWAPWORD(ttRecord.uPlatformID);
			ttRecord.uEncodingID = SWAPWORD(ttRecord.uEncodingID);
			ttRecord.uLanguageID = SWAPWORD(ttRecord.uLanguageID);

			//1 says that this is font name. 0 for example determines copyright info
			if(ttRecord.uNameID == 4 || ttRecord.uNameID == 6)
			{
				ttRecord.uStringLength = SWAPWORD(ttRecord.uStringLength);
				ttRecord.uStringOffset = SWAPWORD(ttRecord.uStringOffset);

				//save file position, so we can return to continue with search
				int nPos = static_cast<int>(f.GetPosition());
				f.Seek(tblDir.uOffset + ttRecord.uStringOffset + 
					ttNTHeader.uStorageOffset, CFile::begin);

				int a[16] = {	0x0C09, 0x2809, 0x1009, 0x2409,
					0x4009, 0x1809, 0x2009, 0x4409,
					0x1409, 0x3409, 0x4809, 0x1C09,
					0x2C09, 0x0809, 0x0409, 0x3009};
				unordered_set<int> english_lang(a, a+16);
				int b[5] = {	0x0C04, 0x1404, 0x0804, 0x1004, 0x0404};
				unordered_set<int> chinese_lang(b, b+5);

				// English Name and Traditional Chinese and some other
				if ((ttRecord.uLanguageID == 0) || (ttRecord.uPlatformID == 1) || (ttRecord.uPlatformID == 3 && english_lang.count(ttRecord.uLanguageID)))
				{
					char lpszNameBuf[MAX_VALUE_NAME] = {"\0"};
					ZeroMemory(lpszNameBuf, ttRecord.uStringLength + 1);

					f.Read(lpszNameBuf, ttRecord.uStringLength);
					std::string temp_string1(lpszNameBuf);
					font_name_local = _T("");
					if(!temp_string1.empty())
					{
						if (ttRecord.uPlatformID == 1 && ttRecord.uLanguageID == 19)	// Traditional Chinese ANSI -> Big5
						{
							wchar_t lpstr_w[MAX_VALUE_NAME] = {0};
							MultiByteToWideChar(950, MB_PRECOMPOSED, lpszNameBuf, temp_string1.size(), lpstr_w, temp_string1.size());
							font_name_local = lpstr_w;
						}else
						{
							MultiByteToWideChar1(temp_string1, font_name_local);
						}
					}
				} 
				// Simplified Chinese
				else if(ttRecord.uPlatformID == 3 && chinese_lang.count(ttRecord.uLanguageID))
				{
					wchar_t lpszNameBuf[MAX_VALUE_NAME] = {0};
					ZeroMemory(lpszNameBuf, ttRecord.uStringLength + 1);
					f.Read(lpszNameBuf, ttRecord.uStringLength);
					for (UINT i = 0; i < MAX_VALUE_NAME; ++i)
					{
						lpszNameBuf[i] = SWAPWORD(lpszNameBuf[i]);
					}
					font_name_local =lpszNameBuf;
				} 
				// Other Languages
				else
				{
					wchar_t lpszNameBuf[MAX_VALUE_NAME] = {0};
					ZeroMemory(lpszNameBuf, ttRecord.uStringLength + 1);
					f.Read(lpszNameBuf, ttRecord.uStringLength);
					for (UINT i = 0; i < MAX_VALUE_NAME; ++i)
					{
						lpszNameBuf[i] = SWAPWORD(lpszNameBuf[i]);
					}
					font_name_local =lpszNameBuf;
					//font_name_local = _T("");
				}

				if(font_name_local.size() > 0)
				{
					m_font_name_path_dictionary.insert(make_pair(font_name_local,path));
				}
				f.Seek(nPos, CFile::begin);
			}
		}
	}
}
//---------------------------------------------------
wstring CLOGFONT_to_FontFileDlg::get_font_path_from_logfont( const LOGFONT& logfont )
{
	wstring font_name = get_font_name_from_logfont(logfont);
	// combine probabal font name
	unordered_set<wstring> weight_probability;
	unordered_set<wstring> italic_probability;
	unordered_set<wstring> font_name_probability;

	font_name_probability.insert(font_name);
	font_name_probability.insert(font_name.substr(0, font_name.find(_T(" Regular"))));
	font_name_probability.insert(font_name+_T("ensed"));
	// Italic
	if (logfont.lfItalic)
	{
		italic_probability.insert(_T("Italic"));
		italic_probability.insert(_T("It"));
		italic_probability.insert(_T("Itali"));
		italic_probability.insert(_T("Slanted"));
		italic_probability.insert(_T("Oblique"));
		italic_probability.insert(_T(""));
	}else
	{
		italic_probability.insert(_T(""));
	}
	// Weight
	switch(logfont.lfWeight)
	{
	case 250:
		weight_probability.insert(_T("ExtraLight"));
		weight_probability.insert(_T(""));
		break;
	case 300:
		weight_probability.insert(_T("Light"));
		weight_probability.insert(_T(""));
		break;
	case 400:
		weight_probability.insert(_T(""));
		weight_probability.insert(_T("Regular"));
		weight_probability.insert(_T("Medium"));
		break;
	case 500:
		weight_probability.insert(_T("Medium"));
		weight_probability.insert(_T(""));
		break;
	case 600:
		weight_probability.insert(_T("Semibold"));
		weight_probability.insert(_T("Bold"));
		weight_probability.insert(_T(""));
		break;
	case 700:
		weight_probability.insert(_T("Black"));
		weight_probability.insert(_T("Bold"));
		weight_probability.insert(_T("old"));
		weight_probability.insert(_T(""));
		break;
	case 900:
		weight_probability.insert(_T("Heavy"));
		weight_probability.insert(_T(""));
		break;
	default:
		weight_probability.insert(_T(""));
		weight_probability.insert(_T("Regular"));
		break;
	}
	wstring combined_font_name;
	for (auto w_it = weight_probability.begin(); w_it != weight_probability.end(); ++w_it)
	{
		for (auto i_it = italic_probability.begin(); i_it != italic_probability.end(); ++i_it)
		{
			for (auto n_it = font_name_probability.begin(); n_it != font_name_probability.end(); ++n_it)
			{
				font_name = *n_it;
				// TrueType way
				combined_font_name = font_name;
				if ((*w_it).size() > 0)
				{
					combined_font_name += _T(" ") + *w_it;
				}
				if ((*i_it).size() > 0)
				{
					combined_font_name += _T(" ") + *i_it;
				}
				if (m_font_name_path_dictionary.count(combined_font_name))
				{
					return m_font_name_path_dictionary.at(combined_font_name);
				}
			}
		}
	}
	// try name and name with regular
	if (m_font_name_path_dictionary.count(font_name))
	{
		return m_font_name_path_dictionary.at(font_name);
	}
	if (m_font_name_path_dictionary.count(font_name+_T("Regular")))
	{
		return m_font_name_path_dictionary.at(font_name+_T("Regular"));
	}
	return _T("");
}
//---------------------------------------------------
wstring CLOGFONT_to_FontFileDlg::get_font_name_from_logfont( const LOGFONT& logfont )
{
	// get font name
	return logfont.lfFaceName;
}
//---------------------------------------------------
void CLOGFONT_to_FontFileDlg::copy_font_to_target_folder()
{
	if (m_font_path.empty() || m_target_path.empty()) return;
	wstring filepath(m_font_path);
	wstring filename = filepath.substr(filepath.find_last_of(_T("\\")), -1);
	wstring to_filepath(m_target_path+filename);
#ifdef _UNICODE
	FILE* fp(NULL);
	_wfopen_s(&fp, filepath.c_str(), L"rb");
	FILE* fw(NULL);
	_wfopen_s(&fw, to_filepath.c_str(), L"wb");
#else
	FILE* fp(NULL);
	fopen_s(filepath.c_str(), "rb");
	FILE* fw(NULL);
	fopen_s(to_filepath.c_str(), "wb");
#endif
	long longBytes = 0;
	if (!fp || !fw)
	{
		fclose(fp);
		fclose(fw);
	} else
	{
		fseek(fp,0,SEEK_SET);fseek(fp,0,SEEK_END);longBytes = ftell(fp);// longBytes is the size of the file
		fseek(fp,0,SEEK_SET);
		BYTE* font_data = NULL;
		font_data = (BYTE*)malloc(longBytes);
		if (!font_data)
		{
			fclose(fp);
			fclose(fw);
		}
		int hh = fread(font_data,1,longBytes,fp);
		fwrite(font_data, longBytes,1, fw);
		fclose(fw);
		fclose(fp);
		delete font_data;
		font_data = NULL;
	}
	AfxMessageBox(_T("Copy Complete"));
}
//--------------------------------------------------------------------------------------------------------------
void CLOGFONT_to_FontFileDlg::MultiByteToWideChar1( const std::string& s,std::wstring& d )
{
	int nSize = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)s.c_str(), s.size(), 0, 0);
	if(nSize <= 0) return;
	WCHAR *pwszDst = new WCHAR[nSize+1];
	if( NULL == pwszDst) return;
	MultiByteToWideChar(CP_ACP, 0,(LPCSTR)s.c_str(), s.size(), pwszDst, nSize);
	pwszDst[nSize] = 0;
	if( pwszDst[0] == 0xFEFF) // skip Oxfeff
		for(int i = 0; i < nSize; i ++) 
			pwszDst[i] = pwszDst[i+1]; 
	d = pwszDst;
	delete pwszDst;
}
