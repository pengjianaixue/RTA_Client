// CallPython.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "CallPython.h"
#include <sstream>

// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 CallPython.h
CCallPython::CCallPython():m_strpypath()
{	
	PyReosureCounter += 1;
	if (Py_IsInitialized() == 0)
	{

		//Py_SetPythonHome();
		std::wstringstream ws;
		ws << std::string("python").c_str();
		Py_SetProgramName(const_cast<wchar_t*>(ws.str().c_str()));
		Py_Initialize();
		if (Py_IsInitialized()==0)
		{	
			PyReosureCounter = 0;
			Py_Finalize();
				

		}
		PyRun_SimpleString("importsys");
		
	}
}
CCallPython::CCallPython(std::string PythonPath):m_strpypath(PythonPath)
{
	
	CCallPython();

}
CCallPython::~CCallPython()
{

	if (PyReosureCounter==1)
	{
		PyReosureCounter = 0;
		Py_Finalize();
	}
	else
	{
		PyReosureCounter -= 1;
	}


}
std::string CCallPython::Runfunction(std::string modulename, std::string functionname, std::list<std::pair<int, std::string>> paramlist)
{

	PyObject* pModule = PyImport_ImportModule(modulename.c_str());
	if (pModule == nullptr) 
	{
		DEBUGINFO("import python module test failed.");
		return  "Fail";
	}
	PyObject* pFunc = PyObject_GetAttrString(pModule, functionname.c_str());
	if (pFunc ==nullptr) 
	{

		DEBUGINFO("import python func failed." );
		return  "Fail";
	}
	PyObject* pParam = Py_BuildValue("(s)", "hahaha");
	PyObject* pResult = PyObject_CallObject(pFunc, pParam);
	return std::string("Success");
}
int CCallPython::RunPyfile(std::string pyfilename)
{

	return 0;
}
void CCallPython::SetPyPath(std::string Pypath)
{
	

	std::string Pypathset;
	Pypathset = "sys.path.append('" + Pypath + "')";
	PyRun_SimpleString(Pypathset.c_str());
	return;
}
int CCallPython::importmodule(std::string modulename)
{

	PyImport_ImportModule(modulename.c_str());
	return 0;
}
std::string Runfunction(std::string functionname, std::list<std::string > ParamList)
{
	
	return std::string("");

}
std::string RunModule(std::string ModuleName, std::list<std::string > ParamList)
{


	
	return std::string("");



}

CALLPYTHON_API std::string ReleasePy(std::string ModuleName, std::list<std::string> ParamList)
{

	return  std::string();
}
