// CallPython.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "CallPython.h"
#include <sstream>

// 这是已导出类的构造函数。
// 有关类定义的信息，请参阅 CallPython.h
std::atomic<int> CCallPython::PyReosureCounter = 0;
CCallPython::CCallPython()
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
CCallPython::CCallPython(std::string PythonPath)
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
std::string CCallPython::Runfunction(std::string modulename, std::string functionname, PARAMLIST param)
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
	try
	{	

		unsigned int paramnum = param.size();
		PyObject *pArgs = PyTuple_New(paramnum);
		for (auto &parampair: param)
		{

			PyTuple_SetItem(pArgs, 0, Py_BuildValue(parampair.first.c_str() , parampair.second.c_str()));//0—序号 i表示创建int型变量 

		}
		
		PyObject* pResult = PyObject_CallObject(pFunc, pArgs);
		if (!pResult)
		{

			PyErr_Print();
		}
		else
		{

			char *res = NULL;
			int retok = PyArg_Parse(pResult, "s", &res); // 
			Py_DECREF(pArgs);
			Py_DECREF(pFunc);
			Py_DECREF(pResult);
			Py_DECREF(pModule);
			std::string ret(res);
			return ret;
		}
		
	}
	catch (const std::exception& e)
	{

		DEBUGINFO(e.what());
		return  "Fail";
	}
	
	return std::string(" ");
}
std::string CCallPython::Runfunction(std::string modulename, std::string functionname, PARAMPAIR param_1, PARAMPAIR param_2)
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
	//pParam = Py_BuildValue("ss", param_1.c_str(), param_2.c_str());
	PyObject* pResult = PyObject_CallObject(pFunc, pParam);
	return std::string("Success");

}
std::string CCallPython::Runfunction(std::string modulename, std::string functionname, PARAMPAIR param_1, PARAMPAIR param_2, PARAMPAIR param_3)
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
	//pParam = Py_BuildValue("sss", param_1.c_str(), param_2.c_str(), param_3.c_str());
	PyObject* pResult = PyObject_CallObject(pFunc, pParam);
	return std::string("Success");
}
std::string CCallPython::Runfunction(std::string modulename, std::string functionname, PARAMPAIR param_1, PARAMPAIR param_2, PARAMPAIR param_3, PARAMPAIR param_4)
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
	//pParam = Py_BuildValue("ssss", param_1.c_str(), param_2.c_str(), param_3.c_str(), param_4.c_str());
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
std::string Runfunction(std::string mudulename ,std::string functionname, PARAMLIST ParamList)
{
	

	CCallPython a;
	try
	{
		a.Runfunction(mudulename, functionname, ParamList);
	}
	catch (const std::exception&)
	{
		return "Fail";
	}
	return std::string("Success");

}
