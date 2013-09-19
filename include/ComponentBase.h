// ComponentBase.h

#pragma once

//---------------------------------------------------------------------------//

STDAPI DllGetPropManager(size_t index, CubeMelon::IPropManager** pm);

//---------------------------------------------------------------------------//

template <class T> struct ComPtr;

namespace std
{
    template <class T> struct less;
    template <class T> class allocator;
    template <class T1, class T2> struct pair;
    template <class Key, class T, class Compare, class Alloc> class multimap;
    template <class charT> struct char_traits;
    template <class charT, class traits, class Alloc> class basic_string;
}

typedef ComPtr<CubeMelon::IComponent> component_ptr;
typedef std::char_traits<wchar_t> wchar_traits;
typedef std::allocator<wchar_t> wchar_alloc;
typedef std::basic_string<wchar_t, wchar_traits, wchar_alloc> wstring;
typedef std::less<wstring> less_wstring;
typedef std::pair<const wstring, component_ptr> notify_map_pair;
typedef std::allocator<notify_map_pair> notify_map_pair_alloc;
typedef std::multimap<wstring, component_ptr, less_wstring, notify_map_pair_alloc> NotifyMap;

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

extern const CLSID    CLSID_Component;
extern const size_t   COMP_INDEX;
extern const wchar_t* COMP_NAME;
extern const wchar_t* COMP_BASE;

//---------------------------------------------------------------------------//

class ComponentBase : public IComponent
{
public:
    explicit ComponentBase(IUnknown* pUnkOuter);
    virtual ~ComponentBase();

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

    REFCLSID      __stdcall ClassID() const override;
    LPCWSTR       __stdcall Name() const override;
    IComponent*   __stdcall Owner() const override;
    IPropManager* __stdcall PropManager() const override;
    STATE         __stdcall Status() const override;

    HRESULT __stdcall Attach(LPCWSTR msg, IComponent* listener) override;
    HRESULT __stdcall Detach(LPCWSTR msg, IComponent* listener) override;
    HRESULT __stdcall GetInstance(REFCLSID rclsid, REFIID riid, void** ppvObject) override;
    HRESULT __stdcall Notify(IMsgObject* msg_obj) override;
    HRESULT __stdcall Start(LPVOID args = nullptr, IComponent* listener = nullptr) override;
    HRESULT __stdcall Stop(IComponent* listener = nullptr) override;

protected:
    ULONG         m_cRef;
    STATE         m_state;
    IComponent*   m_owner;
    NotifyMap*    m_notify_map;
    IPropManager* m_prop_mgr;

private:
    ComponentBase(const ComponentBase&);
    ComponentBase(ComponentBase&&);
    ComponentBase& operator =(const ComponentBase&);
    ComponentBase& operator =(ComponentBase&&);
};

//---------------------------------------------------------------------------//

class UIComponentBase : public IUIComponent
{
public:
    explicit UIComponentBase(IUnknown* pUnkOuter);
    virtual ~UIComponentBase();

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

    REFCLSID      __stdcall ClassID() const override;
    LPCWSTR       __stdcall Name() const override;
    IComponent*   __stdcall Owner() const override;
    IPropManager* __stdcall PropManager() const override;
    STATE         __stdcall Status() const override;

    HRESULT __stdcall Attach(LPCWSTR msg, IComponent* listener) override;
    HRESULT __stdcall Detach(LPCWSTR msg, IComponent* listener) override;
    HRESULT __stdcall GetInstance(REFCLSID rclsid, REFIID riid, void** ppvObject) override;
    HRESULT __stdcall Notify(IMsgObject* msg_obj) override;
    HRESULT __stdcall Start(LPVOID args = nullptr, IComponent* listener = nullptr) override;
    HRESULT __stdcall Stop(IComponent* listener = nullptr) override;

    size_t __stdcall WindowCount() const override;
    HWND   __stdcall Handle(size_t index) const override;

protected:
    ULONG         m_cRef;
    STATE         m_state;
    IComponent*   m_owner;
    NotifyMap*    m_notify_map;
    IPropManager* m_prop_mgr;
    HWND          m_hwnd;

private:
    UIComponentBase(const UIComponentBase&);
    UIComponentBase(UIComponentBase&&);
    UIComponentBase& operator =(const UIComponentBase&);
    UIComponentBase& operator =(UIComponentBase&&);
};

//---------------------------------------------------------------------------//

class InputComponentBase : public IInputComponent
{
public:
    explicit InputComponentBase(IUnknown* pUnkOuter);
    virtual ~InputComponentBase();

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

    REFCLSID      __stdcall ClassID() const override;
    LPCWSTR       __stdcall Name() const override;
    IComponent*   __stdcall Owner() const override;
    IPropManager* __stdcall PropManager() const override;
    STATE         __stdcall Status() const override;

    HRESULT __stdcall Attach(LPCWSTR msg, IComponent* listener) override;
    HRESULT __stdcall Detach(LPCWSTR msg, IComponent* listener) override;
    HRESULT __stdcall GetInstance(REFCLSID rclsid, REFIID riid, void** ppvObject) override;
    HRESULT __stdcall Notify(IMsgObject* msg_obj) override;
    HRESULT __stdcall Start(LPVOID args = nullptr, IComponent* listener = nullptr) override;
    HRESULT __stdcall Stop(IComponent* listener = nullptr) override;

    HRESULT __stdcall Close(IComponent* listener = nullptr) override;
    HRESULT __stdcall Open(LPCWSTR path, LPCWSTR format_as, IComponent* listener = nullptr) override;
    HRESULT __stdcall QuerySupport(LPCWSTR path, LPCWSTR format_as) override;
    HRESULT __stdcall Seek(INT64 offset, DWORD origin, UINT64* new_pos) override;

    HRESULT __stdcall Read(LPVOID buffer, size_t buf_size, size_t* cb_read, IComponent* listener = nullptr) override;

protected:
    ULONG         m_cRef;
    STATE         m_state;
    IComponent*   m_owner;
    NotifyMap*    m_notify_map;
    IPropManager* m_prop_mgr;
    UINT64        m_position;
    UINT64        m_size;
    WCHAR         m_path[MAX_PATH];

private:
    InputComponentBase(const InputComponentBase&);
    InputComponentBase(InputComponentBase&&);
    InputComponentBase& operator =(const InputComponentBase&);
    InputComponentBase& operator =(InputComponentBase&&);
};

//---------------------------------------------------------------------------//

class OutputComponentBase : public IOutputComponent
{
public:
    explicit OutputComponentBase(IUnknown* pUnkOuter);
    virtual ~OutputComponentBase();

    HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
    ULONG   __stdcall AddRef() override;
    ULONG   __stdcall Release() override;

    REFCLSID      __stdcall ClassID() const override;
    LPCWSTR       __stdcall Name() const override;
    IComponent*   __stdcall Owner() const override;
    IPropManager* __stdcall PropManager() const override;
    STATE         __stdcall Status() const override;

    HRESULT __stdcall Attach(LPCWSTR msg, IComponent* listener) override;
    HRESULT __stdcall Detach(LPCWSTR msg, IComponent* listener) override;
    HRESULT __stdcall GetInstance(REFCLSID rclsid, REFIID riid, void** ppvObject) override;
    HRESULT __stdcall Notify(IMsgObject* msg_obj) override;
    HRESULT __stdcall Start(LPVOID args = nullptr, IComponent* listener = nullptr) override;
    HRESULT __stdcall Stop(IComponent* listener = nullptr) override;

    HRESULT __stdcall Close(IComponent* listener = nullptr) override;
    HRESULT __stdcall Open(LPCWSTR path, LPCWSTR format_as, IComponent* listener = nullptr) override;
    HRESULT __stdcall QuerySupport(LPCWSTR path, LPCWSTR format_as) override;
    HRESULT __stdcall Seek(INT64 offset, DWORD origin, UINT64* new_pos) override;

    HRESULT __stdcall Write(LPVOID buffer, size_t buf_size, size_t* cb_written, IComponent* listener = nullptr) override;

protected:
    ULONG         m_cRef;
    STATE         m_state;
    IComponent*   m_owner;
    NotifyMap*    m_notify_map;
    IPropManager* m_prop_mgr;
    UINT64        m_position;
    UINT64        m_size;
    WCHAR         m_path[MAX_PATH];

private:
    OutputComponentBase(const OutputComponentBase&);
    OutputComponentBase(OutputComponentBase&&);
    OutputComponentBase& operator =(const OutputComponentBase&);
    OutputComponentBase& operator =(OutputComponentBase&&);
};

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// ComponentBase.h