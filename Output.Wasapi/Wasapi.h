// Output.Wasapi.Wasapi.h

#pragma once

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

class Wasapi : public OutputComponentBase
{
public:
    explicit Wasapi(IUnknown* pUnkOuter);
    ~Wasapi();

    HRESULT __stdcall Notify(IMsgObject* msg_obj) override;
    HRESULT __stdcall Start(LPVOID args = nullptr, IComponent* listener = nullptr) override;
    HRESULT __stdcall Stop(IComponent* listener = nullptr) override;

    HRESULT __stdcall Close(IComponent* listener = nullptr) override;
    HRESULT __stdcall Open(LPCWSTR path, LPCWSTR format_as, IComponent* listener = nullptr) override;
    HRESULT __stdcall QuerySupport(LPCWSTR path, LPCWSTR format_as) override;

    HRESULT __stdcall Write(LPVOID buffer, size_t buf_size, size_t* cb_written, IComponent* listener = nullptr) override;


protected:
    bool IsSupportedExtention(LPCWSTR path) const;
    bool IsSupportedFormat(LPCWSTR format) const;

    HRESULT CloseSync();
    HRESULT CloseAsync(IComponent* listener);
    HRESULT OpenSync();
    HRESULT OpenAsync(IComponent* listener);
    HRESULT WriteSync(LPVOID buffer, size_t buf_size, size_t* cb_written);
    HRESULT WriteAsync(LPVOID buffer, size_t buf_size, IComponent* listener);

private:
    struct Impl;
    Impl* pimpl;
};

//---------------------------------------------------------------------------//

} // namespace CubeMelon

//---------------------------------------------------------------------------//

// Output.Wasapi.Wasapi.h