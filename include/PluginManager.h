// PluginManager.h

#pragma once

//---------------------------------------------------------------------------//

class IPropertyStore;

//!
//! @class PluginInstance PluginManager.h
//! @brief DLLファイルの各種情報を管理するためのラッパークラスです。
//!
class PluginInstance
{
public:
    //!
    //! @brief コンストラクタ
    //! @param [in] file_path DLLファイルのフルパス
    //! @param [in] index ファイルのうち何番目に格納されているプラグインか
    //!
    explicit PluginInstance(LPCWSTR file_path, size_t index);
    //!
    //! @brief デストラクタ
    //!
    ~PluginInstance();

    //!
    //! @brief プラグイン名を返します。
    //! @param なし
    //! @return プラグイン名
    //!
    LPCWSTR  __stdcall Name() const;
    //!
    //! @brief プラグインが格納されているDLLファイルのフルパスを返します。
    //! @param なし
    //! @return DLLファイルのフルパス
    //!
    LPCWSTR  __stdcall FilePath() const;
    //!
    //! @brief プラグインのクラスIDを返します。
    //! @param なし
    //! @return プラグインのクラスID
    //!
    REFCLSID __stdcall ClassID() const;

    //!
    //! @brief DLLファイルからプラグインを読み込みます。
    //! @param なし
    //! @return 無事読み込めたか
    //! @retval S_OK
    //! @retval E_FAIL
    //!
    HRESULT __stdcall Load();
    //!
    //! @brief DLLファイルを解放します。
    //! @param なし
    //! @return 無事解放できたか
    //! @retval S_OK
    //! @retval E_FAIL
    //!
    HRESULT __stdcall Free();
    //!
    //! @brief プラグインのプロパティを取得します。
    //! @param [out] ps プロパティオブジェクトの格納先アドレス
    //! @return 無事取得できたか
    //! @retval S_OK
    //! @retval E_FAIL
    //!
    HRESULT __stdcall GetProperty(IPropertyStore** ps);
    //!
    //! @brief ファクトリオブジェクトを取得します。
    //! @param [in] riid IID_IClassFactory を指定します。
    //! @param [out] ppvObject ファクトリオブジェクトの格納先アドレス
    //! @return 無事取得できたか
    //! @retval S_OK
    //! @retval E_FAIL
    //!
    HRESULT __stdcall GetClassObject(REFIID riid, void** ppvObject);

private:
    struct Impl;
    Impl* pimpl;

private:
    PluginInstance(const PluginInstance&);
    PluginInstance(PluginInstance&&);
    PluginInstance& operator =(const PluginInstance&);
    PluginInstance& operator =(PluginInstance&&);
};

//---------------------------------------------------------------------------//

//!
//! @class PluginManager PluginManager.h
//! @brief 全てのプラグインを一元的に管理するクラスです。
//! @note シングルトンです。プロセス空間で一意のインスタンスを持ちます。
//!
class PluginManager
{
public:
    //!
    //! @brief コンストラクタ
    //! @param なし
    //!
    PluginManager();
    //!
    //! @brief デストラクタ
    //!
    ~PluginManager();

    //!
    //! @brief 管理しているプラグインの数を取得します。
    //! @param なし
    //! @return プラグインの数
    //!
    size_t           __stdcall PluginCount() const;
    //!
    //! @brief プラグインのコレクションを配列で取得します。
    //! @param なし
    //! @return プラグインの配列
    //! @note 配列は NULL終端です。<BR />
    //!     取得したデータを解放する必要はありません。
    //!
    PluginInstance** __stdcall AllPlugins() const;

    //!
    //! @brief 全てのプラグインを読み込みます。
    //! @param なし
    //! @return 無事読み込めたか
    //! @retval S_OK
    //! @retval E_FAIL
    //!
    HRESULT __stdcall LoadAll();
    //!
    //! @brief 全てのプラグインを解放します。
    //! @param なし
    //! @return 無事解放できたか
    //! @retval S_OK 全て解放できた
    //! @retval S_FALSE 一部は解放できなかった
    //! @retval E_FAIL 一つも解放できなかった
    //!
    HRESULT __stdcall FreeAll();

private:
    struct Impl;
    Impl* pimpl;

private:
    PluginManager(const PluginManager&);
    PluginManager(PluginManager&&);
    PluginManager& operator =(const PluginManager&);
    PluginManager& operator =(PluginManager&&);
};

// PluginManager.h