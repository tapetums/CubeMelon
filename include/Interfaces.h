// Interfaces.h

#pragma once

//---------------------------------------------------------------------------//

static const GUID PKEY_CubeMelon_GetProperty =
{ 0xcfe104fb, 0x5b0a, 0x4e5f, { 0x91, 0x8a, 0xf2, 0xb6, 0x10, 0xa5, 0xb3, 0x9a } };

//!
//! @enum CUBEMELON_PID Interfaces.h
//! @brief プラグインのプロパティを取得するためのプロパティIDです。
//!
enum CUBEMELON_PID : DWORD
{
    CUBEMELON_PID_CLSID       = PID_FIRST_USABLE + 0,
    CUBEMELON_PID_NAME        = PID_FIRST_USABLE + 1,
    CUBEMELON_PID_DESCRIPTION = PID_FIRST_USABLE + 2,
    CUBEMELON_PID_COPYRIGHT   = PID_FIRST_USABLE + 3,
    CUBEMELON_PID_VERSION     = PID_FIRST_USABLE + 4,
};

//!
//! @enum STATE Interfaces.h
//! @brief プラグインの状態を表す定数です。
//!
enum STATE : INT32
{
    STATE_TERMINATING  = -1,
    STATE_IDLE         = 0,
    STATE_RUNNING      = 1,
    STATE_OPEN         = 1 << 1,
};

//---------------------------------------------------------------------------//

//!
//! @enum VersionInfo Interfaces.h
//! @brief プラグインのバージョン情報を格納します。
//!
#pragma pack(push, 1)
struct VersionInfo
{
    UINT16 major;
    UINT16 minor;
    UINT16 revision;
    WCHAR  stage[MAX_PATH];
};
#pragma pack(pop)

//---------------------------------------------------------------------------//

class PluginManager;

static const IID IID_IPlugin =
{ 0xa35dc0c3, 0xac5f, 0x447a, { 0xa4, 0x60, 0x9c, 0x52, 0x17, 0x72, 0x05, 0x7c } };

//!
//! @interface IPlugin Interfaces.h
//! @brief プラグインの基幹インターフェイスです。
//!
class IPlugin : public IUnknown
{
public:
    //!
    //! @brief プラグイン管理オブジェクトを返します。
    //! @param なし
    //! @return プラグイン管理オブジェクトのインスタンス
    //! @retval nullptr 不明なエラー
    //! @note プラグイン自身がインスタンスを保有していない場合、<BR />
    //!     メッセージは親プラグインに転送されます。
    //!
    virtual PluginManager* __stdcall PluginManager() const = 0;

    //!
    //! @brief プラグインのクラスIDを返します。
    //! @param なし
    //! @return プラグインのクラスID
    //! @retval CLSID_NULL 初期化エラー
    //!
    virtual REFCLSID __stdcall ClassID() const = 0;
    //!
    //! @brief このプラグインを所有している親プラグインを返します。
    //! @param なし
    //! @return 親プラグイン
    //! @retval nullptr 親プラグインを持っていない
    //!
    virtual IPlugin* __stdcall Owner()   const = 0;
    //!
    //! @brief プラグインの状態を返します。
    //! @param なし
    //! @return プラグインの状態
    //! @retval STATE_IDLE = 0;         //プラグインは実行待ち状態
    //! @retval STATE_RUNNING = 1;      //プラグインは実行中
    //! @retval STATE_TERMINATING = -1; //プラグインは解放処理中
    //!
    virtual STATE    __stdcall Status()  const = 0;

    //!
    //! @brief プラグインとメッセージを関連付けます。
    //! @param [in] msg 関連付けたいメッセージ
    //! @param [in] listener メッセージの受け取り先プラグイン
    //! @return 無事関連付けられたか
    //! @retval S_OK 関連付け成功
    //! @retval S_FALSE 既に関連付けられている
    //! @retval E_FAIL 関連付け失敗
    //! @attention メッセージのメモリ領域は、呼び出された側でコピーを作成し、保存する必要があります。
    //!
    virtual HRESULT __stdcall Attach(LPCWSTR msg, IPlugin* listener) = 0;
    //!
    //! @brief プラグインとメッセージの関連付けを解除します。
    //! @param [in] msg 関連付けを解除したいメッセージ
    //! @param [in] listener メッセージの受け取り先プラグイン
    //! @return 無事解除できたか
    //! @retval S_OK 解除成功
    //! @retval S_FALSE そもそも登録されていなかった
    //! @retval E_FAIL 解除失敗
    //! @attention メッセージのメモリ領域を解放するのは、呼び出し側の責任です。
    //!
    virtual HRESULT __stdcall Detach(LPCWSTR msg, IPlugin* listener) = 0;
    //!
    //! @brief メッセージを通知します。
    //! @details このメソッド内では、同期または非同期で<BR />
    //!     送信元もしくは別プラグインにメッセージを通知することがあります。
    //! @param [in] sender メッセージの送信元
    //! @param [in] msg 通知するメッセージ
    //! @param [inout] data 同包するデータ
    //! @param [in] cb_data 同包するデータのバイト数
    //! @return 通知に対する応答
    //! @retval S_OK
    //! @retval E_FAIL
    //! @attention メッセージのメモリ領域を解放するのは、呼び出し側の責任です。<BR />
    //!     通知を転送する場合は、転送元でメッセージのメモリ領域をコピーする必要があります。<BR />
    //!     同包のデータに関する扱いは、送受信側で予め定めた約束によって異なります。
    //!
    virtual HRESULT __stdcall Notify(IPlugin* sender, LPCWSTR msg, LPVOID data, size_t cb_data) = 0;
    //!
    //! @brief プラグインが管理している子プラグインを取得します。
    //! @details 指定されたクラスIDのプラグインを所有していない場合、<BR />
    //!     メッセージは親プラグインに転送されます。
    //! @param [in] rclsid 取得したいプラグインのクラスID
    //! @param [in] riid 取得したいプラグインのインターフェイスID
    //! @param [out] ppvObject 取得したプラグインの格納先アドレス
    //! @return 無事取得できたか
    //! @retval S_OK
    //! @retval E_FAIL
    //!
    virtual HRESULT __stdcall GetPluginInstance(REFCLSID rclsid, REFIID riid, void** ppvObject) = 0;
    //!
    //! @brief プラグインの実行を開始します。
    //! @details 開始後、プラグインの状態は STATE_RUNNING になります。
    //! @param [in] args 実行パラメータ
    //! @return 無事開始できたか
    //! @retval S_OK 無事開始できた
    //! @retval S_FALSE 既に実行中である
    //! @retval E_FAIL 実行開始できなかった
    //!
    virtual HRESULT __stdcall Start(LPCVOID args = nullptr) = 0;
    //!
    //! @brief プラグインの実行を停止します。
    //! @details 停止後、プラグインの状態は STATE_IDLE になります。
    //! @param なし
    //! @return 無事停止できたか
    //! @retval S_OK 無事停止できた
    //! @retval S_FALSE 既に停止していた
    //! @retval E_FAIL 停止できなかった
    //!
    virtual HRESULT __stdcall Stop() = 0;
};

//---------------------------------------------------------------------------//

static const IID IID_IUIPlugin =
{ 0xf5dce27e, 0xa45b, 0x45ed, { 0x8f, 0x8e, 0x39, 0xf, 0xe, 0xc7, 0xf0, 0xd2 } };

class IUIPlugin : public IPlugin
{
public:
    virtual size_t __stdcall WindowCount()        const = 0;
    virtual HWND   __stdcall Handle(size_t index) const = 0;
};

//---------------------------------------------------------------------------//

static const IID IID_IIOPlugin=
{ 0xeefb9211, 0xfa4, 0x4c0c, { 0x99, 0xd6, 0xc0, 0xc, 0xc6, 0xc0, 0xe1, 0x1 } };

//!
//! @interface IIOPlugin Interfaces.h
//! @brief 入出力プラグインのためのインターフェイスです。
//!
class IIOPlugin : public IPlugin
{
public:
    //!
    //! @brief 所有しているオブジェクトを閉じます。
    //! @details 無事閉じた場合、プラグインの状態からは STATE_OPEN フラグが取り除かれます。<BR />
    //!     結果は非同期でも受け取ることができます。
    //! @param [in] listener 結果を非同期で受け取る場合の通知先オブジェクト<BR /><BR />
    //!     結果を非同期で受け取る場合、Notifyメッセージのパラメータは以下のようになります。
    //!     @arg [in] <B>sender</B> このオブジェクト
    //!     @arg [in] <B>msg</B> "IIOPlugin.Close()"
    //!     @arg [inout] <B>data</B> S_OK / S_FALSE / E_FAIL etc...
    //!     @arg [in] <B>cb_data</B> sizeof(HRESULT)
    //! @return 無事閉じられたか
    //! @retval S_OK オブジェクトは無事閉じられた
    //! @retval S_OK 非同期処理を開始した
    //! @retval S_FALSE 既に閉じている
    //! @retval E_FAIL オブジェクトを閉じられなかった
    //! @retval E_FAIL 非同期処理を開始できなかった
    //!
    virtual HRESULT __stdcall Close(IPlugin* listener) = 0;
    //!
    //! @brief 所有しているオブジェクトを開きます。
    //! @details 無事開いた場合、プラグインの状態には STATE_OPEN フラグが立ちます。<BR />
    //!     結果は非同期でも受け取ることができます。
    //! @param [in] path 開きたいオブジェクトのフルパス
    //! @param [in] format_as 開きたい形式
    //! @param [in] listener 結果を非同期で受け取る場合の通知先オブジェクト<BR /><BR />
    //!     結果を非同期で受け取る場合、Notifyメッセージのパラメータは以下のようになります。
    //!     @arg [in] <B>sender</B> このオブジェクト
    //!     @arg [in] <B>msg</B> "IIOPlugin.Open()"
    //!     @arg [inout] <B>data</B> S_OK / S_FALSE / E_FAIL etc...
    //!     @arg [in] <B>cb_data</B> sizeof(HRESULT)
    //! @return 無事開けたか
    //! @retval S_OK オブジェクトは無事開くことができた
    //! @retval S_OK 非同期処理を開始した
    //! @retval S_FALSE 既に開いている
    //! @retval E_FAIL オブジェクトを開けなかった
    //! @retval E_FAIL 非同期処理を開始できなかった
    //!
    virtual HRESULT __stdcall Open(LPCWSTR path, LPCWSTR format_as, IPlugin* listener) = 0;
    //!
    //! @brief オブジェクトを指定のフォーマットで開くことができるかを問い合わせます。
    //! @details オブジェクトは開かれません。
    //! @param [in] path 開きたいオブジェクトのフルパス
    //! @param [in] format_as 開きたい形式
    //! @return オブジェクトを開くことができるか
    //! @retval S_OK
    //! @retval E_FAIL
    //!
    virtual HRESULT __stdcall QuerySupport(LPCWSTR path, LPCWSTR format_as) = 0;
    //!
    //! @brief オブジェクトから指定バイト分データを読み込み、ポインタを進めます。
    //! @param [in] buffer データを読み込むバッファ
    //! @param [in] buf_size バッファのサイズ
    //! @param [out] cb_read 実際に読み込んだバイト数
    //! @return 無事読み込めたか
    //! @retval S_OK
    //! @retval E_FAIL
    //!
    virtual HRESULT __stdcall Read(LPVOID buffer, size_t buf_size, size_t* cb_read) = 0;
    //!
    //! @brief オブジェクトのポインタ位置を走査します。
    //! @param [in] offset origin パラメータからの差分
    //! @param [in] origin offset パラメータが基準とする位置
    //!     @arg STREAM_SEEK_SET = 0; // 先頭から
    //!     @arg STREAM_SEEK_CUR = 1; // 現在位置から
    //!     @arg STREAM_SEEK_END = 2; // 終端から
    //! @param [out] nes_pos 走査後のポインタ位置
    //! @details offset に 0、origin に STREAM_SEEK_CUR を指定した場合、<BR />
    //!     new_pos に現在のポインタ位置が返ります。
    //! @return 無事走査できたか
    //! @retval S_OK
    //! @retval E_FAIL
    //!
    virtual HRESULT __stdcall Seek(INT64 offset, DWORD origin, UINT64* new_pos) = 0;
    //!
    //! @brief オブジェクトに指定バイト分データを書き込み、ポインタを進めます。
    //! @param [in] buffer 書き込むデータのバッファ
    //! @param [in] buf_size バッファのサイズ
    //! @param [out] cb_read 実際に書き込んだバイト数
    //! @return 無事書き込めたか
    //! @retval S_OK
    //! @retval E_FAIL
    //!
    virtual HRESULT __stdcall Write(LPCVOID buffer, size_t buf_size, size_t* cb_written) = 0;
};

//---------------------------------------------------------------------------//

// Interfaces.h