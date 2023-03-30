#include "pch.h"
#include "AggregateData.h"
#include "../../../../LIB/CommonUtil/CGeoUtil.h"
#include "../../../../LIB/CommonUtil/ReadINIParam.h"
#include "../../../../LIB/CommonUtil/CFileIO.h"

#ifdef _DEBUG
#include <sys/timeb.h>
#include <time.h>
#include <psapi.h>
#endif

using namespace std;

AggregateData::AggregateData(void)
{
}

AggregateData::~AggregateData(void)
{
}

// split関数
vector<string> split(string str, string separator) {
    if (separator == "") return { str };
    vector<string> result;
    string tstr = str + separator;
    long l = (long)tstr.length(), sl = (long)separator.length();
    string::size_type pos = 0, prev = 0;

    for (;pos < l && (pos = tstr.find(separator, pos)) != string::npos; prev = (pos += sl)) {
        result.emplace_back(tstr, prev, pos - prev);
    }
    return result;
}

void SetJPZone()
{
    GetINIParam()->Initialize();
    JPZONE = GetINIParam()->GetJPZone();
}

// BSTR⇒std::string変換処理
std::string ConvertWCSToMBS(const wchar_t* pstr, long wslen)
{
    int len = ::WideCharToMultiByte(CP_ACP, 0, pstr, wslen, NULL, 0, NULL, NULL);

    std::string dblstr(len, '\0');
    len = ::WideCharToMultiByte(CP_ACP, 0 /* no flags */,
        pstr, wslen /* not necessary NULL-terminated */,
        &dblstr[0], len,
        NULL, NULL /* no default char */);

    return dblstr;
}
// BSTR⇒std::string変換
std::string ConvertBSTRToMBS(BSTR bstr)
{
    int wslen = ::SysStringLen(bstr);
    return ConvertWCSToMBS((wchar_t*)bstr, wslen);
}
/*
    stringをwstringへ変換する
*/
std::wstring StringToWString
(
    std::string oString
)
{
    // SJIS → wstring
    int iBufferSize = MultiByteToWideChar(CP_ACP, 0, oString.c_str()
        , -1, (wchar_t*)NULL, 0);

    // バッファの取得
    wchar_t* cpUCS2 = new wchar_t[iBufferSize];

    // SJIS → wstring
    MultiByteToWideChar(CP_ACP, 0, oString.c_str(), -1, cpUCS2
        , iBufferSize);

    // stringの生成
    std::wstring oRet(cpUCS2, cpUCS2 + iBufferSize - 1);

    // バッファの破棄
    delete[] cpUCS2;

    // 変換結果を返す
    return(oRet);
}
// std::string⇒wchar_t変換処理
wchar_t* ConvertStringTowchar(std::string str)
{
    const size_t newsizew = str.size() + 1;
    size_t convertedChars = 0;
    wchar_t* wcstring = new wchar_t[newsizew];
    mbstowcs_s(&convertedChars, wcstring, newsizew, str.c_str(), _TRUNCATE);

    return wcstring;
}
/**
* @brief フォルダ以下のファイル一覧を取得する関数
* @param[in]    folderPath  フォルダパス
* @param[out]   file_names  ファイル名一覧
* return        true:成功, false:失敗
*/
bool getFileNames(std::string folderPath, std::string ext, std::vector<std::string>& file_names)
{
    using namespace std::filesystem;
    directory_iterator iter(folderPath), end;
    std::error_code err;

    for (; iter != end && !err; iter.increment(err)) {
        const directory_entry entry = *iter;

        if (entry.path().extension() == ext) {
            file_names.push_back(entry.path().string());
            printf("%s\n", file_names.back().c_str());
        }
    }

    /* エラー処理 */
    if (err) {
        std::cout << err.value() << std::endl;
        std::cout << err.message() << std::endl;
        return false;
    }
    return true;
}
// CSV読み込み
vector<vector<string> >csv2vector(string filename, int ignore_line_num = 0) {
    //csvファイルの読み込み
    ifstream reading_file;
    reading_file.open(filename, ios::in);
    if (!reading_file) {
        vector<vector<string> > data;
        return data;
    }
    string reading_line_buffer;
    //最初のignore_line_num行を空読みする
    for (int line = 0; line < ignore_line_num; line++) {
        getline(reading_file, reading_line_buffer);
        if (reading_file.eof()) break;
    }

    //二次元のvectorを作成
    vector<vector<string> > data;
    while (getline(reading_file, reading_line_buffer)) {
        if (reading_line_buffer.size() == 0) break;
        vector<string> temp_data;
        temp_data = split(reading_line_buffer, ",");
        data.push_back(temp_data);
    }
    return data;
}

// 日時文字列(YYYYMMDDhhmmss)取得
string getDatetimeStr() {
    time_t t = time(nullptr);
    struct tm nowTime;
    errno_t error;
    error = localtime_s(&nowTime, &t);

    std::stringstream s;
    s << nowTime.tm_year + 1900;
    // setw(),setfill()で0詰め
    s << setw(2) << setfill('0') << nowTime.tm_mon + 1;
    s << setw(2) << setfill('0') << nowTime.tm_mday;
    s << setw(2) << setfill('0') << nowTime.tm_hour;
    s << setw(2) << setfill('0') << nowTime.tm_min;
    s << setw(2) << setfill('0') << nowTime.tm_sec;
    // std::stringにして値を返す
    return s.str();
}

// 建物IDを取得する
bool getBuildId(const MSXML2::IXMLDOMNodePtr& pXMLDOMNode, std::string& strid)
{
    bool bret = false;
    HRESULT hResult;

    // 建物IDのノードを取得
    MSXML2::IXMLDOMNodePtr building = 0;
    eCityGMLVersion version;
    for (version = eCityGMLVersion::VERSION_1; version != eCityGMLVersion::End; ++version) {

        BSTR build = _bstr_t("");

        switch (version)
        {
        case eCityGMLVersion::VERSION_1:
            build = SysAllocString(XPATH_stringAttribute1);
            break;

        case eCityGMLVersion::VERSION_2:
            build = SysAllocString(XPATH_stringAttribute1_2);
            break;

        default:
            break;
        }

        pXMLDOMNode->selectSingleNode(build, &building);
        if (building != NULL)   break;

    }

    if (version == eCityGMLVersion::End)
    {   // 取得失敗
        return false;
    }

    // 値を取得
    MSXML2::IXMLDOMNodePtr buildingValue = 0;
    BSTR val = _bstr_t("");
    switch (version)
    {
    case eCityGMLVersion::VERSION_1:
        val = SysAllocString(XPATH_stringAttribute2);
        break;

    case eCityGMLVersion::VERSION_2:
        val = SysAllocString(XPATH_stringAttribute2_2);
        break;

    default:
        break;
    }
    building->selectSingleNode(val, &buildingValue);
    if (NULL != buildingValue)
    {
        // ノードタイプ取得
        MSXML2::DOMNodeType eMemberNodeType;
        hResult = buildingValue->get_nodeType(&eMemberNodeType);
        if (FAILED(hResult))
        {
            assert(!"ノードタイプの取得に失敗");
            return false;
        }

        // エレメント型への変換
        MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
        hResult = buildingValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
        if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
        {
            assert(!"エレメント型への変換に失敗");
            return false;
        }

        // 値テキスト取得
        BSTR valueText;
        hResult = pXMLDOMMemberElement->get_text(&valueText);
        if (SUCCEEDED(hResult))
        {
            // BSTR⇒std::string変換
            strid = ConvertBSTRToMBS(valueText);
            bret = true;
        }
    }

    return bret;
}

// 洪水浸水リスクの浸水深を取得する
bool getRiverFloodingRisk(const MSXML2::IXMLDOMNodePtr& pXMLDOMNode, double& depth)
{
    bool bret = false;
    HRESULT hResult;

    // 洪水浸水想定のノードを取得
    MSXML2::IXMLDOMNodeListPtr genericAttributeSetList = NULL;
    long lGenericAttributeCountNode = 0;
    eCityGMLVersion version;
    for (version = eCityGMLVersion::VERSION_1; version != eCityGMLVersion::End; ++version) {

        BSTR xpGeneric1 = _bstr_t("");

        switch (version)
        {
        case eCityGMLVersion::VERSION_1:
            xpGeneric1 = SysAllocString(XPATH_genericAttributeSet1);
            break;

        case eCityGMLVersion::VERSION_2:
            xpGeneric1 = SysAllocString(XPATH_genericAttributeSet1_2);
            break;

        default:
            break;
        }

        pXMLDOMNode->selectNodes(xpGeneric1, &genericAttributeSetList);
        if (genericAttributeSetList == NULL) continue;

        // ノード件数
        hResult = genericAttributeSetList->get_length(&lGenericAttributeCountNode);
        if ( 0 != lGenericAttributeCountNode) {
            break;
        }
    }

    if (version == eCityGMLVersion::End)
    {   // 取得失敗
        return false;
    }

    // 洪水浸水数分繰り返す
    for (int j = 0; j < lGenericAttributeCountNode; j++) {

        // ノードリストのうちの一つのノードの取得
        MSXML2::IXMLDOMNodePtr pXMLDOMGenericNode = NULL;
        hResult = genericAttributeSetList->get_item(j, &pXMLDOMGenericNode);
        if (FAILED(hResult))
        {
            assert(!"ノードリストのうちの一つのノードの取得に失敗");
            continue;
        }

        switch (version)
        {
        case eCityGMLVersion::VERSION_1:
        {
            MSXML2::IXMLDOMNodePtr floodDepth = 0;
            BSTR flood = SysAllocString(XPATH_genericAttributeSet2);
            pXMLDOMGenericNode->selectSingleNode(flood, &floodDepth);
            if (NULL != floodDepth)
            {
                // 値を取得
                MSXML2::IXMLDOMNodePtr floodDepthValue = 0;
                BSTR val = SysAllocString(XPATH_genericAttributeSet3);
                floodDepth->selectSingleNode(val, &floodDepthValue);
                if (NULL != floodDepthValue)
                {
                    // ノードタイプ取得
                    MSXML2::DOMNodeType eMemberNodeType;
                    hResult = floodDepthValue->get_nodeType(&eMemberNodeType);
                    if (FAILED(hResult))
                    {
                        assert(!"ノードタイプの取得に失敗");
                        continue;
                    }

                    // エレメント型への変換
                    MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                    hResult = floodDepthValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                    if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                    {
                        assert(!"エレメント型への変換に失敗");
                        continue;
                    }

                    // 値テキスト取得
                    BSTR valueText;
                    hResult = pXMLDOMMemberElement->get_text(&valueText);
                    if (SUCCEEDED(hResult))
                    {
                        // BSTR⇒std::string変換
                        std::string valueStr = ConvertBSTRToMBS(valueText);
                        if (depth < stod(valueStr)) {
                            depth = stod(valueStr);
                        }
                        bret = true;
                    }
                }
            }
            break;
        }

        case eCityGMLVersion::VERSION_2:
        {
            // 値を取得
            MSXML2::IXMLDOMNodePtr floodDepthValue = 0;
            BSTR val = SysAllocString(XPATH_genericAttributeSet2_2);
            pXMLDOMGenericNode->selectSingleNode(val, &floodDepthValue);
            if (NULL != floodDepthValue)
            {
                // ノードタイプ取得
                MSXML2::DOMNodeType eMemberNodeType;
                hResult = floodDepthValue->get_nodeType(&eMemberNodeType);
                if (FAILED(hResult))
                {
                    assert(!"ノードタイプの取得に失敗");
                    continue;
                }

                // エレメント型への変換
                MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                hResult = floodDepthValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                {
                    assert(!"エレメント型への変換に失敗");
                    continue;
                }

                // 値テキスト取得
                BSTR valueText;
                hResult = pXMLDOMMemberElement->get_text(&valueText);
                if (SUCCEEDED(hResult))
                {
                    // BSTR⇒std::string変換
                    std::string valueStr = ConvertBSTRToMBS(valueText);
                    if (depth < stod(valueStr)) {
                        depth = stod(valueStr);
                    }
                    bret = true;
                }
            }
            break;
        }

        default:
            break;
        }
    }

    return true;
}

// 津波浸水リスクの浸水深を取得する
bool getTsunamiRisk(const MSXML2::IXMLDOMNodePtr& pXMLDOMNode, double& height)
{
    bool bret = false;
    HRESULT hResult;

    // 津波浸水想定のノードを取得
    MSXML2::IXMLDOMNodePtr tsunamiHeightNode = 0;
    eCityGMLVersion version;
    for (version = eCityGMLVersion::VERSION_1; version != eCityGMLVersion::End; ++version) {

        BSTR tsunami = _bstr_t("");

        switch (version)
        {
        case eCityGMLVersion::VERSION_1:
            tsunami = SysAllocString(XPATH_genericAttributeSet4);
            break;

        case eCityGMLVersion::VERSION_2:
            tsunami = SysAllocString(XPATH_genericAttributeSet3_2);
            break;

        default:
            break;
        }

        pXMLDOMNode->selectSingleNode(tsunami, &tsunamiHeightNode);
        if (tsunamiHeightNode != NULL)   break;

    }

    if (version == eCityGMLVersion::End)
    {   // 取得失敗
        return false;
    }

    switch (version)
    {
    case eCityGMLVersion::VERSION_1:
    {
        // 浸水深
        MSXML2::IXMLDOMNodePtr tsunamiHeight = 0;
        BSTR flood = SysAllocString(XPATH_genericAttributeSet2);
        tsunamiHeightNode->selectSingleNode(flood, &tsunamiHeight);
        if (NULL != tsunamiHeight)
        {
            // 値を取得
            MSXML2::IXMLDOMNodePtr tsunamiHeightValue = 0;
            BSTR val = SysAllocString(XPATH_genericAttributeSet3);
            tsunamiHeight->selectSingleNode(val, &tsunamiHeightValue);
            if (NULL != tsunamiHeightValue)
            {
                // ノードタイプ取得
                MSXML2::DOMNodeType eMemberNodeType;
                hResult = tsunamiHeight->get_nodeType(&eMemberNodeType);
                if (FAILED(hResult))
                {
                    assert(!"ノードタイプの取得に失敗");
                    return false;
                }

                // エレメント型への変換
                MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                hResult = tsunamiHeight->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                {
                    assert(!"エレメント型への変換に失敗");
                    return false;
                }

                // 値テキスト取得
                BSTR valueText;
                hResult = pXMLDOMMemberElement->get_text(&valueText);
                if (SUCCEEDED(hResult))
                {
                    // BSTR⇒std::string変換
                    std::string valueStr = ConvertBSTRToMBS(valueText);
                    height = stod(valueStr);
                    bret = true;
                }
            }
        }
        break;
    }

    case eCityGMLVersion::VERSION_2:
    {
        // 値を取得
        MSXML2::IXMLDOMNodePtr tsunamiHeightValue = 0;
        BSTR val = SysAllocString(XPATH_genericAttributeSet2_2);
        tsunamiHeightNode->selectSingleNode(val, &tsunamiHeightValue);
        if (NULL != tsunamiHeightValue)
        {
            // ノードタイプ取得
            MSXML2::DOMNodeType eMemberNodeType;
            hResult = tsunamiHeightValue->get_nodeType(&eMemberNodeType);
            if (FAILED(hResult))
            {
                assert(!"ノードタイプの取得に失敗");
                return false;
            }

            // エレメント型への変換
            MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
            hResult = tsunamiHeightValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
            if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
            {
                assert(!"エレメント型への変換に失敗");
                return false;
            }

            // 値テキスト取得
            BSTR valueText;
            hResult = pXMLDOMMemberElement->get_text(&valueText);
            if (SUCCEEDED(hResult))
            {
                // BSTR⇒std::string変換
                std::string valueStr = ConvertBSTRToMBS(valueText);
                height = stod(valueStr);
                bret = true;
            }
        }
        break;
    }

    default:
        break;
    }

    return true;
}

// 土砂災害リスクを取得する
bool getLandSlideRisk(const MSXML2::IXMLDOMNodePtr& pXMLDOMNode)
{
    // 土砂災害リスクのノードを取得
    MSXML2::IXMLDOMNodePtr landslideArea = 0;
    eCityGMLVersion version;
    for (version = eCityGMLVersion::VERSION_1; version != eCityGMLVersion::End; ++version) {

        BSTR landslide = _bstr_t("");

        switch (version)
        {
        case eCityGMLVersion::VERSION_1:
            landslide = SysAllocString(XPATH_genericAttributeSet5);
            break;

        case eCityGMLVersion::VERSION_2:
            landslide = SysAllocString(XPATH_genericAttributeSet4_2);
            break;

        default:
            break;
        }

        pXMLDOMNode->selectSingleNode(landslide, &landslideArea);
        if (landslideArea != NULL)   break;

    }

    if (version == eCityGMLVersion::End)
    {   // 取得失敗
        return false;
    }

    return true;
}

// 構造物種別を取得する
bool getBuildStructureType(const MSXML2::IXMLDOMNodePtr& pXMLDOMNode, int& iBldStructureType)
{
    bool bret = false;
    HRESULT hResult;

    MSXML2::IXMLDOMNodePtr structureType = 0;
    eCityGMLVersion version;
    for (version = eCityGMLVersion::VERSION_1; version != eCityGMLVersion::End; ++version) {

        BSTR structure = _bstr_t("");

        switch (version)
        {
        case eCityGMLVersion::VERSION_1:
            structure = SysAllocString(XPATH_buildingStructureType);
            break;

        case eCityGMLVersion::VERSION_2:
            structure = SysAllocString(XPATH_buildingStructureType_2);
            break;

        default:
            break;
        }

        pXMLDOMNode->selectSingleNode(structure, &structureType);
        if (structureType != NULL)   break;

    }

    if (version == eCityGMLVersion::End)
    {   // 取得失敗
        return false;
    }

    // ノードタイプ取得
    MSXML2::DOMNodeType eMemberNodeType;
    hResult = structureType->get_nodeType(&eMemberNodeType);
    if (FAILED(hResult))
    {
        assert(!"ノードタイプの取得に失敗");
        return false;
    }

    // エレメント型への変換
    MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
    hResult = structureType->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
    if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
    {
        assert(!"エレメント型への変換に失敗");
        return false;
    }

    // 値テキスト取得
    BSTR valueText;
    hResult = pXMLDOMMemberElement->get_text(&valueText);
    if (SUCCEEDED(hResult))
    {
        // BSTR⇒std::string変換
        std::string valueStr = ConvertBSTRToMBS(valueText);
        iBldStructureType = stoi(valueStr);
        bret = true;
    }

    return bret;
}

// 建物属性取得
vector<BUILDING> GetBldgAttribute(wstring xmldata)
{
    //xmlオブジェクト生成
    MSXML2::IXMLDOMDocument2Ptr reader;
    reader.CreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_INPROC_SERVER);

    //xpathをサポートするように設定
    reader->setProperty(CComBSTR(L"SelectionLanguage"), CComVariant(L"XPath"));

    // uroタグの名前空間URIをチェック
    CFileIO fio;
    if (fio.Open(xmldata, L"rt"))
    {
        std::wstring strLine;
        int lineCnt = 0;
        wchar_t cBuff[1024];

        while (fio.ReadLineW(cBuff, 1024) != NULL)
        {
            strLine = cBuff;
            if (strLine.find(L"core:CityModel") != std::string::npos)
            {
                if (strLine.find(uroNamespace1) != std::string::npos)    // 旧バージョン
                {
                    //namespaceをサポートするように設定
                    reader->setProperty(CComBSTR(L"SelectionNamespaces"), CComVariant(NAME_SPACE));
                }
                else if (strLine.find(uroNamespace2) != std::string::npos)    // 新バージョン
                {
                    //namespaceをサポートするように設定
                    reader->setProperty(CComBSTR(L"SelectionNamespaces"), CComVariant(NAME_SPACE2));
                }
                break;
            }
        }
    }
    else
    {
        //namespaceをサポートするように設定
        reader->setProperty(CComBSTR(L"SelectionNamespaces"), CComVariant(NAME_SPACE));
    }
    fio.Close();

    //ロード
    VARIANT_BOOL isSuccessful;
    reader->load(CComVariant(xmldata.c_str()), &isSuccessful);

    // 建物リスト
    vector<BUILDING> allBuildingList{};

    HRESULT hResult;

    BSTR xp2 = SysAllocString(XPATH2);
    MSXML2::IXMLDOMNodeListPtr buildingList = NULL;
    reader->selectNodes(xp2, &buildingList);

    // ノード件数取得
    long lCountNode = 0;
    hResult = buildingList->get_length(&lCountNode);

    // 建物数分繰り返す
    for (int i = 0; i < lCountNode; i++) {
        // 建物情報初期化
        BUILDING buildingInfo{};

        // ノードリストのうちの一つのノードの取得
        MSXML2::IXMLDOMNodePtr pXMLDOMNode = NULL;
        hResult = buildingList->get_item(i, &pXMLDOMNode);
        if (FAILED(hResult))
        {
            assert(!"ノードリストのうちの一つのノードの取得に失敗");
            continue;
        }

        // 建物ID
        std::string buildId = "";
        if (!getBuildId(pXMLDOMNode, buildId))
        {
            assert(!"建物IDの取得に失敗");
            continue;
        }
        buildingInfo.strBuildingId = buildId;

        // 年間予測日射量
        MSXML2::IXMLDOMNodePtr solorRadiation = 0;
        BSTR solor = SysAllocString(XPATH_measureAttribute1);
        pXMLDOMNode->selectSingleNode(solor, &solorRadiation);
        if (NULL != solorRadiation)
        {
            // 値を取得
            MSXML2::IXMLDOMNodePtr solorRadiationValue = 0;
            BSTR val = SysAllocString(XPATH_measureAttribute2);
            solorRadiation->selectSingleNode(val, &solorRadiationValue);
            if (NULL != solorRadiationValue)
            {
                // ノードタイプ取得
                MSXML2::DOMNodeType eMemberNodeType;
                hResult = solorRadiationValue->get_nodeType(&eMemberNodeType);
                if (FAILED(hResult))
                {
                    assert(!"ノードタイプの取得に失敗");
                    continue;
                }

                // エレメント型への変換
                MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                hResult = solorRadiationValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                {
                    assert(!"エレメント型への変換に失敗");
                    continue;
                }

                // 値テキスト取得
                BSTR valueText;
                hResult = pXMLDOMMemberElement->get_text(&valueText);
                if (SUCCEEDED(hResult))
                {
                    // BSTR⇒std::string変換
                    std::string valueStr = ConvertBSTRToMBS(valueText);
                    buildingInfo.dSolorRadiation = stod(valueStr);

                }
            }
        }

        // 高さ
        MSXML2::IXMLDOMNodePtr height = 0;
        BSTR ht = SysAllocString(XPATH_measuredHeight1);
        pXMLDOMNode->selectSingleNode(ht, &height);
        if (NULL != height)
        {
            // ノードタイプ取得
            MSXML2::DOMNodeType eMemberNodeType;
            hResult = height->get_nodeType(&eMemberNodeType);
            if (FAILED(hResult))
            {
                assert(!"ノードタイプの取得に失敗");
                continue;
            }

            // エレメント型への変換
            MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
            hResult = height->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
            if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
            {
                assert(!"エレメント型への変換に失敗");
                continue;
            }

            // 値テキスト取得
            BSTR valueText;
            hResult = pXMLDOMMemberElement->get_text(&valueText);
            if (SUCCEEDED(hResult))
            {
                // BSTR⇒std::string変換
                std::string valueStr = ConvertBSTRToMBS(valueText);
                buildingInfo.dBuildingHeight = stod(valueStr);

            }
                
        }

        // 洪水浸水想定の浸水深
        double wFloodDepth = 0.0;
        // 値があれば設定(最大値)
        if (getRiverFloodingRisk(pXMLDOMNode, wFloodDepth))
        {
            buildingInfo.dFloodDepth = wFloodDepth;
        }

        // 津波浸水想定の浸水深
        double tsunamiHeight = 0.0;
        if (getTsunamiRisk(pXMLDOMNode, tsunamiHeight))
        {
            buildingInfo.dTsunamiHeight = tsunamiHeight;
        }

        // 土砂災害警戒区域
        if (getLandSlideRisk(pXMLDOMNode))
        {
            buildingInfo.bLandslideArea = true;
        }
        else
        {
            buildingInfo.bLandslideArea = false;
        }

        // 構造種別
        int iBldStructureType = 0;
        if (getBuildStructureType(pXMLDOMNode, iBldStructureType))
        {
            buildingInfo.iBldStructureType = iBldStructureType;
        }

        // 都市ごとの独自区分に基づく建築構造の種類
        // 都市ごとの独自区分に基づく地上階数の範囲
        BSTR xpExtended1 = SysAllocString(XPATH_extendedAttribute1);
        MSXML2::IXMLDOMNodeListPtr extendedAttributeList = NULL;
        pXMLDOMNode->selectNodes(xpExtended1, &extendedAttributeList);

        // ノード件数取得
        long lExtendedAttributeCountNode = 0;
        hResult = extendedAttributeList->get_length(&lExtendedAttributeCountNode);

        // 都市ごとの独自区分数分繰り返す
        for (int j = 0; j < lExtendedAttributeCountNode; j++) {

            // ノードリストのうちの一つのノードの取得
            MSXML2::IXMLDOMNodePtr pXMLDOMExtendedNode = NULL;
            hResult = extendedAttributeList->get_item(j, &pXMLDOMExtendedNode);
            if (FAILED(hResult))
            {
                assert(!"ノードリストのうちの一つのノードの取得に失敗");
                continue;
            }

            MSXML2::IXMLDOMNodePtr keyValuePair = 0;
            BSTR key = SysAllocString(XPATH_extendedAttribute2);
            pXMLDOMExtendedNode->selectSingleNode(key, &keyValuePair);
            if (NULL != keyValuePair)
            {
                // 値を取得
                    // ノードタイプ取得
                MSXML2::DOMNodeType ekeyValuePairNodeType;
                hResult = keyValuePair->get_nodeType(&ekeyValuePairNodeType);
                if (FAILED(hResult))
                {
                    assert(!"ノードタイプの取得に失敗");
                    continue;
                }

                // エレメント型への変換
                MSXML2::IXMLDOMElementPtr pXMLDOMkeyValuePairElement = NULL;
                hResult = keyValuePair->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMkeyValuePairElement);
                if (FAILED(hResult) || NULL == pXMLDOMkeyValuePairElement)
                {
                    assert(!"エレメント型への変換に失敗");
                    continue;
                }

                // 値テキスト取得
                BSTR valueText;
                std::string keyValueStr;
                hResult = pXMLDOMkeyValuePairElement->get_text(&valueText);
                if (SUCCEEDED(hResult))
                {
                    // BSTR⇒std::string変換
                    keyValueStr = ConvertBSTRToMBS(valueText);
                }

                // 建築構造の種類or地上階数の範囲
                if (keyValueStr == "10" || keyValueStr == "100")
                {
                    // 値を取得
                    MSXML2::IXMLDOMNodePtr codeValue = 0;
                    BSTR val = SysAllocString(XPATH_extendedAttribute3);
                    pXMLDOMExtendedNode->selectSingleNode(val, &codeValue);
                    if (NULL != codeValue)
                    {
                        // ノードタイプ取得
                        MSXML2::DOMNodeType eCodeValueNodeType;
                        hResult = codeValue->get_nodeType(&eCodeValueNodeType);
                        if (FAILED(hResult))
                        {
                            assert(!"ノードタイプの取得に失敗");
                            continue;
                        }

                        // エレメント型への変換
                        MSXML2::IXMLDOMElementPtr pXMLDOMCodeValueElement = NULL;
                        hResult = codeValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMCodeValueElement);
                        if (FAILED(hResult) || NULL == pXMLDOMCodeValueElement)
                        {
                            assert(!"エレメント型への変換に失敗");
                            continue;
                        }

                        // 値テキスト取得
                        BSTR valueText;
                        hResult = pXMLDOMCodeValueElement->get_text(&valueText);
                        if (SUCCEEDED(hResult))
                        {
                            // BSTR⇒std::string変換
                            std::string valueStr = ConvertBSTRToMBS(valueText);
                            if (keyValueStr == "10")
                            {
                                buildingInfo.iBldStructureType2 = stoi(valueStr);
                            }
                            else if(keyValueStr == "100")
                            {
                                buildingInfo.iFloorType = stoi(valueStr);
                            }
                        }
                    }
                }
            }

        }

        // 屋根面座標
        BSTR xp4 = SysAllocString(XPATH4);
        MSXML2::IXMLDOMNodeListPtr roofSurfaceList = NULL;
        pXMLDOMNode->selectNodes(xp4, &roofSurfaceList);

        // ノード件数取得
        long lRoofSurfaceCountNode = 0;
        hResult = roofSurfaceList->get_length(&lRoofSurfaceCountNode);

        // 屋根数分繰り返す
        for (int j = 0; j < lRoofSurfaceCountNode; j++) {

            // 屋根情報初期化
            ROOFSURFACES roofSurfaces{};

            // ノードリストのうちの一つのノードの取得
            MSXML2::IXMLDOMNodePtr pXMLDOMRoofNode = NULL;
            hResult = roofSurfaceList->get_item(j, &pXMLDOMRoofNode);
            if (FAILED(hResult))
            {
                assert(!"ノードリストのうちの一つのノードの取得に失敗");
                continue;
            }

            // 屋根情報タグ選択
            MSXML2::IXMLDOMNodePtr roofSurface = 0;
            BSTR roof = SysAllocString(XPATH5);
            pXMLDOMRoofNode->selectSingleNode(roof, &roofSurface);
            // 屋根情報があれば処理を実行
            if (NULL != roofSurface) {
                // ノードタイプ取得
                MSXML2::DOMNodeType eRoofNodeType;
                hResult = roofSurface->get_nodeType(&eRoofNodeType);
                if (FAILED(hResult))
                {
                    assert(!"ノードタイプの取得に失敗");
                    continue;
                }

                // エレメント型への変換
                MSXML2::IXMLDOMElementPtr pXMLDOMRoofElement = NULL;
                hResult = roofSurface->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMRoofElement);
                if (FAILED(hResult) || NULL == pXMLDOMRoofElement)
                {
                    assert(!"エレメント型への変換に失敗");
                    continue;
                }

                // 屋根IDを取得
                MSXML2::IXMLDOMAttribute* pAttributeRoofNode = NULL;
                CComVariant varValue;
                BSTR id = SysAllocString(L"gml:id");
                hResult = pXMLDOMRoofElement->getAttribute(id, &varValue);
                if (SUCCEEDED(hResult) && VT_BSTR == varValue.vt)
                {
                    // BSTR⇒std::string変換
                    std::string roofSurfaceId = ConvertBSTRToMBS(varValue.bstrVal);
                    roofSurfaces.roofSurfaceId = roofSurfaceId;

                }

                // 屋根詳細情報取得(ノード)
                BSTR xp6 = SysAllocString(XPATH6);
                MSXML2::IXMLDOMNodeListPtr surfaceMemberList = NULL;
                pXMLDOMRoofNode->selectNodes(xp6, &surfaceMemberList);

                // ノード件数取得
                long lSurfaceMemberCountNode = 0;
                hResult = surfaceMemberList->get_length(&lSurfaceMemberCountNode);

                // 屋根詳細数分繰り返す
                for (int k = 0; k < lSurfaceMemberCountNode; k++) {

                    // ワーク屋根詳細初期化
                    SURFACEMEMBERS wSurfaceMembers{};

                    // ノードリストのうちの一つのノードの取得
                    MSXML2::IXMLDOMNodePtr pXMLDOMSurfaceNode = NULL;
                    hResult = surfaceMemberList->get_item(k, &pXMLDOMSurfaceNode);
                    if (FAILED(hResult))
                    {
                        assert(!"ノードリストのうちの一つのノードの取得に失敗");
                        continue;
                    }


                    // 座標タグ選択
                    MSXML2::IXMLDOMNodePtr Position = 0;
                    BSTR pos = SysAllocString(XPATH7);
                    pXMLDOMSurfaceNode->selectSingleNode(pos, &Position);

                    if (NULL != Position) {
                        // ノードタイプ取得
                        MSXML2::DOMNodeType eMemberNodeType;
                        hResult = Position->get_nodeType(&eMemberNodeType);
                        if (FAILED(hResult))
                        {
                            assert(!"ノードタイプの取得に失敗");
                            continue;
                        }

                        // エレメント型への変換
                        MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                        hResult = Position->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                        if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                        {
                            assert(!"エレメント型への変換に失敗");
                            continue;
                        }

                        // 座標テキスト取得
                        BSTR positionText;
                        hResult = pXMLDOMMemberElement->get_text(&positionText);
                        if (SUCCEEDED(hResult))
                        {
                            // BSTR⇒std::string変換
                            std::string posStr = ConvertBSTRToMBS(positionText);

                            // 座標分割
                            vector<std::string> posAry = split(posStr, " ");

                            // 座標格納
                            int posCnt = 0;
                            // 一時座標リスト初期化
                            CPointBase wPosition{};

                            // 平面直角変換用
                            double dEast, dNorth;

                            for (int x = 0; x < posAry.size(); x++) {

                                if (posCnt == 0) {
                                    // doubleに変換
                                    wPosition.y = std::stod(posAry[x]);

                                    posCnt++;
                                }
                                else if (posCnt == 1) {
                                    // doubleに変換
                                    wPosition.x = std::stod(posAry[x]);

                                    posCnt++;
                                }
                                else if (posCnt == 2) {
                                    // doubleに変換
                                    wPosition.z = std::stod(posAry[x]);

                                    // 平面直角座標に変換

                                    CGeoUtil::LonLatToXY(wPosition.x, wPosition.y, JPZONE, dEast, dNorth);
                                    CPointBase pt(dEast, dNorth, wPosition.z);

                                    // 屋根詳細リスト内座標リストに追加                                                
                                    wSurfaceMembers.posList.push_back(pt);

                                    // 一時座標リスト初期化
                                    CPointBase wPosition{};
                                    posCnt = 0;
                                }

                            }
                        }
                    }
                    else {
                        continue;
                    }
                    // 屋根詳細リストに追加
                    roofSurfaces.roofSurfaceList.push_back(wSurfaceMembers);
                }
                // 屋根リストに追加
                buildingInfo.roofSurfaceList.push_back(roofSurfaces);
            }

        }

        // 建物リストに追加
        allBuildingList.push_back(buildingInfo);

    }


    //xmlオブジェクト解放
    reader.Release();

    return allBuildingList;

}

// 集計元データ取得
vector<AGTBUILDING> GetBldgAggregateData(wstring xmldata, int meshid, vector<JUDGESUITABLEPLACE> judge)
{
    //xmlオブジェクト生成
    MSXML2::IXMLDOMDocument2Ptr reader;
    reader.CreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_INPROC_SERVER);

    //xpathをサポートするように設定
    reader->setProperty(CComBSTR(L"SelectionLanguage"), CComVariant(L"XPath"));
    //namespaceをサポートするように設定
    reader->setProperty(CComBSTR(L"SelectionNamespaces"), CComVariant(NAME_SPACE2));

    //ロード
    VARIANT_BOOL isSuccessful;
    reader->load(CComVariant(xmldata.c_str()), &isSuccessful);

    // 建物リスト
    vector<AGTBUILDING> AgtBuildingList{};

    HRESULT hResult;

    // 建物数分繰り返す
    for (int i = 0; i < judge.size(); i++) {
        // 建物情報初期化
        AGTBUILDING buildingInfo{};

        // 建物IDを検索
        MSXML2::IXMLDOMNodeListPtr buildingList = NULL;
        long lCountNode = 0;
        eCityGMLVersion version;
        for (version = eCityGMLVersion::VERSION_1; version != eCityGMLVersion::End; ++version)
        {
            BSTR result = _bstr_t("");

            switch (version)
            {
            case eCityGMLVersion::VERSION_1:
            {
                const std::string bldgPath = "core:CityModel/core:cityObjectMember/bldg:Building/gen:stringAttribute/gen:value";
                string path = bldgPath + "[contains(text(),'" + judge[i].strBuildingId + "')]";
                wstring wide_string = wstring(path.begin(), path.end());
                result = _bstr_t(wide_string.c_str());
                break;
            }

            case eCityGMLVersion::VERSION_2:
            {
                const std::string bldgPath = "core:CityModel/core:cityObjectMember/bldg:Building/uro:buildingIDAttribute/uro:BuildingIDAttribute/uro:buildingID";
                string path = bldgPath + "[contains(text(),'" + judge[i].strBuildingId + "')]";
                wstring wide_string = wstring(path.begin(), path.end());
                result = _bstr_t(wide_string.c_str());
                break;
            }

            default:
                break;
            }

            // 建物ID指定のノードを選択
            BSTR xp2 = SysAllocString(result);
            reader->selectNodes(xp2, &buildingList);
            if (buildingList == NULL)   continue;

            // ノード件数取得
            hResult = buildingList->get_length(&lCountNode);
            if (0 != lCountNode) {
                break;
            }

        }

        if (version == eCityGMLVersion::End)
        {   // 取得失敗
            continue;
        }
                
        for (int j = 0; j < lCountNode; j++) {

            // ノードリストのうちの一つのノードの取得
            MSXML2::IXMLDOMNodePtr pXMLDOMNode = NULL;
            hResult = buildingList->get_item(j, &pXMLDOMNode);
            if (FAILED(hResult))
            {
                assert(!"ノードリストのうちの一つのノードの取得に失敗");
                continue;
            }

            // 建物IDを取得
            buildingInfo.strBuildingId = judge[i].strBuildingId;

            // 親の親に移動　
            MSXML2::IXMLDOMNodePtr pPearentNode = NULL;
            hResult = pXMLDOMNode->get_parentNode(&pPearentNode);
            if (FAILED(hResult))
            {
                assert(!"親ノードタイプの取得に失敗");
                continue;
            }
            hResult = pPearentNode->get_parentNode(&pXMLDOMNode);
            if (FAILED(hResult))
            {
                assert(!"親ノードタイプの取得に失敗");
                continue;
            }
            if (version == eCityGMLVersion::VERSION_2)
            {
                MSXML2::IXMLDOMNodePtr pPearentNode2 = pXMLDOMNode;
                hResult = pPearentNode2->get_parentNode(&pXMLDOMNode);
                if (FAILED(hResult))
                {
                    assert(!"親ノードタイプの取得に失敗");
                    continue;
                }
            }
            

            // 年間予測日射量
            MSXML2::IXMLDOMNodePtr solorRadiation = 0;
            BSTR solor = SysAllocString(XPATH_aggregateData1);
            pXMLDOMNode->selectSingleNode(solor, &solorRadiation);
            if (NULL != solorRadiation)
            {
                // 値を取得
                MSXML2::IXMLDOMNodePtr solorRadiationValue = 0;
                BSTR val = SysAllocString(XPATH_aggregateData2);
                solorRadiation->selectSingleNode(val, &solorRadiationValue);
                if (NULL != solorRadiationValue)
                {
                    // ノードタイプ取得
                    MSXML2::DOMNodeType eMemberNodeType;
                    hResult = solorRadiationValue->get_nodeType(&eMemberNodeType);
                    if (FAILED(hResult))
                    {
                        assert(!"ノードタイプの取得に失敗");
                        continue;
                    }

                    // エレメント型への変換
                    MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                    hResult = solorRadiationValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                    if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                    {
                        assert(!"エレメント型への変換に失敗");
                        continue;
                    }

                    // 値テキスト取得
                    BSTR valueText;
                    hResult = pXMLDOMMemberElement->get_text(&valueText);
                    if (SUCCEEDED(hResult))
                    {
                        // BSTR⇒std::string変換
                        std::string valueStr = ConvertBSTRToMBS(valueText);
                        buildingInfo.dSolorRadiation = stod(valueStr);

                    }
                }
            }

            // 年間予測発電量
            MSXML2::IXMLDOMNodePtr electricGeneration = 0;
            BSTR electric = SysAllocString(XPATH_aggregateData3);
            pXMLDOMNode->selectSingleNode(electric, &electricGeneration);
            if (NULL != electricGeneration)
            {
                // 値を取得
                MSXML2::IXMLDOMNodePtr electricGenerationValue = 0;
                BSTR val = SysAllocString(XPATH_aggregateData2);
                electricGeneration->selectSingleNode(val, &electricGenerationValue);
                if (NULL != electricGenerationValue)
                {
                    // ノードタイプ取得
                    MSXML2::DOMNodeType eMemberNodeType;
                    hResult = electricGenerationValue->get_nodeType(&eMemberNodeType);
                    if (FAILED(hResult))
                    {
                        assert(!"ノードタイプの取得に失敗");
                        continue;
                    }

                    // エレメント型への変換
                    MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                    hResult = electricGenerationValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                    if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                    {
                        assert(!"エレメント型への変換に失敗");
                        continue;
                    }

                    // 値テキスト取得
                    BSTR valueText;
                    hResult = pXMLDOMMemberElement->get_text(&valueText);
                    if (SUCCEEDED(hResult))
                    {
                        // BSTR⇒std::string変換
                        std::string valueStr = ConvertBSTRToMBS(valueText);
                        buildingInfo.dElectricGeneration = stod(valueStr);

                    }
                }
            }

            // 光害発生時間(夏至)
            MSXML2::IXMLDOMNodePtr lightPollutionSummer = 0;
            BSTR summer = SysAllocString(XPATH_aggregateData4);
            pXMLDOMNode->selectSingleNode(summer, &lightPollutionSummer);
            if (NULL != lightPollutionSummer)
            {
                // 値を取得
                MSXML2::IXMLDOMNodePtr lightPollutionSummerValue = 0;
                BSTR val = SysAllocString(XPATH_aggregateData2);
                lightPollutionSummer->selectSingleNode(val, &lightPollutionSummerValue);
                if (NULL != lightPollutionSummerValue)
                {
                    // ノードタイプ取得
                    MSXML2::DOMNodeType eMemberNodeType;
                    hResult = lightPollutionSummerValue->get_nodeType(&eMemberNodeType);
                    if (FAILED(hResult))
                    {
                        assert(!"ノードタイプの取得に失敗");
                        continue;
                    }

                    // エレメント型への変換
                    MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                    hResult = lightPollutionSummerValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                    if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                    {
                        assert(!"エレメント型への変換に失敗");
                        continue;
                    }

                    // 値テキスト取得
                    BSTR valueText;
                    hResult = pXMLDOMMemberElement->get_text(&valueText);
                    if (SUCCEEDED(hResult))
                    {
                        // BSTR⇒std::string変換
                        std::string valueStr = ConvertBSTRToMBS(valueText);
                        buildingInfo.dLightPollutionSummer = stod(valueStr);

                    }
                }
            }

            // 光害発生時間(春分)
            MSXML2::IXMLDOMNodePtr lightPollutionSpling = 0;
            BSTR spling = SysAllocString(XPATH_aggregateData5);
            pXMLDOMNode->selectSingleNode(spling, &lightPollutionSpling);
            if (NULL != lightPollutionSpling)
            {
                // 値を取得
                MSXML2::IXMLDOMNodePtr lightPollutionSplingValue = 0;
                BSTR val = SysAllocString(XPATH_aggregateData2);
                lightPollutionSpling->selectSingleNode(val, &lightPollutionSplingValue);
                if (NULL != lightPollutionSplingValue)
                {
                    // ノードタイプ取得
                    MSXML2::DOMNodeType eMemberNodeType;
                    hResult = lightPollutionSplingValue->get_nodeType(&eMemberNodeType);
                    if (FAILED(hResult))
                    {
                        assert(!"ノードタイプの取得に失敗");
                        continue;
                    }

                    // エレメント型への変換
                    MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                    hResult = lightPollutionSplingValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                    if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                    {
                        assert(!"エレメント型への変換に失敗");
                        continue;
                    }

                    // 値テキスト取得
                    BSTR valueText;
                    hResult = pXMLDOMMemberElement->get_text(&valueText);
                    if (SUCCEEDED(hResult))
                    {
                        // BSTR⇒std::string変換
                        std::string valueStr = ConvertBSTRToMBS(valueText);
                        buildingInfo.dLightPollutionSpling = stod(valueStr);

                    }
                }
            }

            // 光害発生時間(冬至)
            MSXML2::IXMLDOMNodePtr lightPollutionWinter = 0;
            BSTR winter = SysAllocString(XPATH_aggregateData6);
            pXMLDOMNode->selectSingleNode(winter, &lightPollutionWinter);
            if (NULL != lightPollutionWinter)
            {
                // 値を取得
                MSXML2::IXMLDOMNodePtr lightPollutionWinterValue = 0;
                BSTR val = SysAllocString(XPATH_aggregateData2);
                lightPollutionWinter->selectSingleNode(val, &lightPollutionWinterValue);
                if (NULL != lightPollutionWinterValue)
                {
                    // ノードタイプ取得
                    MSXML2::DOMNodeType eMemberNodeType;
                    hResult = lightPollutionWinterValue->get_nodeType(&eMemberNodeType);
                    if (FAILED(hResult))
                    {
                        assert(!"ノードタイプの取得に失敗");
                        continue;
                    }

                    // エレメント型への変換
                    MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                    hResult = lightPollutionWinterValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                    if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                    {
                        assert(!"エレメント型への変換に失敗");
                        continue;
                    }

                    // 値テキスト取得
                    BSTR valueText;
                    hResult = pXMLDOMMemberElement->get_text(&valueText);
                    if (SUCCEEDED(hResult))
                    {
                        // BSTR⇒std::string変換
                        std::string valueStr = ConvertBSTRToMBS(valueText);
                        buildingInfo.dLightPollutionWinter = stod(valueStr);

                    }
                }
            }

            // 建物リストに追加
            AgtBuildingList.push_back(buildingInfo);
        }
    }


    //xmlオブジェクト解放
    reader.Release();
    //COMの解放
    CoUninitialize();

    return AgtBuildingList;

}

// 集計データ作成
vector<string> GetAggregateData(string folderPath, string csvfile){

    // CSVファイル読み込み
    vector<vector<string> > data = csv2vector(csvfile, 1);
    // 適地判定結果配列
    vector<JUDGELIST> judgeList{};
    // 適地判定結果配列
    vector<JUDGESUITABLEPLACE> judgeRsultList{};
    // 適地判定
    JUDGELIST tmpJudgeList{};
    // 集計結果
    vector<string> result{};


    // データ設定
    int tmpMeshID = 0;
    for (int i = 0; i < data.size(); i++) {

        if (i == 0) {
            // 初回は値追加
            tmpJudgeList={};
            tmpMeshID = stoi(data[i][0]);
            vector<JUDGESUITABLEPLACE> judgeRsultList{};
        }
        else if (tmpMeshID != stoi(data[i][0])) {
            // メッシュIDが変更されたらリスト追加
            tmpJudgeList.meshID = tmpMeshID;
            tmpJudgeList.judgeSuitablePlaceList = judgeRsultList;
            judgeList.push_back(tmpJudgeList);
            JUDGELIST tmpJudgeList{};
            tmpMeshID = stoi(data[i][0]);
            judgeRsultList={};
        }

        if (data[i].size() == 14) {
            JUDGESUITABLEPLACE tmpJudge{};

            tmpJudge.strBuildingId = data[i][1];
            tmpJudge.priority = stoi(data[i][2]);
            tmpJudge.condition_1_1_1 = data[i][3];
            tmpJudge.condition_1_1_2 = data[i][4];
            tmpJudge.condition_1_2 = data[i][5];
            tmpJudge.condition_1_3 = data[i][6];
            tmpJudge.condition_2_1 = data[i][7];
            tmpJudge.condition_2_2 = data[i][8];
            tmpJudge.condition_2_3 = data[i][9];
            tmpJudge.condition_2_4 = data[i][10];
            tmpJudge.condition_3_1 = data[i][11];
            tmpJudge.condition_3_2 = data[i][12];
            tmpJudge.condition_3_3 = data[i][13];

            judgeRsultList.push_back(tmpJudge);

        }

    }
    // 最後のデータを設定
    if (judgeRsultList.size() > 0) {
        tmpJudgeList.meshID = tmpMeshID;
        tmpJudgeList.judgeSuitablePlaceList = judgeRsultList;
        judgeList.push_back(tmpJudgeList);

    }

    // データ取得
    std::vector<std::string> filePath;
    vector<AGTBLDGLIST> allBuiBldgAggregateList{};

    if (getFileNames(folderPath, extension_gml, filePath) == true) {

        // メッシュIDごとに集計元データ作成
        for (int i = 0; i < judgeList.size(); i++) {
            // ファイル数繰り返す
            for (auto& p : filePath) {
                vector<AGTBUILDING> result{};
                AGTBLDGLIST wbldgList{};

                // ファイル名からメッシュIDを取得
                std::string fullpath = p.c_str();
                int path_i = (int)fullpath.find_last_of("\\") + 1;
                int ext_i = (int)fullpath.find_last_of(".");
                std::string filename = fullpath.substr(path_i, (int64_t)ext_i - path_i);
                int meshId = stoi(filename.substr(0, 8));

                // メッシュIDが一致したらデータ作成
                if (meshId == judgeList[i].meshID) {
                    // メッシュIDセット
                    wbldgList.meshID = meshId;

                    //  Readerの作成
                    std::wstring oWString = StringToWString(p.c_str());

                    // XMLデータから固定XPathのデータを取得
                    result = GetBldgAggregateData(oWString, meshId, judgeList[i].judgeSuitablePlaceList);

                    // データがない場合は次のファイルへ移動
                    if (result.empty()) {
                        continue;
                    }
                    wbldgList.buildingList = result;

                    // データ追加
                    allBuiBldgAggregateList.push_back(wbldgList);

                    // 次のデータへ変更
                    break;
                }

            }

        }

    }

    // 範囲内建物数
    int building = 0;
    // 年間予測日射量総計
    double solorRadiationTotal = 0.0;
    // 年間予測発電量総計
    double electricGenerationTotal = 0.0;
    // 光害を発生させる建物数
    int lightPollutionBuilding = 0;
    // 光害発生時間総計（夏至）
    int lightPollutionSummerTotal = 0;
    // 光害発生時間総計（春分）
    int lightPollutionSplingTotal = 0;
    // 光害発生時間総計（冬至）
    int lightPollutionWinterTotal = 0;
    // 範囲内優先度1建物数
    int priorityLevel1Count = 0;
    // 範囲内優先度2建物数
    int priorityLevel2Count = 0;
    // 範囲内優先度3建物数
    int priorityLevel3Count = 0;
    // 範囲内優先度4建物数
    int priorityLevel4Count = 0;
    // 範囲内優先度5建物数
    int priorityLevel5Count = 0;
 

    // 集計
    if (allBuiBldgAggregateList.size() > 0) {
        // ヘッダー設定
        result.push_back(outputHeader);
        for (int i = 0; i < judgeList.size(); i++) {
            for (const auto& item : allBuiBldgAggregateList) {
            
                if (item.meshID == judgeList[i].meshID) {

                    for (const auto& item2 : item.buildingList) {
                        for (const auto& item3 : judgeList[i].judgeSuitablePlaceList) {
                            if (item2.strBuildingId == item3.strBuildingId) {
                                building++;
                                solorRadiationTotal = solorRadiationTotal + item2.dSolorRadiation;
                                electricGenerationTotal = electricGenerationTotal + item2.dElectricGeneration;
                                lightPollutionSummerTotal = lightPollutionSummerTotal + (int)item2.dLightPollutionSummer;
                                lightPollutionSplingTotal = lightPollutionSplingTotal + (int)item2.dLightPollutionSpling;
                                lightPollutionWinterTotal = lightPollutionWinterTotal + (int)item2.dLightPollutionWinter;
                                // 光害建物数
                                if (item2.dLightPollutionSummer + item2.dLightPollutionSpling + item2.dLightPollutionWinter > 0) {
                                    lightPollutionBuilding++;
                                }
                                // 優先度
                                if (item3.priority == priorityLevel1) {
                                    priorityLevel1Count++;
                                }
                                else if (item3.priority == priorityLevel2) {
                                    priorityLevel2Count++;
                                }
                                else if (item3.priority == priorityLevel3) {
                                    priorityLevel3Count++;
                                }
                                else if (item3.priority == priorityLevel4) {
                                    priorityLevel4Count++;
                                }
                                else if (item3.priority == priorityLevel5) {
                                    priorityLevel5Count++;
                                }

                                // 一致したら抜ける
                                break;
                            }

                        }
                    }
                }
            }
        }
        // 集計値設定
        std::ostringstream ss;
        ss << building << "," << solorRadiationTotal << "," << electricGenerationTotal << "," 
            << lightPollutionBuilding << "," << lightPollutionSummerTotal << "," << lightPollutionSplingTotal << "," 
            << lightPollutionWinterTotal << "," << priorityLevel1Count << "," << priorityLevel2Count << ","
            << priorityLevel3Count << "," << priorityLevel4Count << "," << priorityLevel5Count;
        std::string s = ss.str();
        result.push_back(s);
    }

    return result;

}

int __cdecl AggregateBldgFiles(const char* str, const char* strOut)
{
#ifdef _DEBUG
    std::cout << "AggregateBldgFiles Start " << std::endl;
    struct __timeb32 timebuffer;
    _ftime32_s(&timebuffer);
    double dStart = timebuffer.time + (double)timebuffer.millitm / 1000.0;
#endif

    if (strlen(str) == 0) {
        std::cout << "str is null." << std::endl;
        return 1;
    }
    if (strlen(strOut) == 0) {
        std::cout << "strOut is null." << std::endl;
        return 1;
    }
    std::vector<std::string> fileName;

    allList.clear();

    // キャンセルファイルパス
    std::filesystem::path path = std::filesystem::path(strOut) / CANCELFILE;
    std::string cancelPath = path.string();

    //ファイル名取得
    if (getFileNames(str, extension_gml, fileName) == true) {
    
        // ファイル数繰り返す
        for (auto& p : fileName) {
            vector<BUILDING> result{};
            BLDGLIST wbldgList{};

            // ファイル名からメッシュIDを取得
            std::string fullpath = p.c_str();
            int path_i = (int)fullpath.find_last_of("\\") + 1;
            int ext_i = (int)fullpath.find_last_of(".");
            std::string filename = fullpath.substr(path_i, (int64_t)ext_i - path_i);
            int meshId = stoi(filename.substr(0, 8));

            // メッシュIDセット
            wbldgList.meshID = meshId;

            //  Readerの作成
            std::wstring oWString = StringToWString(p.c_str());

            // XMLデータから固定XPathのデータを取得
            result = GetBldgAttribute(oWString);

            // データがない場合は次のファイルへ移動
            if (result.empty()) {
                continue;
            }
            wbldgList.buildingList = result;

            // データ追加
            allList.push_back(wbldgList);

            // キャンセルファイルチェック
            if (std::filesystem::exists(cancelPath)) {
                return 2;
            }
        }
    }
    else {
        return 1;
    }
#ifdef _DEBUG
    _ftime32_s(&timebuffer);
    double dEndStart = timebuffer.time + (double)timebuffer.millitm / 1000.0 - dStart;
    std::cout << "AggregateBldgFiles Time: " << dEndStart << " sec" << std::endl;
    dStart = timebuffer.time + (double)timebuffer.millitm / 1000.0;
#endif

    return 0;
}

// 集計処理
int __cdecl AggregateAllData(const char* str, const char* strOut) {


    // パスチェック
    if (strlen(str) == 0) {
        std::cout << "str is null." << std::endl;
        return 1;
    }
    if (strlen(strOut) == 0) {
        std::cout << "strOut is null." << std::endl;
        return 1;
    }

    std::vector<std::string> filePath;

    std::vector<std::string> result{};

    // キャンセルファイルパス
    std::filesystem::path path = std::filesystem::path(strOut).parent_path() / CANCELFILE;
    std::string cancelPath = path.string();

    // 参照パス設定
    std::filesystem::path path1 = std::filesystem::path(str) / "output" / "bldg" ;      // 3D都市データ
    std::filesystem::path path2 = std::filesystem::path(strOut).parent_path() / "data"; // 適地判定
    std::string bldgPath = path1.string();
    std::string csvPath = path2.string();

    // ファイル名取得
    if (getFileNames(csvPath, extension_csv, filePath) == true) {
    
        if (filePath.size() > 0) {
            for (auto& p : filePath) {

                // 適地判定ファイルがあれば処理を実行
                std::string strfile = p.c_str();
                std::filesystem::path filePath = strfile;
                if (filePath.filename() == judgeFile) {

                    result = GetAggregateData(bldgPath, p.c_str());

                    // csv出力する
                    // ファイル名に日付時刻を付与
                    string dateStr = getDatetimeStr();
                    std::filesystem::path outPath = std::filesystem::path(strOut) / outputFile;
                    ofstream ofs(outPath);
                    for (int i = 0; i < result.size(); i++) {
                        ofs << result[i] << endl;
                    }
                }

                // キャンセルファイルチェック
                if (std::filesystem::exists(cancelPath)) {
                    return 2;
                }
            }
        }
        else {
            return 1;
        }
    }
    else {
        return 1;
    }

    return 0;
}