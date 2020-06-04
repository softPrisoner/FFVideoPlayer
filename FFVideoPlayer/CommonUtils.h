#pragma once
#include <qstring.h>

class CommonUtils {
public:
	CommonUtils();
	~CommonUtils();
	//Ğ´IniÅäÖÃ
	static void writeIni(QString inipath, QString keystr, QString value);
	//¶ÁÈ¡IniÎÄ¼ş
	static QString readIni(QString inipath, QString keystr);

};