// Input.Wave.Wave.h

#pragma once

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

class Wave : public InputComponentBase
{
public:
    explicit Wave(IUnknown* pUnkOuter);
    ~Wave();

    HRESULT __stdcall Notify(IMsgObject* msg_obj) override;
    HRESULT __stdcall Start(LPVOID args = nullptr, IComponent* listener = nullptr) override;
    HRESULT __stdcall Stop(IComponent* listener = nullptr) override;

    HRESULT __stdcall Close(IComponent* listener = nullptr) override;
    HRESULT __stdcall Open(LPCWSTR path, LPCWSTR format_as, IComponent* listener = nullptr) override;
    HRESULT __stdcall QuerySupport(LPCWSTR path, LPCWSTR format_as) override;
    HRESULT __stdcall Seek(INT64 offset, DWORD origin, UINT64* new_pos) override;

    HRESULT __stdcall Read(LPVOID buffer, size_t buf_size, size_t* cb_read, IComponent* listener = nullptr) override;

protected:
    bool IsSupportedExtention(LPCWSTR path) const;
    bool IsSupportedFormat(LPCWSTR format) const;

    HRESULT CloseSync();
    HRESULT OpenSync();
    HRESULT LoadSync();
    HRESULT ReadSync(LPVOID buffer, size_t buf_size, size_t* cb_erad);
    HRESULT ReadAsync(LPVOID buffer, size_t buf_size, IComponent* listener);

private:
    struct Impl;
    Impl* pimpl;
};

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// Input.Wave.Wave.h