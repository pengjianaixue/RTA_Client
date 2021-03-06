#include "stdafx.h"
#include "MainWindows.h"

MainWindows::MainWindows(QWidget *parent)
	: QMainWindow(parent), m_pyeditorprocess(new QProcess(this)),
	  PythonHomeSet(new CaseandEditorhome(this)), m_RunPythonCaseprocess(nullptr)
{

	Init();
	
}
bool MainWindows::openconfigform(int i)
{
	m_UICaseConfigure.show();
	return true;
}
MainWindows::~MainWindows()
{
	delete m_pyeditorprocess;
	if (m_RunPythonCaseprocess.IsRuning())
	{
		emit Signal_emitPyCaseStop();
	}	
	m_CaseRunThread.terminate();
	//delete m_RunPythonCaseprocess;
}
bool MainWindows::LoadInitXmlConfigure()
{
	CXmlConfigureFileOperation Reader(this, m_strInitXmlFilePath, CXmlConfigureFileOperation::OPERATIONTYPE::READ);
	QString Pycharmpath;
	QString PycasePath;
	if (Reader.ReadInitXml("Pycharmpath", Pycharmpath)&&!Pycharmpath.isEmpty())
	{
		TRACE(Pycharmpath);
		m_strPycharmBinPath = Pycharmpath;
	}
	if (Reader.ReadInitXml("PyCasePath", PycasePath)&&!PycasePath.isEmpty())
	{
		TRACE(PycasePath);
		m_strPyCaseFileHomePath = PycasePath;
		emit Signal_emitpycasefilehomepath(m_strPyCaseFileHomePath);
	}
	PythonHomeSet->SetPypathExtra(m_strPyCaseFileHomePath, m_strPycharmBinPath);
	
	return true;
}

bool MainWindows::Init()
{
	uiInit();
	m_TheCurrentPath = QDir::currentPath();
	//TODO this absolutely path is only for  local test 
	m_strInitXmlFilePath = m_TheCurrentPath + R"(\Initloadfile\InitParam.xml)";
	m_hPycharmProcessid = 0;
	ConnectSlots();
	m_CaseRunThread.setPriority(QThread::Priority::HighPriority);
	m_CaseRunThread.start();
	LoadInitXmlConfigure();
	if (!m_strPyCaseFileHomePath.isEmpty())
	{
		m_TheCurrentPath = m_strPyCaseFileHomePath;
	}
	return true;
}

void MainWindows::closeEvent(QCloseEvent * event)
{

	QMessageBox msgbox(this);
	msgbox.setText(tr("Are you sure to close RTS ? "));
	msgbox.setWindowTitle(tr("exit confirm"));
	msgbox.setIcon(QMessageBox::Icon::Question);
	QAbstractButton *applyButton =   msgbox.addButton("Apply", QMessageBox::ActionRole);
	QAbstractButton *cancelButton =  msgbox.addButton("Cancel", QMessageBox::ActionRole);
	msgbox.exec();
	if (msgbox.clickedButton() == cancelButton) 
	{
		event->ignore();
	}
	else
	{
		CXmlConfigureFileOperation Wirter(this, m_strInitXmlFilePath, CXmlConfigureFileOperation::OPERATIONTYPE::WRITE);
		Wirter.WirteInitXml("Pycharmpath", m_strPycharmBinPath);
		Wirter.WirteInitXml("PyCasePath", m_strPyCaseFileHomePath);
		Wirter.ForamtEndEleAndSave();
	}

}

bool MainWindows::openpycharmIDE()
{
	
	if (m_strPyCaseFileHomePath.isEmpty()|| m_strPycharmBinPath.isEmpty())
	{
		QMessageBox::critical(this, "File path error ", "Please Set Python case home ");
		return false;
	}
	if (m_pyeditorprocess->pid()!=nullptr && m_hPycharmProcessid == m_pyeditorprocess->pid()->dwProcessId)
	{
		QMessageBox::warning(this, "Warning", "the  python editor  pycharm have opened!");
	}
	QString pypath =  m_strPyCaseFileHomePath ;
	QString program = m_strPycharmBinPath + R"(/pycharm64.exe)"; //R"(D:/SoftwareInstall/JetBrains/PyCharm 2018.1.1/bin/pycharm64.exe)";
	QFileInfo  pycasedir(program);
	if (!pycasedir.isExecutable())
	{
		QMessageBox::critical(this, "File path error", "the pycharm path have no pycharm64.exe file!");
	}
	QStringList arguments;
	arguments << pypath;
	qDebug() << program << "\n" << arguments;
	m_pyeditorprocess->start(program, arguments);
	m_pyeditorprocess->waitForStarted();
	if (m_pyeditorprocess->pid())
	{
		m_hPycharmProcessid = m_pyeditorprocess->pid()->dwProcessId;
		return false;
	}
	this->ui.action_Script_Editor_Pycharm->setChecked(true);
	return true;
}

bool MainWindows::SetPycaseFilehome()
{

	PythonHomeSet->show();
	return true;
}

bool MainWindows::Recvipycasefilehomepath(const QString & pycasefilehomepath)
{

	m_strPyCaseFileHomePath = pycasefilehomepath;
	if (!m_strPyCaseFileHomePath.isEmpty())
	{
		m_TheCurrentPath = m_strPyCaseFileHomePath;
		emit Signal_emitpycasefilehomepath(m_strPyCaseFileHomePath);
	}
	return true;
}

bool MainWindows::RecviPycharmhomepath(const QString & Pycharmhomepath)
{
	m_strPycharmBinPath = Pycharmhomepath;
	return true;
}

bool MainWindows::RunPyFileInTerminal()
{
	if (m_RunPythonCaseprocess.IsRuning())
	{
		 if(QMessageBox::StandardButton::Apply ==  QMessageBox::warning(this, "case run waring", "python case is running,whether you want to stop it", QMessageBox::StandardButton::Apply, QMessageBox::StandardButton::Cancel))
		 {
			 emit Signal_emitPyCaseStop();
		 }
		 else
		 {
			 QMessageBox::information(this, "case run information", "user have cancel run case requset");
		 }
		//m_RunPythonCaseprocess.Stop();
	}
	else
	{
		m_RunPythonCaseprocess.RegisterRunList(m_CaseExecListToFullPathList);
		m_RunPythonCaseprocess.moveToThread(&m_CaseRunThread);
		/*m_CaseRunThread.start();*/
		emit Signal_emitPyCaseRun();
		//m_RunPythonCaseprocess.Start();

	}
	return true;
}
//// Backup
void MainWindows::DisplayToTerminal(const QString &PyProcessRunInfor)
{
	this->ui.PTE_TerimnalDisplayArea->appendPlainText(PyProcessRunInfor);
	this->ui.PTE_TerimnalDisplayArea->update();
	return;
}


void MainWindows::GetExecuteCaseList()
{
	m_CaseExecListToFullPathList =  this->m_UICaseConfigure.GetCaseExecuteList();
}
bool MainWindows::ConnectSlots()
{
	if (
			!(
				connect(this->ui.ScriptConfigure, &QAction::triggered, this, &MainWindows::openconfigform) 
				&& connect(this->ui.action_SetPythonFileHome, &QAction::triggered, this, &MainWindows::SetPycaseFilehome) 
				&& connect(this->ui.action_Script_Editor_Pycharm, &QAction::triggered, this, &MainWindows::openpycharmIDE) 
				&& connect(this->PythonHomeSet, &CaseandEditorhome::Signal_eimtPythonFileHome, this, &MainWindows::Recvipycasefilehomepath) 
				&& connect(this->PythonHomeSet, &CaseandEditorhome::Signal_eimtPycharmHome, this, &MainWindows::RecviPycharmhomepath) 
				&& connect(this, &MainWindows::Signal_emitpycasefilehomepath, &this->m_UICaseConfigure, &CaseScriptConfigure::SetPyFilePath)
				&& connect(this->ui.Auto_Test, &QAction::triggered, this, &MainWindows::RunPyFileInTerminal)
				&& connect(&this->m_RunPythonCaseprocess, &PyScriptProcess::s_ProcessOutPutinfo, this, &MainWindows::DisplayToTerminal)
				&& connect(&this->m_UICaseConfigure, &CaseScriptConfigure::s_emitCaseExecListChanged,this, &MainWindows::GetExecuteCaseList)
				&& connect(this->ui.pushButton_clearTerminal, &QPushButton::clicked, this->ui.PTE_TerimnalDisplayArea, &QPlainTextEdit::clear)
				&& connect(this, &MainWindows::Signal_emitPyCaseRun, &this->m_RunPythonCaseprocess, &PyScriptProcess::Start)
				&& connect(this, &MainWindows::Signal_emitPyCaseStop, &this->m_RunPythonCaseprocess, &PyScriptProcess::Stop)
				
			)
		)
	{

		throw(" connect signal and  slots faile in MainWindows ");

	}
	return true;
}

void MainWindows::uiInit()
{
	ui.setupUi(this);
	this->tabifyDockWidget(this->ui.dockWidget_Terminal, this->ui.dockWidget_Opeartioninfodisp);
	this->tabifyDockWidget(this->ui.dockWidget_Opeartioninfodisp, this->ui.dockWidget_ErrorInfo);
	this->ui.dockWidget_Terminal->raise();
	this->ui.PTE_TerimnalDisplayArea->setReadOnly(true);
}
