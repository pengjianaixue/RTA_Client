#pragma once

#include <QDialog>
#include <QtXml\qxml.h>
#include <qxmlstream.h>
#include "Trace.h"
#include "treemodel.h"
#include "ui_CaseScriptConfigure.h"
#include "../CallPython/CallPython.h"

class CaseScriptConfigure : public QDialog
{

	Q_OBJECT
public:
	CaseScriptConfigure(QWidget *parent = Q_NULLPTR);
	~CaseScriptConfigure();
	void ParamInit();
	typedef QPair<QString, QStringList> QSTRINGLISTPAIR;
private:
	bool PyRun();
	void ConnectSlots();
	
public slots:
	int  TestRun();
	int  Reset();
	bool SetPyFilePath(const QString &path);
	bool ReLoadPyFilePath();
	bool LoadCaseFileListInfo(const QString  &filepath);
	bool GetPyCasePathAndLaodCaseFile();
private:
	Ui::CaseScriptConfigure ui;
	std::string							m_strRunres;
	CCallPython							m_cpycaller;
	PARAMLIST							m_callpyparmlist;
	QString								m_strpythonfilehome;
	QSTRINGLISTPAIR						m_pycasefileanddirinfo;
	QStringList							m_pycasefilenamelist;
	QList<QSTRINGLISTPAIR>				m_pycasetreeinfostruct;
	QList<QString>						m_ignorePyDirNameList;
	TreeModel							*m_CaseTreeModel;
};
