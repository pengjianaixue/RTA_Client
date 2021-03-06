#include "stdafx.h"
#include "pyCaller.h"
#include <sstream>

std::atomic<int> PyCaller::PyReosureCounter = 0;
PyCaller::PyCaller()
{	
	
	PyReosureCounter += 1;
	if (Py_IsInitialized() == 0)
	{

		//Py_SetPythonHome();
		std::wstringstream ws;
		ws << std::string("python").c_str();
		Py_SetProgramName(const_cast<wchar_t*>(ws.str().c_str()));
		if (!Py_IsInitialized())
		{
			Py_Initialize();
		}
		if (!Py_IsInitialized())
		{	
			DEBUGINFO("Initialized python env failed.");
			PyReosureCounter = 0;
		}
		PyRun_SimpleString("import sys");
	}
}
PyCaller::PyCaller(const std::string& PythonPath):PyCaller()
{
	setPyPath(PythonPath);
}
PyCaller::~PyCaller()
{
	if (PyReosureCounter==1)
	{
		PyReosureCounter = 0;
	}
	else
	{
		PyReosureCounter -= 1;
	}
	if (PyCaller::PyReosureCounter == 0 && Py_IsInitialized()>0)
	{
		Py_FinalizeEx();
	}

}
int PyCaller::runPyFile(const std::string &pyfilename)
{
	//PyCodec_StreamReader
	PyObject *pyfileobj = Py_BuildValue("s", pyfilename.c_str());
	FILE *file = nullptr;
	file = _Py_fopen_obj(pyfileobj, "r+");
	if (file!=nullptr) 
	{
#ifdef _DEBUG
		int ret = PyRun_SimpleFile(file, pyfilename.c_str());
		return ret;
#else
		return PyRun_SimpleFile(file, pyfilename.c_str());
#endif // _DEBUG
	
	}
	return -1;
}
void PyCaller::setPyPath(const std::string &Pypath/*="./"*/)
{
	
	std::string Pypathset;
	Pypathset = "sys.path.append('" + Pypath + "')";
	PyRun_SimpleString(Pypathset.c_str());
	return;
}
std::list<std::string> PyCaller::GetPyFileDict(std::string & modulename)
{
	std::list<std::string> ret;
	PyObject* pModule = nullptr;
	pModule = PyImport_ImportModule(modulename.c_str());
	if (pModule == nullptr)
	{
		//DEBUGINFO("import python module:"+ modulename +" failed.");
		ret.push_back(std::string("failed:import python module:" + modulename + " failed."));
		return ret;
	}
	PyObject* pModuledict =  PyModule_GetDict(pModule);
	if (pModuledict == nullptr)
	{
		ret.push_back(std::string("failed:Get  python module:" + modulename + " dict fail."));
	}
	else
	{
		;
	}
	return std::list<std::string>();
}
int PyCaller::importModule(const std::string& modulename)
{
	PyImport_ImportModule(modulename.c_str());
	return 0;
}
void PyCaller::emptyCallStoreVar()
{
	m_pyModuleName = std::string();
	m_functionName = std::string();
	m_callArgs = nullptr;
	m_parameterCounter = 0;
}
std::string Runfunction(std::string mudulename ,std::string functionname, PyCaller::PYPARAMLIST ParamList)
{
	
	/*PyCaller a;
	try
	{
		a.runPyFunction(mudulename, functionname, ParamList,nullptr);
	}
	catch (const std::exception&)
	{
		return "Fail";
	}*/
	return std::string("Success");

}
CALLPYTHON_API bool FinalizePyIter()
{
	if (PyCaller::PyReosureCounter ==0)
	{
		return (Py_FinalizeEx()==0);
	}
	return true;
}
