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
#include <functional>
#include <iostream>

#ifdef _DEBUG
#define DEBUGINFO(param)  std::cout<<  param <<std::endl
#else
#define DEBUGINFO(param)  
#endif // _DEBUG
extern std::atomic<int> PyReosureCounter;
//template  < typename _T> ;
typedef std::pair<std::string, std::string > PARAMPAIR;
typedef std::list<PARAMPAIR> PARAMLIST;
template< typename Ttype>
class CALLPYTHON_API CCallPython 
{
public:
	CCallPython(void);
	CCallPython(std::string PythonPath);
	~CCallPython();
	/*static std::atomic<int> PyReosureCounter;*/
public:
	Ttype CCallPython<Ttype>::Runfunction(std::string &modulename, std::string &functionname, PARAMLIST &param);
	std::string Runfunction(std::string modulename, std::string functionname, PARAMPAIR param_1, PARAMPAIR param_2);
	std::string Runfunction(std::string modulename, std::string functionname, PARAMPAIR param_1, PARAMPAIR param_2, PARAMPAIR param_3);
	std::string Runfunction(std::string modulename, std::string functionname, PARAMPAIR param_1, PARAMPAIR param_2, PARAMPAIR param_3, 
							PARAMPAIR param_4);
	int  RunPyfile(std::string pyfilename);
	void SetPyPath(std::string Pypath="./");
private:
	std::string m_strpypath;
private:
	int importmodule(std::string modulename);
	
};
// template class instance export
template class CALLPYTHON_API CCallPython<int>;
template class CALLPYTHON_API CCallPython<std::string>;
template class CALLPYTHON_API CCallPython<long long>;
template class CALLPYTHON_API CCallPython<long long>;

//extern CALLPYTHON_API int nCallPython;
//CALLPYTHON_API int fnCallPython(void);
#ifdef __cplusplus
	extern "C" {
#endif // _cplusplus
	
	CALLPYTHON_API std::string Runfunction(std::string mudulename, std::string functionname, PARAMLIST ParamList);
	CALLPYTHON_API bool FinalizePyIter();
#ifdef __cplusplus
			}
#endif // _cplusplus

