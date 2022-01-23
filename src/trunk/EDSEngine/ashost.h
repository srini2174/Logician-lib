#ifdef USE_JAVASCRIPT
	#ifdef _MSC_VER
		#ifndef USE_WINDOWS_SCRIPTING

#ifndef __ASHOST_H__
#define __ASHOST_H__

#include <cassert>

class IActiveScriptHost : public IUnknown {

public:

	// IUnknown
	virtual ULONG __stdcall AddRef(void) = 0;
	virtual ULONG __stdcall Release(void) = 0;
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void **obj) = 0;

	// IActiveScriptHost
	virtual HRESULT __stdcall Eval(const WCHAR *source, VARIANT *result) = 0;
	virtual HRESULT __stdcall Run(WCHAR *procname, DISPPARAMS *args, VARIANT *result) = 0;
	virtual HRESULT __stdcall Inject(const WCHAR *name, IUnknown *unkn) = 0;
	virtual HRESULT __stdcall AddScript(const WCHAR *source) = 0;
};

class ScriptHost : 
	public IActiveScriptHost, 
	public IActiveScriptSite {

private:

	LONG _ref;
	IActiveScript *_activeScript;
	IActiveScriptParse *_activeScriptParse;

	ScriptHost(IActiveScript *activeScript, IActiveScriptParse *activeScriptParse)
		: _ref(1),
		_activeScript(activeScript),
		_activeScriptParse(activeScriptParse)
	{
		assert(activeScript != NULL);
		assert(activeScriptParse != NULL);
	}

	virtual ~ScriptHost()
	{
		assert(_ref == 0);
		assert(_activeScript != NULL);
		assert(_activeScriptParse != NULL);

		_activeScript->Release();
		_activeScriptParse->Release();
	}

public:

	// IUnknown
	virtual ULONG __stdcall AddRef(void);
	virtual ULONG __stdcall Release(void);
	virtual HRESULT __stdcall QueryInterface(REFIID iid, void **obj);

	// IActiveScriptSite
	virtual HRESULT __stdcall GetLCID(LCID *lcid);
	virtual HRESULT __stdcall GetItemInfo(LPCOLESTR name,
		DWORD returnMask,
		IUnknown **item,
		ITypeInfo **typeInfo);
	virtual HRESULT __stdcall GetDocVersionString(BSTR *versionString);
	virtual HRESULT __stdcall OnScriptTerminate(const VARIANT *result,
		const EXCEPINFO *exceptionInfo);
	virtual HRESULT __stdcall OnStateChange(SCRIPTSTATE state);
	virtual HRESULT __stdcall OnEnterScript(void);
	virtual HRESULT __stdcall OnLeaveScript(void);
	virtual HRESULT __stdcall OnScriptError(IActiveScriptError *error);

	// IActiveScriptHost
	virtual HRESULT __stdcall Eval(const WCHAR *source, VARIANT *result);
	virtual HRESULT __stdcall Run(WCHAR *procname, DISPPARAMS *args, VARIANT *result);
	virtual HRESULT __stdcall Inject(const WCHAR *name, IUnknown *unkn);
	virtual HRESULT __stdcall AddScript(const WCHAR *source);
public:

	static HRESULT Create(IActiveScriptHost **host)
	{
		HRESULT hr = S_OK;
		IActiveScriptHost *activeScriptHost = NULL;
		IActiveScript *activeScript = NULL;
		IActiveScriptParse *activeScriptParse = NULL;
		IActiveScriptSite *activeScriptSite = NULL;

		try {
			const GUID CLSID_JScript  = {0xf414c260, 0x6ac0, 0x11cf, {0xb6, 0xd1, 0x00, 0xaa, 0x00, 0xbb, 0xbb, 0x58}};
			const GUID CLSID_JScript9 = {0x16d51579, 0xa30b, 0x4c8b, {0xa2, 0x76, 0x0f, 0xf4, 0xdc, 0x41, 0xe7, 0x55 } };
			// Try using JScript9 from IE9
			// {16d51579-a30b-4c8b-a276-0ff4dc41e755}
			hr = CoCreateInstance(CLSID_JScript9,
					NULL,
					CLSCTX_INPROC_SERVER,
					IID_IActiveScript,
					(void **)&activeScript);
			if (FAILED(hr))
			{
				hr = CoCreateInstance(CLSID_JScript,
					NULL,
					CLSCTX_INPROC_SERVER,
					IID_IActiveScript,
					(void **)&activeScript);
				if (FAILED(hr))
					throw;
			}

			hr = activeScript->QueryInterface(IID_IActiveScriptParse, (void **)&activeScriptParse);
			if (FAILED(hr))
				throw;

			activeScriptHost = (IActiveScriptHost *)
				new (std::nothrow) ScriptHost(activeScript,
				activeScriptParse);
			if (activeScriptHost == NULL) {
				hr = E_OUTOFMEMORY;
				throw;
			}

			hr = activeScriptHost->QueryInterface(IID_IActiveScriptSite, (void **)&activeScriptSite);
			if (FAILED(hr))
				throw;

			activeScriptSite = (IActiveScriptSite *) 
				new(std::nothrow) ScriptHost(activeScript, 
				activeScriptParse);
			if (activeScriptSite == NULL) {
				hr = E_OUTOFMEMORY;
				throw;
			}

			hr = activeScript->SetScriptSite(activeScriptSite);
			if (FAILED(hr))
				throw;

			*host = activeScriptHost;

		} catch (...){}

		if (FAILED(hr)) 
		{
			if (activeScriptHost)
				activeScriptHost->Release();
			if (activeScriptSite)
				activeScriptSite->Release();
			if (activeScriptParse)
				activeScriptParse->Release();
			if (activeScript)
				activeScript->Release();
		}



		return hr;
	}

};

#endif // __ASHOST_H__

#endif
#endif
#endif
