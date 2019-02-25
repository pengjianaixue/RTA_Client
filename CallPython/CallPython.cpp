// CallPython.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "CallPython.h"
#include <sstream>

// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 CallPython.h
std::atomic<int> PyReosureCounter;
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
			DEBUGINFO("Initialized python Env failed.");
			PyReosureCounter = 0;
			Py_Finalize();

				

		}
		PyRun_SimpleString("import sys");
		
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
std::string CCallPython::Runfunction(std::string modulename, std::string functionname, std::string param_1)
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
	PyObject* pParam = nullptr;
	try
	{	
		PyObject *pArgs = PyTuple_New(1);
		PyTuple_SetItem(pArgs, 0, Py_BuildValue("i", param_1.c_str()));//0—序号 i表示创建int型变量 

		PyObject* pResult = PyObject_CallObject(pFunc, pArgs);
		//PyObject* pResult = PyObject_CallFunction(pFunc, "i", param_1.c_str());
		if (!pResult)
		{

			PyErr_Print();
		}
		else
		{
			Py_DECREF(pFunc);
		}
		
	}
	catch (const std::exception& e)
	{

		DEBUGINFO(e.what());
		return  "Fail";
	}
	
	return std::string("Success");
}
std::string CCallPython::Runfunction(std::string modulename, std::string functionname, std::string param_1, std::string param_2)
{	
	PyObject* pModule = PyImport_ImportModule(modulename.c_str());
	if (pModule == nullptr)
	{
		
		DEBUGINFO("import python module test failed.");
		return  "Fail";
	}
	PyObject* pFunc = PyObject_GetAttrString(pModule, functionname.c_str());
	if (pFunc == nullptr)
	{

		DEBUGINFO("Get python func failed.");
		return  "Fail";
	}
	PyObject* pParam = nullptr;
	pParam = Py_BuildValue("ss", param_1.c_str(), param_2.c_str());
	PyObject* pResult = PyObject_CallObject(pFunc, pParam);
	return std::string("Success");

}
std::string CCallPython::Runfunction(std::string modulename, std::string functionname, std::string param_1, std::string param_2, std::string param_3)
{
	PyObject* pModule = PyImport_ImportModule(modulename.c_str());
	if (pModule == nullptr)
	{
		DEBUGINFO("import python module test failed.");
		return  "Fail";
	}
	PyObject* pFunc = PyObject_GetAttrString(pModule, functionname.c_str());
	if (pFunc == nullptr)
	{
		DEBUGINFO("import python func failed.");
		return  "Fail";
	}
	PyObject* pParam = nullptr;
	pParam = Py_BuildValue("sss", param_1.c_str(), param_2.c_str(), param_3.c_str());
	PyObject* pResult = PyObject_CallObject(pFunc, pParam);
	return std::string("Success");
}
std::string CCallPython::Runfunction(std::string modulename, std::string functionname, std::string param_1, std::string param_2, std::string param_3, std::string param_4)
{
	PyObject* pModule = PyImport_ImportModule(modulename.c_str());
	if (pModule == nullptr)
	{
		DEBUGINFO("import python module test failed.");
		return  "Fail";
	}
	PyObject* pFunc = PyObject_GetAttrString(pModule, functionname.c_str());
	if (pFunc == nullptr)
	{
		DEBUGINFO("import python func failed.");
		return  "Fail";
	}
	PyObject* pParam = nullptr;
	pParam = Py_BuildValue("ssss", param_1.c_str(), param_2.c_str(), param_3.c_str(), param_4.c_str());
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
