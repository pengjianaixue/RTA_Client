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
#include <string>
#include <list>
#include <atomic>
#include <iostream>

#ifdef _DEBUG
#define DEBUGINFO(param)  std::cout<<  param <<std::endl
#else
#define DEBUGINFO(param)  
#endif // _DEBUG

// 此类是从 CallPython.dll 导出的

class CALLPYTHON_API CCallPython 
{
public:
	CCallPython(void);
	CCallPython(std::string PythonPath);
	~CCallPython();
public:
	std::string Runfunction(std::string modulename, std::string functionname,  std::string param_1);
	std::string Runfunction(std::string modulename, std::string functionname, std::string param_1, std::string param_2);
	std::string Runfunction(std::string modulename, std::string functionname, std::string param_1, std::string param_2, std::string param_3);
	std::string Runfunction(std::string modulename, std::string functionname, std::string param_1, std::string param_2, std::string param_3,std::string param_4);
	int  RunPyfile(std::string pyfilename);
	void SetPyPath(std::string Pypath="./");


private:
	std::string m_strpypath;
private:
	int importmodule(std::string modulename);
	// TODO:  在此添加您的方法。
};
//extern CALLPYTHON_API int nCallPython;
//CALLPYTHON_API int fnCallPython(void);
CALLPYTHON_API std::string Runfunction(std::string modulename,std::string functionname ,std::list<std::string > ParamList);
CALLPYTHON_API std::string RunModule(std::string ModuleName, std::list<std::string > ParamList);
CALLPYTHON_API std::string ReleasePy(std::string ModuleName, std::list<std::string > ParamList);

