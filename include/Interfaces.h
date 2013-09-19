// Interfaces.h

#pragma once

//---------------------------------------------------------------------------//

struct IUnknown;

//---------------------------------------------------------------------------//

namespace CubeMelon {

//---------------------------------------------------------------------------//

struct VersionInfo;

struct IMsgObject;
struct IProperty;
struct IPropManager;
struct ICompAdapter;
struct ICompManager;
struct IComponent;

//---------------------------------------------------------------------------//

static const wchar_t* MSG_COMP_ATTACH       = TEXT("Comp.Attach");
static const wchar_t* MSG_COMP_DETACH       = TEXT("Comp.Detach");

static const wchar_t* MSG_COMP_START_DONE   = TEXT("Comp.Start.Done");
static const wchar_t* MSG_COMP_START_FAILED = TEXT("Comp.Start.Failed");
static const wchar_t* MSG_COMP_STOP_DONE    = TEXT("Comp.Stop.Done");
static const wchar_t* MSG_COMP_STOP_FAILED  = TEXT("Comp.Stop.Failed");

static const wchar_t* MSG_IO_CLOSE_DONE     = TEXT("IO.Close.Done");
static const wchar_t* MSG_IO_CLOSE_FAILED   = TEXT("IO.Close.Failed");
static const wchar_t* MSG_IO_OPEN_DONE      = TEXT("IO.Open.Done");
static const wchar_t* MSG_IO_OPEN_FAILED    = TEXT("IO.Open.Failed");
static const wchar_t* MSG_IO_READ_DONE      = TEXT("IO.Read.Done");
static const wchar_t* MSG_IO_READ_FAILED    = TEXT("IO.Read.Failed");
static const wchar_t* MSG_IO_WRITE_DONE     = TEXT("IO.Write.Done");
static const wchar_t* MSG_IO_WRITE_FAILED   = TEXT("IO.Write.Failed");

static const HRESULT S_OK_ASYNC  = 0x00000002;
static const HRESULT E_COMP_BUSY = 0x8FFFFFFF;

//---------------------------------------------------------------------------//

//!
//! @enum STATE Interfaces.h
//! @brief コンポーネントの状態を表す定数です。
//!
enum STATE : INT32
{
    STATE_TERMINATING  = -1,
    STATE_IDLE         = 0,
    STATE_ACTIVE       = 1,
    STATE_OPEN         = 1 << 1,
    STATE_STARTING     = 1 << 2,
    STATE_STOPPING     = 1 << 3,
    STATE_CLOSING      = 1 << 4,
    STATE_OPENING      = 1 << 5,
    STATE_SEEKING      = 1 << 6,
    STATE_READING      = 1 << 7,
    STATE_WRITING      = 1 << 8,
};

//!
//! @enum DATATYPE Interfaces.h
//! @brief データの種類を表す定数です。
//!
enum DATATYPE : UINT32
{
    TYPE_VOID = 0,
    TYPE_BOOL,
    TYPE_SINT8,
    TYPE_UINT8,
    TYPE_SINT16,
    TYPE_UINT16,
    TYPE_SINT32,
    TYPE_UINT32,
    TYPE_SINT64,
    TYPE_UINT64,
    TYPE_FLOAT16,
    TYPE_FLOAT32,
    TYPE_FLOAT64,
    TYPE_HRESULT,
    TYPE_SIZE,
    TYPE_PTR,
    TYPE_DBL_PTR,
    TYPE_BYTE_PTR,
    TYPE_STRING,
    TYPE_WSTRING,
    TYPE_HANDLE,
    TYPE_HWND,
    TYPE_VERSIONINFO,
    TYPE_IUNKNOWN,
    TYPE_IMSGOBJ,
    TYPE_IPROPERTY,
    TYPE_ICOMPONENT,
    TYPE_GUID,
    TYPE_IID,
    TYPE_CLSID,
};

//---------------------------------------------------------------------------//

//!
//! @union VARIANT Interfaces.h
//! @brief 様々な種類のデータを格納するための共用体です。
//! @detail 現在のサイズは sizeof(GUID) == 16
//!
union VARIANT
{
    bool             int1;
    signed   __int8  sint8;
    unsigned __int8  uint8;
    signed   __int16 sint16;
    unsigned __int16 uint16;
    signed   __int32 sint32;
    unsigned __int32 uint32;
    signed   __int64 sint64;
    unsigned __int64 uint64;
    unsigned __int16 float16;
    float            float32;
    double           float64;
    HRESULT          hresult;
    DATATYPE         type;
    size_t           size;
    void*            ptr;
    void**           pptr;
    unsigned __int8* data;
    const char*      string;
    const wchar_t*   wstring;
    HANDLE           handle;
    HWND             hwnd;
    VersionInfo*     version;
    IUnknown*        unknown;
    IMsgObject*      msg_obj;
    IProperty*       property;
    IComponent*      component;
    GUID             guid;
    IID              iid;
    CLSID            clsid;
};

//---------------------------------------------------------------------------//

//!
//! @struct VersionInfo Interfaces.h
//! @brief コンポーネントのバージョン情報を格納します。
//!
#pragma pack(push, 1)
struct VersionInfo
{
    //! @brief メジャーバージョン
    UINT16 major;
    //! @brief マイナーバージョン
    UINT16 minor;
    //! @brief リビジョン
    UINT16 revision;
    //! @brief 補足説明 (alpha, beta, RTM など)
    UINT16 stage;
};
#pragma pack(pop)

//---------------------------------------------------------------------------//

static const IID IID_IMsgObject =
{ 0xc7db12f7, 0x2cd4, 0x4820, { 0x9f, 0xb5, 0x73, 0x18, 0x5, 0x8f, 0x26, 0xb6 } };

//!
//! @interface IMsgObject Interfaces.h
//! @brief コンポーネント間での通信に使うメッセージオブジェクトのインターフェイスです。
//!
struct IMsgObject : public IUnknown
{
    //!
    //! @brief このメッセージオブジェクトの名前を返します。
    //! @param なし
    //! @return メッセージオブジェクトの名前
    virtual LPCWSTR     __stdcall Name()                     const = 0;
    //!
    //!
    //! @brief このメッセージの文字情報を返します。
    //! @param なし
    //! @return メッセージの文字情報
    virtual LPCWSTR     __stdcall Message()                  const = 0;
    //!
    //!
    //! @brief このメッセージの送り主を返します。
    //! @param なし
    //! @return メッセージの送り主
    virtual IComponent* __stdcall Sender()                   const = 0;
    //!
    //!
    //! @brief このメッセージの送り先を返します。
    //! @param なし
    //! @return メッセージの送り先
    virtual IComponent* __stdcall Listener()                 const = 0;
    //!
    //!
    //! @brief このメッセージに同梱されるデータの数を返します。
    //! @param なし
    //! @return データの数
    virtual size_t      __stdcall DataCount()                const = 0;
    //!
    //!
    //! @brief このメッセージに同梱されるデータの名前を返します。
    //! @param [in] index データのインデックス
    //! @return データの名前
    virtual LPCWSTR     __stdcall DataName(size_t index = 0) const = 0;
    //!
    //!
    //! @brief このメッセージに同梱されるデータの大きさをバイト数で返します。
    //! @param [in] index データのインデックス
    //! @return データの大きさ
    virtual size_t      __stdcall DataSize(size_t index = 0) const = 0;
    //!
    //!
    //! @brief このメッセージに同梱されるデータの種類を返します。
    //! @param [in] index データのインデックス
    //! @return データの種類
    virtual DATATYPE    __stdcall DataType(size_t index = 0) const = 0;
    //!
    //!
    //! @brief このメッセージに同梱されるデータを返します。
    //! @param [in] index データのインデックス
    //! @return メッセージに同梱されるデータ
    virtual LPVOID      __stdcall Data(size_t index = 0)     const = 0;
};

//---------------------------------------------------------------------------//

static const IID IID_IProperty =
{ 0x4fe37727, 0x3644, 0x43bf, { 0x9f, 0xea, 0x3e, 0xd2, 0x48, 0x35, 0x3d, 0xc5 } };

//!
//! @interface IProperty Interfaces.h
//! @brief 様々な種類のデータを格納可能なオブジェクトのインターフェイスです。
//!
struct IProperty : public IUnknown
{
    //!
    //! @brief データを保持している共用体を返します。
    //! @param なし
    //! @return データを保持している共用体
    //! @attention データのメモリ領域を解放するには、<BR />
    //!            このオブジェクトの Release() メソッドを使用します。
    virtual VARIANT*  __stdcall Data() const = 0;
    //!
    //!
    //! @brief このデータの名前を返します。
    //! @param なし
    //! @return データの名前
    virtual LPCWSTR   __stdcall Name() const = 0;
    //!
    //!
    //! @brief このデータの大きさをバイト数で返します。
    //! @param なし
    //! @return データの大きさ
    virtual size_t    __stdcall Size() const = 0;
    //!
    //!
    //! @brief このデータの種類を表す列挙体を返します。
    //! @param なし
    //! @return データの種類
    virtual DATATYPE  __stdcall Type() const = 0;
};

//---------------------------------------------------------------------------//

static const IID IID_IPropManager =
{ 0x704e6fe9, 0xb308, 0x4bdf, { 0xb4, 0xa1, 0xd6, 0xaf, 0x95, 0xeb, 0x60, 0xd5 } };

//!
//! @interface IPropManager Interfaces.h
//! @brief コンポーネントのプロパティ情報を管理するオブジェクトのインターフェイスです。
//!
struct IPropManager : public IUnknown
{
    //!
    //! @brief 管理しているプロパティの数を返します。
    //! @param なし
    //! @return 管理しているプロパティの数
    virtual size_t __stdcall PropCount() const = 0;

    //!
    //! @brief 任意のプロパティを通し番号で取得します。
    //! @param [in] name プロパティのインデックス
    //! @param [out] prop プロパティ情報を保持するオブジェクト
    //! @return 取得に成功したかどうか
    //! @retval S_OK
    //! @retval E_FAIL
    virtual HRESULT __stdcall GetAt(size_t index, IProperty** prop) = 0;
    //!
    //!
    //! @brief 任意のプロパティを名前で取得します。
    //! @param [in] name プロパティの名前
    //! @param [out] prop プロパティ情報を保持するオブジェクト
    //! @return 取得に成功したかどうか
    //! @retval S_OK
    //! @retval E_FAIL
    virtual HRESULT __stdcall GetByName(LPCWSTR name, IProperty** prop) = 0;
    //!
    //!
    //! @brief 指定した通し番号のプロパティを変更します。
    //! @param [in] name プロパティのインデックス
    //! @param [in] prop プロパティ情報を保持するオブジェクト
    //! @return 設定に成功したかどうか
    //! @retval S_OK
    //! @retval E_FAIL
    virtual HRESULT __stdcall SetAt(size_t index, IProperty* prop) = 0;
    //!
    //!
    //! @brief 指定した名前のプロパティを変更します。
    //! @param [in] name プロパティの名前
    //! @param [in] prop プロパティ情報を保持するオブジェクト
    //! @return 設定に成功したかどうか
    //! @retval S_OK
    //! @retval E_FAIL
    virtual HRESULT __stdcall SetByName(LPCWSTR name, IProperty* prop) = 0;
};

//---------------------------------------------------------------------------//

static const IID IID_ICompAdapter =
{ 0x6036103d, 0xe3bd, 0x46ba, { 0xb9, 0xa8, 0x3f, 0xfd, 0xfd, 0x68, 0xd7, 0x23 } };

//!
//! @interface ICompAdapter Interfaces.h
//! @brief コンポーネントが格納されたDLLファイルを透過的に扱うためのオブジェクトのインターフェイスです。
//!
struct ICompAdapter : public IUnknown
{
    //!
    //! @brief このコンポーネントのクラスIDを返します。
    //! @param なし
    //! @return コンポーネントのクラスID
    virtual REFCLSID      __stdcall ClassID()     const = 0;
    //!
    //!
    //! @brief このコンポーネントの著作権情報を返します。
    //! @param なし
    //! @return コンポーネントの著作権情報
    virtual LPCWSTR       __stdcall Copyright()   const = 0;
    //!
    //!
    //! @brief このコンポーネントの詳細説明を返します。
    //! @param なし
    //! @return コンポーネントの詳細説明
    virtual LPCWSTR       __stdcall Description() const = 0;
    //!
    //!
    //! @brief このコンポーネントが格納されているDLLファイルのフルパスを返します。
    //! @param なし
    //! @return DLLファイルのフルパス
    virtual LPCWSTR       __stdcall FilePath()    const = 0;
    //!
    //!
    //! @brief このコンポーネントのDLLファイル中におけるインデックスを返します。
    //! @param なし
    //! @return コンポーネントのインデックス
    virtual size_t        __stdcall Index()       const = 0;
    //!
    //!
    //! @brief このコンポーネントのプロパティ管理オブジェクトを返します。
    //! @param なし
    //! @return コンポーネントのプロパティ管理オブジェクト
    virtual IPropManager* __stdcall PropManager() const = 0;
    //!
    //!
    //! @brief このコンポーネントの名前を返します。
    //! @param なし
    //! @return コンポーネントの名前
    virtual LPCWSTR       __stdcall Name()        const = 0;
    //!
    //!
    //! @brief このコンポーネントのバージョン情報を返します。
    //! @param なし
    //! @return コンポーネントのバージョン情報
    virtual VersionInfo*  __stdcall Version()     const = 0;

    //!
    //! @brief コンポーネントが格納されているDLLファイルをプロセスに読み込みます。
    //! @param file_path DLLファイルのパス
    //! @param index 読み込みたいコンポーネントのDLLファイル中におけるインデックス
    //! @return 無事読み込めたか
    //! @retval S_OK
    //! @retval E_FAIL
    virtual HRESULT __stdcall Load(LPCWSTR file_path, size_t index) = 0;
    //!
    //!
    //! @brief コンポーネントが格納されているDLLファイルをプロセスから解放します。
    //! @param なし
    //! @return 無事解放できたか
    //! @retval S_OK
    //! @retval E_FAIL
    virtual HRESULT __stdcall Free() = 0;
    //!
    //!
    //! @brief このコンポーネントの設定画面を呼び出します。
    //! @param [in] hwndParent 親ウィンドウのハンドル
    //! @return 無事画面を呼び出せたか
    //! @retval S_OK 無事呼び出せた
    //! @retval S_FALSE 既に表示されている
    //! @retval E_FAIL 呼び出せなかった
    virtual HRESULT __stdcall Configure(HWND hwndParent = nullptr) = 0;
    //!
    //!
    //! @brief コンポーネントのインスタンスを生成します。
    //! @param [in] owner 親コンポーネント
    //! @param [in] riid 取得したいインターフェイス
    //! @param [out] ppvObject インスタンスの格納先アドレス
    //! @return 無事生成できたか
    //! @retval S_OK
    //! @retval E_FAIL
    virtual HRESULT __stdcall CreateInstance(IComponent* owner, REFIID riid, void** ppvObject) = 0;
};

//---------------------------------------------------------------------------//

static const IID IID_ICompManager =
{ 0x45e0aaf1, 0x3e4e, 0x4e6e, { 0x92, 0x5b, 0x92, 0x9e, 0xc1, 0xa5, 0x63, 0xf7 } };

//!
//! @interface ICompManager Interfaces.h
//! @brief コンポーネント管理オブジェクトのインターフェイスです。
//!
struct ICompManager : public IUnknown
{
    //!
    //! @brief このオブジェクトが管理しているコンポーネントの数を取得します。
    //! @param なし
    //! @return コンポーネントの数
    virtual size_t        __stdcall ComponentCount()      const = 0;
    //!
    //!
    //! @brief このオブジェクトが管理しているフォルダのフルパスを返します。
    //! @param なし
    //! @return フォルダのフルパス
    virtual LPCWSTR       __stdcall DirectoryPath()       const = 0;
    //!
    //!
    //! @brief コンポーネント格納オブジェクトを取得します。
    //! @param [in] index 取得したいコンポーネントのDLLファイル中におけるインデックス
    //! @return コンポーネント格納オブジェクト
    //! @retval nullptr 無効なインデックス値
    virtual ICompAdapter* __stdcall GetAt(size_t index)   const = 0;
    //!
    //!
    //! @brief コンポーネント格納オブジェクトを取得します。
    //! @param [in] rclsid コンポーネントのクラスID
    //! @return コンポーネント格納オブジェクト
    //! @retval nullptr 無効なクラスID
    virtual ICompAdapter* __stdcall Find(REFCLSID rclsid) const = 0;

    //!
    //! @brief 全てのコンポーネントを読み込みます。
    //! @param [in] dir_path 読み込みたいフォルダのパス
    //! @return 無事読み込めたか
    //! @retval S_OK 読み込めた
    //! @retval S_FALSE コンポーネントは一つも見つからなかった
    //! @retval E_ABORT 読み込みがキャンセルされた
    //! @retval E_FAIL 読み込み中にエラーが発生した
    virtual HRESULT __stdcall LoadAll(LPCWSTR dir_path) = 0;
    //!
    //!
    //! @brief 全てのコンポーネントを解放します。
    //! @param なし
    //! @return 無事解放できたか
    //! @retval S_OK 全て解放できた
    //! @retval S_FALSE 一部は解放できなかった
    //! @retval E_FAIL 一つも解放できなかった
    virtual HRESULT __stdcall FreeAll() = 0;
};

//---------------------------------------------------------------------------//

static const IID IID_IComponent =
{ 0xa35dc0c3, 0xac5f, 0x447a, { 0xa4, 0x60, 0x9c, 0x52, 0x17, 0x72, 0x05, 0x7c } };

//!
//! @interface IComponent Interfaces.h
//! @brief コンポーネントの基幹インターフェイスです。
//!
struct IComponent : public IUnknown
{
    //!
    //! @brief このオブジェクトのクラスIDを返します。
    //! @param なし
    //! @return コンポーネントのクラスID
    //! @retval CLSID_NULL 初期化エラー
    virtual REFCLSID      __stdcall ClassID()     const = 0;
    //!
    //!
    //! @brief このオブジェクトの名前を返します。
    //! @param なし
    //! @return コンポーネントの名前
    virtual LPCWSTR       __stdcall Name()        const = 0;
    //!
    //!
    //! @brief このオブジェクトを所有している親オブジェクトを返します。
    //! @param なし
    //! @return 親オブジェクト
    //! @retval nullptr 親オブジェクトを持っていない
    virtual IComponent*   __stdcall Owner()       const = 0;
    //!
    //!
    //! @brief このオブジェクトのプロパティ管理オブジェクトを返します。
    //! @param なし
    //! @return プロパティ管理オブジェクト
    virtual IPropManager* __stdcall PropManager() const = 0;
    //!
    //!
    //! @brief このオブジェクトの状態を返します。
    //! @param なし
    //! @return オブジェクトの状態
    //! @retval STATE_IDLE = 0;         // オブジェクトはアイドル状態
    //! @retval STATE_ACTIVE = 1;       // オブジェクトは実行中
    //! @retval STATE_TERMINATING = -1; // オブジェクトは解放処理中
    //! @retval 他、実装による
    virtual STATE         __stdcall Status()      const = 0;

    //!
    //! @brief このオブジェクトとメッセージを関連付けます。
    //! @param [in] msg 関連付けたいメッセージ
    //! @param [in] listener メッセージの受け取り先コンポーネントオブジェクト
    //! @return 無事関連付けられたか
    //! @retval S_OK 関連付け成功
    //! @retval S_FALSE 既に関連付けられている
    //! @retval E_FAIL 関連付け失敗
    //! @attention 引数に指定されたメッセージの文字列は、<BR />
    //!            呼び出された側でコピーを作成し、保存する必要があります。
    virtual HRESULT __stdcall Attach(LPCWSTR msg, IComponent* listener) = 0;
    //!
    //!
    //! @brief このオブジェクトとメッセージとの関連付けを解除します。
    //! @param [in] msg 関連付けを解除したいメッセージ
    //! @param [in] listener メッセージの受け取り先コンポーネントオブジェクト
    //! @return 無事解除できたか
    //! @retval S_OK 解除成功
    //! @retval S_FALSE そもそも登録されていなかった
    //! @retval E_FAIL 解除失敗
    //! @attention 引数に指定されたメッセージのメモリ領域を解放するのは、<BR />
    //!            呼び出された側ではなく、呼び出した側の責任です。
    virtual HRESULT __stdcall Detach(LPCWSTR msg, IComponent* listener) = 0;
    //!
    //!
    //! @brief このオブジェクトが管理しているオブジェクトを取得します。
    //! @details 指定されたクラスIDのオブジェクトを所有していない場合、<BR />
    //!          呼び出された側は <B>親オブジェクトの GetInstance() メソッドを呼び出します。</B>
    //! @param [in] rclsid 取得したいオブジェクトのクラスID
    //! @param [in] riid 取得したいオブジェクトのインターフェイスID
    //! @param [out] ppvObject 取得するオブジェクトの格納先アドレス
    //! @return 無事取得できたか
    //! @retval S_OK
    //! @retval E_FAIL
    virtual HRESULT __stdcall GetInstance(REFCLSID rclsid, REFIID riid, void** ppvObject) = 0;
    //!
    //! @brief メッセージを通知します。
    //! @details このメソッド内では、さらに別のコンポーネントオブジェクトに<BR />
    //!          Notify() メソッドを通じてメッセージを再送することがあります。<BR />
    //!          呼び出し先でのメッセージの再送は <B>同期と非同期 両方</B>の場合があります。
    //! @param [in] msg_obj メッセージオブジェクト
    //! @return 通知に対する応答
    //! @retval S_OK メッセージは適切に処理された
    //! @retval S_OK_ASYNC メッセージは非同期的に別のコンポーネントオブジェクトに再送された
    //! @retval S_FALSE メッセージは受け取ったが、何もしなかった
    //! @retval E_FAIL メッセージを適切に処理できなかった
    //! @attention 呼び出し側は、通知する前に<BR />
    //!            メッセージオブジェクトの AddRef() メソッドを呼び出必要があります。<BR />
    //!            また呼び出された側は、このメソッド内で必ず<BR />
    //!            メッセージオブジェクトの Release() メソッドを呼び出さなければなりません。
    virtual HRESULT __stdcall Notify(IMsgObject* msg_obj) = 0;
    //!
    //!
    //! @brief このオブジェクトの実行を開始します。
    //! @details 開始後、このオブジェクトの状態には STATE_ACTIVE フラグが立ちます。
    //! @details listener に nullptr 以外を渡した場合、<BR />
    //!          結果は Notify() メソッドを通じて非同期で送信されます。
    //! @param [in] args 実行パラメータ
    //! @param [in] listener 非同期メッセージを受け取るコンポーネントオブジェクト
    //! @return 無事開始できたか
    //! @retval S_OK 無事開始できた
    //! @retval S_OK_ASYNC 非同期処理を開始した
    //! @retval S_FALSE 既に実行中である
    //! @retval E_FAIL 実行開始できなかった
    //! @retval E_FAIL 非同期処理を開始できなかった
    virtual HRESULT __stdcall Start(LPVOID args = nullptr, IComponent* listener = nullptr) = 0;
    //!
    //!
    //! @brief このオブジェクトの実行を停止します。
    //! @details 停止後、このオブジェクトの状態からは STATE_ACTIVE フラグが取り除かれます。<BR />
    //! @details listener に nullptr 以外を渡した場合、<BR />
    //!          結果は Notify() メソッドを通じて非同期で送信されます。
    //! @param [in] listener 非同期メッセージを受け取るコンポーネントオブジェクト
    //! @return 無事停止できたか
    //! @retval S_OK 無事停止できた
    //! @retval S_OK_ASYNC 非同期処理を開始した
    //! @retval S_FALSE 既に停止していた
    //! @retval E_FAIL 停止できなかった
    //! @retval E_FAIL 非同期処理を開始できなかった
    virtual HRESULT __stdcall Stop(IComponent* listener = nullptr) = 0;
};

//---------------------------------------------------------------------------//

static const IID IID_IUIComponent =
{ 0xf5dce27e, 0xa45b, 0x45ed, { 0x8f, 0x8e, 0x39, 0xf, 0xe, 0xc7, 0xf0, 0xd2 } };

//!
//! @interface IUIComponent Interfaces.h
//! @brief UIコンポーネントのためのインターフェイスです。
//!
struct IUIComponent : public IComponent
{
    //!
    //! @brief 管理しているウィンドウの数を取得します。
    //! @param なし
    //! @return ウィンドウの数
    virtual size_t __stdcall WindowCount()        const = 0;
    //!
    //!
    //! @brief 管理しているウィンドウのハンドルを取得します。
    //! @param [in] index 取得するウィンドウのインデックス
    //! @return ウィンドウハンドル
    //! @retval nullptr 無効なインデックス
    virtual HWND   __stdcall Handle(size_t index) const = 0;
};

//---------------------------------------------------------------------------//

static const IID IID_IIOComponent=
{ 0xeefb9211, 0xfa4, 0x4c0c, { 0x99, 0xd6, 0xc0, 0xc, 0xc6, 0xc0, 0xe1, 0x1 } };

//!
//! @interface IIOComponent Interfaces.h
//! @brief 入出力コンポーネントのための基底インターフェイスです。
//!
struct IIOComponent : public IComponent
{
    //!
    //! @brief 所有しているオブジェクトを閉じます。
    //! @details 無事閉じた場合、このオブジェクトの状態からは STATE_OPEN フラグが取り除かれます。<BR />
    //! @details listener に nullptr 以外を渡した場合、<BR />
    //!          結果は Notify() メソッドを通じて非同期で送信されます。
    //! @param [in] listener 非同期メッセージを受け取るコンポーネントオブジェクト
    //! @return 無事閉じられたか
    //! @retval S_OK オブジェクトは無事閉じられた
    //! @retval S_OK_ASYNC 非同期処理を開始した
    //! @retval S_FALSE 既に閉じている
    //! @retval E_COMP_BUSY 別の処理を実行中
    //! @retval E_FAIL オブジェクトを閉じられなかった
    //! @retval E_FAIL 非同期処理を開始できなかった
    virtual HRESULT __stdcall Close(IComponent* listener = nullptr) = 0;
    //!
    //!
    //! @brief 指定された名前のオブジェクトを開きます。
    //! @details 無事開いた場合、このオブジェクトの状態には STATE_OPEN フラグが立ちます。<BR />
    //! @details listener に nullptr 以外を渡した場合、<BR />
    //!          結果は Notify() メソッドを通じて非同期で送信されます。
    //! @param [in] path 開きたいオブジェクトのフルパス
    //! @param [in] format_as 開きたい形式
    //! @param [in] listener 非同期メッセージを受け取るコンポーネントオブジェクト
    //! @return 無事開けたか
    //! @retval S_OK オブジェクトは無事開くことができた
    //! @retval S_OK_ASYNC 非同期処理を開始した
    //! @retval S_FALSE 既に開いている
    //! @retval E_COMP_BUSY 別の処理を実行中
    //! @retval E_FAIL オブジェクトを開けなかった
    //! @retval E_FAIL 非同期処理を開始できなかった
    virtual HRESULT __stdcall Open(LPCWSTR path, LPCWSTR format_as, IComponent* listener = nullptr) = 0;
    //!
    //!
    //! @brief オブジェクトを指定のフォーマットで開くことができるかを問い合わせます。
    //! @details オブジェクトは開かれません。
    //! @param [in] path 開きたいオブジェクトのパス
    //! @param [in] format_as フォーマット
    //! @return オブジェクトを開くことができるか
    //! @retval S_OK
    //! @retval E_FAIL
    virtual HRESULT __stdcall QuerySupport(LPCWSTR path, LPCWSTR format_as) = 0;
    //!
    //!
    //! @brief 所有しているオブジェクトのポインタ位置を走査します。
    //! @details offset に 0、origin に STREAM_SEEK_CUR を指定した場合、<BR />
    //!          new_pos に現在のポインタ位置が返ります。
    //! @param [in] offset origin パラメータからの差分
    //! @param [in] origin offset パラメータが基準とする位置
    //!     @arg STREAM_SEEK_SET = 0; // 先頭から
    //!     @arg STREAM_SEEK_CUR = 1; // 現在位置から
    //!     @arg STREAM_SEEK_END = 2; // 終端から
    //! @param [out] nes_pos 走査後のポインタ位置
    //! @return 無事走査できたか
    //! @retval S_OK 走査完了
    //! @retval E_COMP_BUSY 別の処理を実行中
    //! @retval E_PENDING オブジェクトはまだ開かれていない
    virtual HRESULT __stdcall Seek(INT64 offset, DWORD origin, UINT64* new_pos) = 0;
};

//---------------------------------------------------------------------------//

static const IID IID_IInputComponent =
{ 0x6b132e6c, 0x4703, 0x4281, { 0x88, 0x8d, 0x36, 0x7c, 0x1b, 0x40, 0xbd, 0x6a } };

//!
//! @interface IInputComponent Interfaces.h
//! @brief 入力コンポーネントのためのインターフェイスです。
//!
struct IInputComponent : public IIOComponent
{
    //!
    //! @brief 所有しているオブジェクトから指定バイト分データを読み込み、ポインタを進めます。
    //! @details listener に nullptr 以外を渡した場合、<BR />
    //!     結果は Notify() メソッドを通じて非同期で送信されます。
    //! @param [out] buffer データを読み込むバッファ
    //! @param [in] buf_size バッファのサイズ
    //! @param [out] cb_read 実際に読み込んだバイト数
    //! @param [in] listener 非同期メッセージを受け取るコンポーネントオブジェクト
    //! @return 無事読み込めたか
    //! @retval S_OK 無事読み込めた
    //! @retval S_OK_ASYNC 非同期処理を開始した
    //! @retval S_FALSE cb_read が buf_size より小さな値になった
    //! @retval E_PENDING オブジェクトはまだ開かれていない
    //! @retval E_FAIL 正常に読み込めなかった
    //! @retval E_FAIL 非同期処理を開始できなかった
    virtual HRESULT __stdcall Read(LPVOID buffer, size_t buf_size, size_t* cb_read, IComponent* listener = nullptr) = 0;
};

//---------------------------------------------------------------------------//

static const IID IID_IOutputComponent =
{ 0xba6ffb76, 0xa1cb, 0x439c, { 0x9f, 0x6d, 0xca, 0xd6, 0xd1, 0x8a, 0x8f, 0x3e } };

//!
//! @interface IOutputComponent Interfaces.h
//! @brief 出力コンポーネントのためのインターフェイスです。
//!
struct IOutputComponent : public IIOComponent
{
    //!
    //! @brief 所有しているオブジェクトに指定バイト分データを書き込み、ポインタを進めます。
    //! @details listener に nullptr 以外を渡した場合、<BR />
    //!     結果は Notify() メソッドを通じて非同期で送信されます。
    //! @param [in] buffer データを書き込むバッファ
    //! @param [in] buf_size バッファのサイズ
    //! @param [out] cb_written 実際に書き込んだバイト数
    //! @param [in] listener 非同期メッセージを受け取るコンポーネントオブジェクト
    //! @return 無事書き込めたか
    //! @retval S_OK 無事書き込めた
    //! @retval S_OK_ASYNC 非同期処理を開始した
    //! @retval S_FALSE cb_written が buf_size より小さな値になった
    //! @retval E_PENDING オブジェクトはまだ開かれていない
    //! @retval E_FAIL 正常に書き込めなかった
    //! @retval E_FAIL 非同期処理を開始できなかった
    virtual HRESULT __stdcall Write(LPVOID buffer, size_t buf_size, size_t* cb_written, IComponent* listener = nullptr) = 0;
};

//---------------------------------------------------------------------------//

} // namespace CubeMelon

// Interfaces.h