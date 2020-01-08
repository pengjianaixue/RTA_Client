// 下列 ifdef 块是创建使从 DLL 导出更简单的
// 宏的标准方法。此 DLL 中的所有文件都是用命令行上定义的 CALLPYTHON_EXPORTS
// 符号编译的。在使用此 DLL 的
// 任何其他项目上不应定义此符号。这样，源文件中包含此文件的任何其他项目都会将
// CALLPYTHON_API 函数视为是从 DLL 导入的，而此 DLL 则将用此宏定义的
// 符号视为是被导出的。
#ifdef CALLPYTHON_EXPORTS
#define CALLPYTHON_API __declspec(dllexport)
#else
#define CALLPYTHON_API __declspec(dllimport)
#endif


#include "Python.h"
#include "frameobject.h"
#include "traceback.h"
#include <string>
#include <list>
#include <atomic>
#include <iostream>

#ifdef _DEBUG
#define DEBUGINFO(param)  std::cout<<  param <<std::endl
#else
#define DEBUGINFO(param)  
#endif // _DEBUG
//extern std::atomic<int> PyReosureCounter;
//template  < typename _T> ;

class CALLPYTHON_API PyCaller 
{
public:
	using PYPARAMPAIR	= std::pair<std::string, std::string > ;
	using PYPARAMLIST	= std::list<PYPARAMPAIR> ;
	using PyResult		= PyObject*	;
public:
	PyCaller();
	PyCaller(const std::string& PythonPath);
	PyCaller(const PyCaller&) = delete;
	~PyCaller();
	static std::atomic<int> PyReosureCounter;
public:
	int  runPyFile(const std::string &pyfilename);
	void setPyPath(const std::string &Pypath="./");
	std::list<std::string> GetPyFileDict(std::string &modulename);
	template<typename T>
	bool runPyFunction(const std::string &moduleName, const std::string &functionName, const PYPARAMLIST& callParamter, T& resultrsave);
	template<typename T, typename ...argtype>
	bool runPyFunction(const std::string& moduleName, const std::string& functionName, PyResult resultrsave, const T& firstParam, const argtype& ...pyarg);
	template<typename T>
	static bool pyResultCast(T& vaule, PyResult resultToParse);
	template<>
	static bool pyResultCast(std::string& vaule, PyResult resultToParse);

private:
	std::string m_pyPath;
private:
	int importModule(const std::string& modulename);
	
};
template<>
bool PyCaller::pyResultCast(std::string& vaule, PyResult resultToParse)
{
	if (!resultToParse)
	{
		return false;
	}
	char *res = { nullptr };
	int parseRes = 0;
	parseRes = PyArg_Parse(resultToParse, "s", &res);
	if (res)
	{
		vaule = std::string(res);
		return true;
	}
	return false;
}

template<typename T>
bool PyCaller::pyResultCast(T& vaule, PyResult resultToParse)
{
	if (!resultToParse)
	{
		return false;
	}
	char *res = { nullptr };
	int parseRes = 0;
	if (typeid(T) == typeid(int) || typeid(T) == typeid(long  long) || typeid(T) == typeid(long))
	{
		PyLong_Check(resultToParse);
		vaule = PyLong_AsLongLong(resultToParse);
		return true;
	}
	else if (typeid(T) == typeid(double) || typeid(T) == typeid(float))
	{
		PyFloat_Check(resultToParse);
		vaule = PyFloat_AsDouble(resultToParse);
		return true;
	}
	else if (typeid(T) == typeid(unsigned int) || typeid(T) == typeid(unsigned long long))
	{
		vaule = PyLong_AsUnsignedLongLong(resultToParse); //PyArg_Parse(resultToParse, "K", &res);
		return true;
	}
	return false;

}
template<typename T>
bool PyCaller::runPyFunction(const std::string &moduleName, const std::string &functionName, const PYPARAMLIST& callParamter, T& resultrsave)
{

	PyObject* pModule = nullptr;
	pModule = PyImport_ImportModule(moduleName.c_str());
	if (pModule == nullptr)
	{
		//DEBUGINFO("import python module:"+ modulename +" failed.");
		std::cerr << "failed:import python module:" + moduleName + " failed.";
		return  false;

	}
	PyObject* pFunc = PyObject_GetAttrString(pModule, functionName.c_str());
	if (pFunc == nullptr)
	{

		//DEBUGINFO("import python func:" + functionname + " failed" );
		std::cerr << "failed:import python func:" + functionName + " failed.";
		return false;
	}
	try
	{
		unsigned int paramNum = callParamter.size();
		PyObject *pArgs = PyTuple_New(paramNum);
		size_t paramnumconunter = 0;
		for (auto &paramPair : callParamter)
		{

			if (paramPair.first == "i")
			{
				int paramtransfer = std::stoi(paramPair.second);
				PyTuple_SetItem(pArgs, paramnumconunter, Py_BuildValue(paramPair.first.c_str(), paramtransfer));//0—sequence num  i-  type int
			}
			else if (paramPair.first == "I" || paramPair.first == "k")
			{
				size_t paramtransfer = std::stoul(paramPair.second);
				PyTuple_SetItem(pArgs, paramnumconunter, Py_BuildValue(paramPair.first.c_str(), paramtransfer));//0—sequence num  i-  type int
			}
			else if (paramPair.first == "l" || paramPair.first == "L")
			{
				long long paramtransfer = std::stoll(paramPair.second);
				PyTuple_SetItem(pArgs, paramnumconunter, Py_BuildValue(paramPair.first.c_str(), paramtransfer));//0—sequence num  i-  type int
			}
			else if (paramPair.first == "s")
			{
				PyTuple_SetItem(pArgs, paramnumconunter, Py_BuildValue(paramPair.first.c_str(), paramPair.second.c_str()));//0—sequence num  i-  type int
			}
			paramnumconunter++;
		}
		if (!PyCallable_Check(pFunc))
		{
			return false;
		}
		PyObject *callResult = PyObject_CallObject(pFunc, pArgs);
		if (!callResult)
		{
			PyObject *perror = nullptr;
			perror = PyErr_Occurred();
			PyObject *type, *value, *traceback;
			type = nullptr;
			value = nullptr;
			traceback = nullptr;
			if (perror)
			{
				std::string tracebackfull = "fail:";
#pragma region GetThreadState
				PyThreadState *tstate = PyThreadState_GET();
				if (NULL != tstate)
				{
					PyTracebackObject* traceback = (PyTracebackObject*)tstate->curexc_traceback;
					if (nullptr == traceback)
					{
						goto fetcherror;
					}
					PyFrameObject *frame = traceback->tb_frame;
					while (NULL != frame) {
						/*
						frame->f_lineno will not always return the correct line number
						you need to call PyCode_Addr2Line().
						*/
						int line = PyCode_Addr2Line(traceback->tb_frame->f_code, traceback->tb_frame->f_lasti);
						std::string str_filename = PyUnicode_AsUTF8(traceback->tb_frame->f_code->co_filename);
						std::string funcname = PyUnicode_AsUTF8(traceback->tb_frame->f_code->co_name);
						tracebackfull += "fetch Error in  File: " + str_filename + "\r\nfunction: " + funcname +
							"\r\nLine: " + std::to_string(line) + "\r\n";
						frame = frame->f_back;
					}
					Py_XDECREF(traceback);
					Py_XDECREF(frame);
					Py_XDECREF(tstate);

				}
#pragma endregion GetThreadState
				/*#pragma region GetFull TraceBack
				PyObject *trace_moudle = PyImport_ImportModule("traceback");
				if (!trace_moudle)
				{
				return "Fetch Error";
				}

				PyObject * python_func = PyObject_GetAttrString(trace_moudle,"format_exception");
				if (python_func && PyCallable_Check(python_func))
				{

				PyObject *pyres = nullptr;
				pyres = PyObject_CallFunctionObjArgs(python_func, type, value, traceback, NULL);
				if (pyres)
				{
				PyObject *pyo_tarceback = PyObject_Str(pyres);
				std::string traceback = PyUnicode_AsUTF8(pyo_tarceback);
				Py_DECREF(pyres);
				Py_DECREF(python_func);
				return traceback;
				}
				else
				{
				return std::string("fetch full traceback fail");
				}

				}
				#pragma endregion GetFull TraceBack*/
			fetcherror:
				PyErr_Fetch(&type, &value, &traceback);
				if (type || value || traceback)
				{
					std::string errorinfo_1, errorinfo_2, errorinfo_3;
					if (type && (std::string(type->ob_type->tp_name) == std::string("str")))
					{
						errorinfo_1 = PyUnicode_AsUTF8(type);
					}
					if (value && (std::string(value->ob_type->tp_name) == std::string("str")))
					{
						errorinfo_1 = PyUnicode_AsUTF8(value);
					}
					if (traceback && (std::string(traceback->ob_type->tp_name) == std::string("str")))
					{
						errorinfo_2 = PyUnicode_AsUTF8(traceback);
					}
					/*PyErr_NormalizeException(&type, &value, &traceback);*/
					tracebackfull += " error cause:" + errorinfo_1 + errorinfo_2 + errorinfo_3;
					std::cerr << tracebackfull;
					Py_CLEAR(pArgs);
					Py_CLEAR(pFunc);
					Py_CLEAR(pModule);
					return  false;
				}
#ifdef _DEBUG
				PyErr_Print();
#endif // DEBUG
			}
		}
		Py_CLEAR(pArgs);
		Py_CLEAR(pFunc);
		Py_CLEAR(pModule);
		pyResultCast(resultrsave, callResult);
		return true;
	}
	catch (const std::exception& e)
	{
		std::cerr << (e.what());
	}
}


//extern CALLPYTHON_API int nCallPython;
//CALLPYTHON_API int fnCallPython(void);
#ifdef __cplusplus
	extern "C" {
#endif // _cplusplus
	
	CALLPYTHON_API std::string Runfunction(std::string mudulename, std::string functionname, PyCaller::PYPARAMLIST ParamList);
	CALLPYTHON_API bool FinalizePyIter();
#ifdef __cplusplus
			}
#endif // _cplusplus

	
