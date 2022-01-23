#ifdef USE_JAVASCRIPT
	#ifdef _MSC_VER
		#ifndef USE_WINDOWS_SCRIPTING
#include "stdafx.h"

#include <activscp.h>
#include <iostream>
#include <string>
#include <cassert>
#include "ashost.h"
#include <map>

using namespace std;

// ============================================================================
// Map to store the injectabile variables
// ============================================================================
map<wstring, IUnknown*> rootList;
typedef map<wstring, IUnknown*>::iterator MapIter;
typedef pair<wstring, IUnknown*> InjectPair;

#pragma region IUnknown implementation
ULONG __stdcall ScriptHost::AddRef(void)
{
    return (ULONG)InterlockedIncrement(&_ref);
}

ULONG __stdcall ScriptHost::Release(void)
{
    LONG ref = InterlockedDecrement(&_ref);
    if (ref == 0)
        delete this;

    return ref;
}

HRESULT __stdcall ScriptHost::QueryInterface(REFIID iid, void **obj)
{
    *obj = nullptr;
    if (IsEqualIID(iid, IID_IUnknown) == TRUE)
        *obj = (IUnknown *)((IActiveScriptHost *)this);
    else if (IsEqualIID(iid, IID_IActiveScriptSite) == TRUE)
        *obj = (IActiveScriptSite *)this;
    else
        return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

#pragma endregion

#pragma region TActoveScriptHost Impl (Ignore)
HRESULT __stdcall ScriptHost::GetLCID(LCID *lcid)
{
    return E_NOTIMPL;
}

HRESULT __stdcall ScriptHost::GetDocVersionString(BSTR *versionString)
{
    return E_NOTIMPL;
}

HRESULT __stdcall ScriptHost::OnScriptTerminate(const VARIANT *result,
                                                       const EXCEPINFO *exceptionInfo)
{
    return E_NOTIMPL;
}

HRESULT __stdcall ScriptHost::OnStateChange(SCRIPTSTATE state)
{
    return E_NOTIMPL;
}

HRESULT __stdcall ScriptHost::OnEnterScript(void)
{
    return E_NOTIMPL;
}

HRESULT __stdcall ScriptHost::OnLeaveScript(void)
{
    return E_NOTIMPL;
}

HRESULT __stdcall ScriptHost::OnScriptError(IActiveScriptError *error)
{
    using namespace std;
    wcout << L"ERROR" << endl;

    return S_OK;
}

#pragma endregion

#pragma region ScriptHost Impl (Important)
// ============================================================================
// This is called for each named item. Look into the map and return
// ============================================================================
HRESULT __stdcall ScriptHost::GetItemInfo(LPCOLESTR name,
                                                 DWORD returnMask,
                                                 IUnknown **item,
                                                 ITypeInfo **typeInfo)
{
    MapIter iter = rootList.find(name);
    if (iter != rootList.end())
    {
        *item = (*iter).second;
        return S_OK;
    }
    else
        return E_NOTIMPL;
}

// ============================================================================
// Inject new objects into the script namespace
// ============================================================================
HRESULT __stdcall ScriptHost::Inject(const WCHAR *name, IUnknown *unkn)
{
    assert(name != nullptr);

    if (name == nullptr)
        return E_POINTER;

    _activeScript->AddNamedItem(name, SCRIPTITEM_GLOBALMEMBERS | SCRIPTITEM_ISVISIBLE );
    rootList.insert(InjectPair(std::wstring(name), unkn));

    return S_OK;
}

// ============================================================================
// Evaluation routine.
// ============================================================================
HRESULT __stdcall ScriptHost::Eval(const WCHAR *source, VARIANT *result)
{
    assert(source != nullptr);

    if (source == nullptr)
        return E_POINTER;

    return _activeScriptParse->ParseScriptText(source, nullptr, nullptr, nullptr, 0, 0, SCRIPTTEXT_ISEXPRESSION, result, NULL);
}

HRESULT __stdcall ScriptHost::Run(WCHAR *procname, DISPPARAMS *args, VARIANT *results)
{
    assert(procname != nullptr);

    if (procname == nullptr)
        return E_POINTER;

	IDispatch *disp = nullptr;
	_activeScript->GetScriptDispatch(nullptr, &disp);
	DISPID dispid = 0;
	disp->GetIDsOfNames(IID_NULL, &procname, 1, LOCALE_SYSTEM_DEFAULT, &dispid);
	EXCEPINFO info;
	UINT argErr;
	args->rgdispidNamedArgs = &dispid;
	HRESULT hr = disp->Invoke(dispid, IID_NULL, NULL, DISPATCH_METHOD, args, results, &info, &argErr);
	return hr;
}

HRESULT __stdcall ScriptHost::AddScript(const WCHAR *source)
{
	assert(source != nullptr);

    if (source == nullptr)
        return E_POINTER;

	VARIANT vRes;
	HRESULT hr = _activeScriptParse->ParseScriptText(source, nullptr, nullptr, nullptr, 0, 0, NULL, &vRes, nullptr);
	return hr;
}
#endif
#endif
#endif
