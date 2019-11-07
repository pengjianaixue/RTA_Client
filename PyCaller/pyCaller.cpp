#include "stdafx.h"
#include "pyCaller.h"
#include <sstream>

std::atomic<int> pyCaller::PyReosureCounter = 0;
pyCaller::pyCaller()
{	
	
	PyReosureCounter += 1;
	if (Py_IsInitialized() == 0)
	{

		//Py_SetPythonHome();
		std::wstringstream ws;
		ws << std::string("python").c_str();
		Py_SetProgramName(const_cast<wchar_t*>(ws.str().c_str()));
		if (Py_IsInitialized() == 0)
		{
			Py_Initialize();
		}
		if (Py_IsInitialized()==0)
		{	
			DEBUGINFO("Initialized python Env failed.");
			PyReosureCounter = 0;
			Py_Finalize();
		}
		PyRun_SimpleString("import sys");
	}
}
pyCaller::pyCaller(std::string PythonPath)
{
	
	pyCaller();

}
pyCaller::~pyCaller()
{
	if (PyReosureCounter==1)
	{
		PyReosureCounter = 0;
	}
	else
	{
		PyReosureCounter -= 1;
	}
	if (pyCaller::PyReosureCounter == 0)
	{
		Py_FinalizeEx();
	}

}
template<typename T>
bool pyCaller::pyResultCast(T& vaule, PyResult resultToParse)
{
	/*std::list<std::pair<decltype,std::string>> pyTypeMapChar;
	pyTypeMapChar.append({decltype(int),"i"});
	pyTypeMapChar.append({ decltype(double),"i" });
	pyTypeMapChar.append({ decltype(long long),"l" });
	pyTypeMapChar.append({ decltype(std::string),"s" });*/
	char *res = {nullptr};
	int parseRes = 0;
	if (typeid(T) == typeid(int) || typeid(T) == typeid( long  long) || typeid(T) == typeid(long))
	{
		PyLong_Check(resultToParse);
		vaule = PyLong_AsLongLong(resultToParse);
	}
	else if (typeid(T) == typeid(double) || typeid(T) == typeid(float))
	{
		PyFloat_Check(resultToParse);
		vaule = PyFloat_AsDouble(resultToParse);
		return true;
	}
	else if (typeid(T) == typeid(unsigned int)  || typeid(T) == typeid(unsigned long long))
	{
		parseRes = PyArg_Parse(resultToParse, "K", &res);
		if (res)
		{
			vaule = std::stoul(res);
			return parseRes;
		}
	}
	else if(typeid(T) == typeid(std::string) )
	{
		parseRes = PyArg_Parse(resultToParse, "s", &res);
		if (res)
		{
			vaule = std::string(res);
			return parseRes;
		}
	}
	return false;

}
bool pyCaller::runPyFunction(const std::string &modulename, const std::string &functionname, const PYPARAMLIST &param, PyResult resultrsave)
{
	
	PyObject* pModule = nullptr;
	pModule = PyImport_ImportModule(modulename.c_str());
	if (pModule == nullptr) 
	{	
		//DEBUGINFO("import python module:"+ modulename +" failed.");
		std::cerr << "faile:import python module:" + modulename + " failed.";
		return  false;
		
	}
	PyObject* pFunc = PyObject_GetAttrString(pModule, functionname.c_str());
	if (pFunc ==nullptr)
	{

		//DEBUGINFO("import python func:" + functionname + " failed" );
		std::cerr << "faile:import python func:" + functionname + " failed.";
		return false;
	}
	try
	{	
		unsigned int paramnum = param.size();
		PyObject *pArgs = PyTuple_New(paramnum);
		size_t paramnumconunter = 0;
		for (auto &parampair: param)
		{

			if (parampair.first == "i")
			{
				int paramtransfer  = std::stoi(parampair.second);
				PyTuple_SetItem(pArgs, paramnumconunter, Py_BuildValue(parampair.first.c_str(), paramtransfer));//0—sequence num  i-  type int
			}
			else if (parampair.first == "I" || parampair.first == "k")
			{
				size_t paramtransfer = std::stoul(parampair.second);
				PyTuple_SetItem(pArgs, paramnumconunter, Py_BuildValue(parampair.first.c_str(), paramtransfer));//0—sequence num  i-  type int
			}
			else if (parampair.first == "l" || parampair.first == "L")
			{
				long long paramtransfer = std::stoll(parampair.second);
				PyTuple_SetItem(pArgs, paramnumconunter, Py_BuildValue(parampair.first.c_str(), paramtransfer));//0—sequence num  i-  type int
			}
			else if (parampair.first == "s")
			{
				PyTuple_SetItem(pArgs, paramnumconunter, Py_BuildValue(parampair.first.c_str(), parampair.second.c_str()));//0—sequence num  i-  type int
			}
			paramnumconunter++;
		}
		if (!PyCallable_Check(pFunc))
		{
			return false;
		}
		PyObject *pResult = PyObject_CallObject(pFunc, pArgs);
		if (!pResult)
		{
			PyObject *perror = nullptr;
			perror = PyErr_Occurred();
			PyObject *type, *value, *traceback;
			type = nullptr;
			value = nullptr;
			traceback = nullptr;
			if (perror)
			{

				std::string tracebackfull="fail:";
				#pragma region GetThreadState
				PyThreadState *tstate = PyThreadState_GET();
				if (NULL != tstate )
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
					if (type && (std::string(type->ob_type->tp_name) ==std::string("str")))
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
					tracebackfull +=  " error cause:"+ errorinfo_1 + errorinfo_2 + errorinfo_3;
					std::cerr << tracebackfull;
					return  false;
				}
				#ifdef _DEBUG
					PyErr_Print();
				#endif // DEBUG
			}
		}
		resultrsave = pResult;
		return true;
		//else
		//{	

		//	char *res = NULL;
		//	int retok = PyArg_Parse(pResult, "s", &res); //
		//	Py_XDECREF(pArgs);
		//	Py_XDECREF(pFunc);
		//	//Py_XDECREF(pResult);
		//	Py_XDECREF(pModule);
		//	if (res)
		//	{
		//		std::string ret("Sucees:" + std::string(res));
		//		Py_XDECREF(res);
		//		return ret;
		//	}
		//	else
		//	{
		//		return std::string("fetch result fail");
		//	}
		//	
		//}	
	}
	catch (const std::exception& e)
	{
		std::cerr<<(e.what());
	}
}
int pyCaller::runPyFile(const std::string &pyfilename)
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
void pyCaller::SetPyPath(const std::string &Pypath)
{
	
	std::string Pypathset;
	Pypathset = "sys.path.append('" + Pypath + "')";
	PyRun_SimpleString(Pypathset.c_str());
	return;
}
std::list<std::string> pyCaller::GetPyFileDict(std::string & modulename)
{
	std::list<std::string> ret;
	PyObject* pModule = nullptr;
	pModule = PyImport_ImportModule(modulename.c_str());
	if (pModule == nullptr)
	{
		//DEBUGINFO("import python module:"+ modulename +" failed.");
		ret.push_back(std::string("faile:import python module:" + modulename + " failed."));
		return ret;
	}
	PyObject* pModuledict =  PyModule_GetDict(pModule);
	if (pModuledict == nullptr)
	{
		ret.push_back(std::string("faile:Get  python module:" + modulename + " dict fail."));
	}
	else
	{
		;
	}
	return std::list<std::string>();
}
int pyCaller::importmodule(const std::string& modulename)
{
	PyImport_ImportModule(modulename.c_str());
	return 0;
}
std::string Runfunction(std::string mudulename ,std::string functionname, pyCaller::PYPARAMLIST ParamList)
{
	
	pyCaller a;
	try
	{
		a.runPyFunction(mudulename, functionname, ParamList,nullptr);
	}
	catch (const std::exception&)
	{
		return "Fail";
	}
	return std::string("Success");

}
CALLPYTHON_API bool FinalizePyIter()
{
	if (pyCaller::PyReosureCounter ==0)
	{
		return (Py_FinalizeEx()==0);
	}
	return true;
}
