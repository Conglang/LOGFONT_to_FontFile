
// LOGFONT_to_FontFileDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
using std::string;
using std::wstring;
using std::unordered_map;
using std::unordered_set;

// CLOGFONT_to_FontFileDlg �Ի���
class CLOGFONT_to_FontFileDlg : public CDialogEx
{
// ����
public:
	CLOGFONT_to_FontFileDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_LOGFONT_TO_FONTFILE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedChooseFont();
	afx_msg void OnBnClickedSetTarget();
	afx_msg void OnBnClickedCopyFontFile();
	CStatic m_fontname;
	CStatic m_fontfile;
	CEdit m_targetpath;

private:
	// font related
	bool generate_font_name_path_map();
	bool get_all_font_file_in_registry();
	void scan_TTF_OTF_file_for_name(wstring path);
	void scan_TTC_file_for_name(wstring path);
	void read_offset_table(CFile& f, wstring path);

	wstring get_font_path_from_logfont(const LOGFONT& logfont);
	wstring get_font_name_from_logfont(const LOGFONT& logfont);

	void copy_font_to_target_folder();

private:
	unordered_map<wstring, wstring> m_font_name_path_dictionary;
	wstring m_font_path;
	wstring m_target_path;

private:
	void MultiByteToWideChar1(const std::string& s,std::wstring& d);
};
