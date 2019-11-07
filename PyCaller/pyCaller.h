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

class CALLPYTHON_API pyCaller 
{
public:
	typedef std::pair<std::string, std::string > PYPARAMPAIR;
	typedef std::list<PYPARAMPAIR> PYPARAMLIST;
	typedef	PyObject*	PyResult;
public:
	pyCaller(void);
	pyCaller(std::string PythonPath);
	~pyCaller();
	static std::atomic<int> PyReosureCounter;
public:
	int  runPyFile(const std::string &pyfilename);
	void SetPyPath(const std::string &Pypath="./");
	std::list<std::string> GetPyFileDict(std::string &modulename);
	bool runPyFunction(const std::string &modulename, const std::string &functionname, const PYPARAMLIST &param, PyResult resultrsave);
	template<typename T> 
	bool pyResultCast(T& vaule, PyResult resultToParse);
private:
	std::string m_strpypath;
private:
	int importmodule(const std::string& modulename);
	
};

//extern CALLPYTHON_API int nCallPython;
//CALLPYTHON_API int fnCallPython(void);
#ifdef __cplusplus
	extern "C" {
#endif // _cplusplus
	
	CALLPYTHON_API std::string Runfunction(std::string mudulename, std::string functionname, pyCaller::PYPARAMLIST ParamList);
	CALLPYTHON_API bool FinalizePyIter();
#ifdef __cplusplus
			}
#endif // _cplusplus

	
