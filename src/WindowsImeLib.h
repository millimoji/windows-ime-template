#pragma once
#include <windows.h>
#include <cassert>
#include <memory>
#include <vector>

//---------------------------------------------------------------------
// candidate list
//---------------------------------------------------------------------
enum CANDIDATE_MODE
{
    CANDIDATE_NONE = 0,
    CANDIDATE_ORIGINAL,
    CANDIDATE_PHRASE,
    CANDIDATE_INCREMENTAL,
    CANDIDATE_WITH_NEXT_COMPOSITION
};

//---------------------------------------------------------------------
// enum
//---------------------------------------------------------------------
enum KEYSTROKE_CATEGORY
{
    CATEGORY_NONE = 0,
    CATEGORY_COMPOSING,
    CATEGORY_CANDIDATE,
    CATEGORY_PHRASE,
    // CATEGORY_PHRASEFROMKEYSTROKE,
    CATEGORY_INVOKE_COMPOSITION_EDIT_SESSION
};

enum KEYSTROKE_FUNCTION
{
    FUNCTION_NONE = 0,
    FUNCTION_INPUT,

    FUNCTION_CANCEL,
    FUNCTION_FINALIZE_TEXTSTORE,
    FUNCTION_FINALIZE_TEXTSTORE_AND_INPUT,
    FUNCTION_FINALIZE_CANDIDATELIST,
    FUNCTION_FINALIZE_CANDIDATELIST_AND_INPUT,
    FUNCTION_CONVERT,
    FUNCTION_CONVERT_WILDCARD,
    FUNCTION_SELECT_BY_NUMBER,
    FUNCTION_BACKSPACE,
    FUNCTION_MOVE_LEFT,
    FUNCTION_MOVE_RIGHT,
    FUNCTION_MOVE_UP,
    FUNCTION_MOVE_DOWN,
    FUNCTION_MOVE_PAGE_UP,
    FUNCTION_MOVE_PAGE_DOWN,
    FUNCTION_MOVE_PAGE_TOP,
    FUNCTION_MOVE_PAGE_BOTTOM,

    // Function Double/Single byte
    FUNCTION_DOUBLE_SINGLE_BYTE,

    // Function Punctuation
    FUNCTION_PUNCTUATION
};

//---------------------------------------------------------------------
// structure
//---------------------------------------------------------------------
struct _KEYSTROKE_STATE
{
    KEYSTROKE_CATEGORY Category;
    KEYSTROKE_FUNCTION Function;
};

template<class T>
class CSampleImeArray
{
    typedef typename std::vector<T> CSampleImeInnerArray;
    typedef typename std::vector<T>::iterator CSampleImeInnerIter;

public:
    CSampleImeArray(): _innerVect()
    {
    }

    explicit CSampleImeArray(size_t count): _innerVect(count)
    {
    }

    virtual ~CSampleImeArray()
    {
    }

    inline T* GetAt(size_t index)
    {
        assert(index >= 0);
        assert(index < _innerVect.size());

        T& curT = _innerVect.at(index);

        return &(curT);
    }

    inline const T* GetAt(size_t index) const
    {
        assert(index >= 0);
        assert(index < _innerVect.size());

        T& curT = _innerVect.at(index);

        return &(curT);
    }

    void RemoveAt(size_t index)
    {
        assert(index >= 0);
        assert(index < _innerVect.size());

        CSampleImeInnerIter iter = _innerVect.begin();
        _innerVect.erase(iter + index);
    }

    UINT Count() const 
    { 
        return static_cast<UINT>(_innerVect.size());
    }

    T* Append()
    {
        T newT = {};
        _innerVect.push_back(newT);
        T& backT = _innerVect.back();

        return &(backT);
    }

    void reserve(size_t Count)
    {
        _innerVect.reserve(Count);
    }

    void Clear()
    {
        _innerVect.clear();
    }

private:
    CSampleImeInnerArray _innerVect;
};

class CCandidateRange
{
public:
    CCandidateRange(void);
    ~CCandidateRange(void);

    BOOL IsRange(UINT vKey);
    int GetIndex(UINT vKey);

    inline int Count() const 
    { 
        return _CandidateListIndexRange.Count(); 
    }
    inline DWORD *GetAt(int index) 
    { 
        return _CandidateListIndexRange.GetAt(index); 
    }
    inline DWORD *Append() 
    { 
        return _CandidateListIndexRange.Append(); 
    }

private:
    CSampleImeArray<DWORD> _CandidateListIndexRange;
};

class CStringRange
{
public:
    CStringRange();
    ~CStringRange();

    const WCHAR *Get() const;
    const DWORD_PTR GetLength() const;
    void Clear();
    void Set(const WCHAR *pwch, DWORD_PTR dwLength);
    void Set(CStringRange &sr);
    CStringRange& operator=(const CStringRange& sr);
    void CharNext(_Inout_ CStringRange* pCharNext);
    static int Compare(LCID locale, _In_ CStringRange* pString1, _In_ CStringRange* pString2);
    static BOOL WildcardCompare(LCID locale, _In_ CStringRange* stringWithWildcard, _In_ CStringRange* targetString);

protected:
    DWORD_PTR _stringBufLen;         // Length is in character count.
    const WCHAR *_pStringBuf;    // Buffer which is not add zero terminate.
};

//---------------------------------------------------------------------
// CCandidateListItem
//  _ItemString - candidate string
//  _FindKeyCode - tailing string
//---------------------------------------------------------------------
struct CCandidateListItem
{
    CStringRange _ItemString;
    CStringRange _FindKeyCode;

    CCandidateListItem& operator =( const CCandidateListItem& rhs)
    {
        _ItemString = rhs._ItemString;
        _FindKeyCode = rhs._FindKeyCode;
        return *this;
    }
};

struct ITfThreadMgr;
typedef /* [uuid] */  DECLSPEC_UUID("de403c21-89fd-4f85-8b87-64584d063fbc") DWORD TfClientId;

namespace WindowsImeLib
{

inline USHORT ModifiersValue = 0;
inline BOOL   IsShiftKeyDownOnly = FALSE;
inline BOOL   IsControlKeyDownOnly = FALSE;
inline BOOL   IsAltKeyDownOnly = FALSE;

struct LanguageBarButtonProperty
{
    GUID id;
    GUID compartmentId;
    LPCWSTR langBarDescription;
    LPCWSTR description;
    int onIconResourceIndex;
    int offIconResourceIndex;
};

struct ICompositionProcessorEngineOwner
{
    virtual ~ICompositionProcessorEngineOwner() {}

    virtual void SetDefaultCandidateTextFont(int idsDefaultFont) = 0;

    virtual void SetupLanguageBar(_In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, BOOL isSecureMode, _In_reads_(countButtons) const LanguageBarButtonProperty* properties, UINT countButtons) = 0;
    virtual BOOL GetCompartmentBool(ITfThreadMgr *pThreadMgr, TfClientId tfClientId, REFGUID guidCompartment) = 0;
    virtual void SetCompartmentBool(ITfThreadMgr *pThreadMgr, TfClientId tfClientId, REFGUID guidCompartment, BOOL value) = 0;
    virtual DWORD GetCompartmentDword(ITfThreadMgr *pThreadMgr, TfClientId tfClientId, REFGUID guidCompartment) = 0;
    virtual void SetCompartmentDword(ITfThreadMgr *pThreadMgr, TfClientId tfClientId, REFGUID guidCompartment, DWORD value) = 0;
    virtual void ClearCompartment(ITfThreadMgr *pThreadMgr, TfClientId tfClientId, REFGUID guidCompartment) = 0;
};

struct ICompositionProcessorEngine
{
    virtual ~ICompositionProcessorEngine() {}

    // Get language profile.
    virtual GUID GetLanguageProfile(LANGID *plangid) = 0;

    // Get locale
    virtual BOOL SetupLanguageProfile(LANGID langid, REFGUID guidLanguageProfile, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId, BOOL isSecureMode, BOOL isComLessMode) = 0;
    virtual LCID GetLocale() = 0;

    virtual BOOL IsKeyEaten(_In_ ITfThreadMgr* pThreadMgr, TfClientId tfClientId, UINT code, _Inout_updates_(1) WCHAR *pwch,
        BOOL isComposing, CANDIDATE_MODE candidateMode, BOOL isCandidateWithWildcard, _Out_opt_ _KEYSTROKE_STATE *pKeyState) = 0;

    virtual BOOL AddVirtualKey(WCHAR wch) = 0;
    virtual void RemoveVirtualKey(DWORD_PTR dwIndex) = 0;
    virtual void PurgeVirtualKey() = 0;

    virtual DWORD_PTR GetVirtualKeyLength() = 0;

    virtual void GetReadingStrings(_Inout_ CSampleImeArray<CStringRange> *pReadingStrings, _Out_ BOOL *pIsWildcardIncluded) = 0;
    virtual void GetCandidateList(_Inout_ CSampleImeArray<CCandidateListItem> *pCandidateList, BOOL isIncrementalWordSearch, BOOL isWildcardSearch) = 0;
    virtual void GetCandidateStringInConverted(CStringRange &searchString, _In_ CSampleImeArray<CCandidateListItem> *pCandidateList) = 0;

    // Preserved key handler
    virtual void OnPreservedKey(REFGUID rguid, _Out_ BOOL *pIsEaten, _In_ ITfThreadMgr *pThreadMgr, TfClientId tfClientId) = 0;

    // Punctuation
    virtual BOOL IsPunctuation(WCHAR wch) = 0;
    virtual WCHAR GetPunctuation(WCHAR wch) = 0;

    virtual BOOL IsDoubleSingleByte(WCHAR wch) = 0;
    virtual BOOL IsMakePhraseFromText() = 0;

    // Language bar control
    virtual void ConversionModeCompartmentUpdated(_In_ ITfThreadMgr *pThreadMgr) = 0;

    virtual CCandidateRange *GetCandidateListIndexRange() = 0;
    virtual UINT GetCandidateWindowWidth() = 0;

    // Compartment
    virtual HRESULT CompartmentCallback(REFGUID guidCompartment) noexcept = 0;
    virtual void ClearCompartment(ITfThreadMgr *pThreadMgr, TfClientId tfClientId) = 0;
};

struct IProcessorFactory
{
    virtual ~IProcessorFactory() {}

    virtual std::shared_ptr<ICompositionProcessorEngine> CreateCompositionProcessorEngine(const std::weak_ptr<ICompositionProcessorEngineOwner>& owner) = 0;
};

// TODO: re-design how to inject factory
extern std::shared_ptr<IProcessorFactory> g_processorFactory;

// legacy code, hard to make all be in specific namspace.
extern BOOL DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pvReserved);
extern HRESULT DllGetClassObject(_In_ REFCLSID rclsid, _In_ REFIID riid, _Outptr_ void** ppv);
extern HRESULT DllCanUnloadNow(void);
extern HRESULT DllUnregisterServer(LANGID langId);
extern HRESULT DllRegisterServer(LANGID langId, int textServiceIconIndex);

}
