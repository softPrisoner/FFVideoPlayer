#pragma once
#include <qstring.h>

class CommonUtils {
public:
	CommonUtils();
	~CommonUtils();
	//дIni����
	static void writeIni(QString inipath, QString keystr, QString value);
	//��ȡIni�ļ�
	static QString readIni(QString inipath, QString keystr);

};