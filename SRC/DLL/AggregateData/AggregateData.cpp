#include "pch.h"
#include "AggregateData.h"
#include "../../LIB/CommonUtil/CGeoUtil.h"
#include "../../LIB/CommonUtil/ReadINIParam.h"
#include "../../LIB/CommonUtil/CFileIO.h"
#include "../../LIB/CommonUtil/CFileUtil.h"
#include "../../LIB/CommonUtil/CPoint2DPolygon.h"
#include "../../LIB/CommonUtil/ExitCode.h"

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

void __cdecl Initialize()
{
    GetINIParam()->Initialize();
    JPZONE = GetINIParam()->GetJPZone();
}

void __cdecl DllDispose()
{
    allAreaList.clear();
    allAreaList.shrink_to_fit();
    hazardRiskData.fldRisks.clear();
    hazardRiskData.fldRisks.shrink_to_fit();
    hazardRiskData.tnmRisks.clear();
    hazardRiskData.tnmRisks.shrink_to_fit();
    hazardRiskData.lsldRisks.clear();
    hazardRiskData.lsldRisks.shrink_to_fit();
}

/// <summary>
/// 集計対象となる解析範囲データを追加
/// </summary>
/// <param name="p"></param>
/// <returns></returns>
void __cdecl AddAnalyzeAreaData(AnalyzeAreaData* p)
{
    // 解析範囲を設定
    std::vector<CPoint2D> areaPoints;
    // 構成点を取得
    int arrlength = p->nPointCount * 2;
    for (int j = 0; j < arrlength; j += 2)
    {
        // 緯度経度→平面直角座標系に変換
        double dX, dY;
        CGeoUtil::LonLatToXY(p->pPointArray[j + 1], p->pPointArray[j], JPZONE, dX, dY);
        areaPoints.emplace_back(CPoint2D(dX, dY));
    }

    AREADATA areaData{};
    areaData.areaID = p->strAreaId;
    areaData.areaName = p->strAreaName;
    areaData.pos2dList = areaPoints;

    // エリア範囲を凸多角形に分割する
    vector<CPoint2DPolygon> aryPolygons;
    CPoint2DPolygon areaPolygon;
    for (const auto& p : areaData.pos2dList)
    {
        CPoint2D pt2d = CPoint2D(p.x, p.y);
        areaPolygon.Add(pt2d);
    }
    // 左側を始点にする
    areaPolygon.StartLeft();

    // 凸多角形
    if (areaPolygon.IsConvexPolygon())
    {
        aryPolygons.emplace_back(areaPolygon);
    }
    // 凹多角形
    else
    {
        // 凸多角形に分割する
        areaPolygon.GetConvexPolygons(aryPolygons);
    }
    areaData.polygons = aryPolygons;

    allAreaList.emplace_back(areaData);
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
            file_names.emplace_back(entry.path().string());
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
        data.emplace_back(temp_data);
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

/// <summary>
/// 名前空間を設定
/// </summary>
/// <param name="xmldata"></param>
/// <returns></returns>
std::wstring GetSelectionNamespaces()
{
    std::wstring namespaces;

    // 名前空間の設定ファイルを読み込み
    std::wstring assetsDir = GetFUtil()->Combine(GetFUtil()->GetModulePathW(), L"Assets");
    std::wstring nsFile = GetFUtil()->Combine(assetsDir, L"CityGMLNameSpaces.txt");
    assert(GetFUtil()->IsExistPath(nsFile));

    CFileIO fio;
    if (fio.Open(nsFile, L"rt"))
    {
        std::wstring strLine;
        wchar_t cBuff[1024];

        while (fio.ReadLineW(cBuff, 1024) != NULL)
        {
            strLine = cBuff;
            if (strLine.front() == '#') continue;
            for (const auto& ns : namespaceKeys)
            {
                if (strLine.find(ns) == std::string::npos) continue;
                strLine.pop_back(); // 改行を削除
                namespaces += strLine + L" ";
            }
        }
        fio.Close();
    }
    namespaces.pop_back();

    return namespaces;

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
    //namespaceをサポートするように設定
    std::wstring ns = GetSelectionNamespaces();
    reader->setProperty(CComBSTR(L"SelectionNamespaces"), CComVariant(ns.c_str()));

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

        // 予測日射量
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
        else
        {
            continue;
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
                                    wSurfaceMembers.posList.emplace_back(pt);

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
                    roofSurfaces.roofSurfaceList.emplace_back(wSurfaceMembers);
                }
                // 屋根リストに追加
                buildingInfo.roofSurfaceList.emplace_back(roofSurfaces);
            }

        }

        // 建物リストに追加
        allBuildingList.emplace_back(buildingInfo);

    }


    //xmlオブジェクト解放
    reader.Release();

    return allBuildingList;

}

/// <summary>
/// メッシュのバウンディングを取得
/// </summary>
/// <returns></returns>
bool GetMeshBounding(wstring xmldata, POSITION& upperCorner, POSITION& lowerCorner)
{
    //xmlオブジェクト生成
    MSXML2::IXMLDOMDocument2Ptr reader;
    reader.CreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_INPROC_SERVER);

    //xpathをサポートするように設定
    reader->setProperty(CComBSTR(L"SelectionLanguage"), CComVariant(L"XPath"));
    //namespaceをサポートするように設定
    std::wstring ns = GetSelectionNamespaces();
    reader->setProperty(CComBSTR(L"SelectionNamespaces"), CComVariant(ns.c_str()));

    //ロード
    VARIANT_BOOL isSuccessful;
    reader->load(CComVariant(xmldata.c_str()), &isSuccessful);

    // 最小緯度経度取得
    HRESULT hBoundResult;
    MSXML2::IXMLDOMNodePtr lowerCornerNode = 0;
    BSTR bound = SysAllocString(BOUND_XPATH1);
    reader->selectSingleNode(bound, &lowerCornerNode);

    if (NULL != lowerCornerNode) {
        // ノードタイプ取得
        MSXML2::DOMNodeType eMemberNodeType;
        hBoundResult = lowerCornerNode->get_nodeType(&eMemberNodeType);
        if (FAILED(hBoundResult))
        {
            assert(!"ノードタイプの取得に失敗");
            return false;
        }
        else {
            // エレメント型への変換
            MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
            hBoundResult = lowerCornerNode->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
            if (FAILED(hBoundResult) || NULL == pXMLDOMMemberElement)
            {
                assert(!"エレメント型への変換に失敗");
                return false;
            }
            else {

                // 座標テキスト取得
                BSTR positionText;
                hBoundResult = pXMLDOMMemberElement->get_text(&positionText);
                if (SUCCEEDED(hBoundResult))
                {
                    // BSTR⇒std::string変換
                    std::string posStr = ConvertBSTRToMBS(positionText);

                    // 座標分割
                    vector<std::string> posAry = split(posStr, " ");

                    // 座標格納
                    int posCnt = 0;
                    // 一時座標リスト初期化
                    POSITION wPosition{};

                    for (int x = 0; x < posAry.size(); x++) {

                        if (posCnt == 0) {
                            // doubleに変換
                            wPosition.lat = std::stod(posAry[x]);

                            posCnt++;
                        }
                        else if (posCnt == 1) {
                            // doubleに変換
                            wPosition.lon = std::stod(posAry[x]);

                            posCnt++;
                        }
                        else if (posCnt == 2) {
                            // doubleに変換
                            wPosition.ht = std::stod(posAry[x]);

                            // 一時座標リスト初期化
                            POSITION wPosition{};
                            posCnt = 0;
                        }

                    }

                    // 値を設定
                    lowerCorner.lat = wPosition.lat;
                    lowerCorner.lon = wPosition.lon;
                    lowerCorner.ht = wPosition.ht;
                }
            }
        }
    }

    // 最大緯度経度取得
    MSXML2::IXMLDOMNodePtr upperCornerNode = 0;
    bound = SysAllocString(BOUND_XPATH2);
    reader->selectSingleNode(bound, &upperCornerNode);

    if (NULL != upperCornerNode) {
        // ノードタイプ取得
        MSXML2::DOMNodeType eMemberNodeType;
        hBoundResult = upperCornerNode->get_nodeType(&eMemberNodeType);
        if (FAILED(hBoundResult))
        {
            assert(!"ノードタイプの取得に失敗");
            return false;
        }
        else {
            // エレメント型への変換
            MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
            hBoundResult = upperCornerNode->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
            if (FAILED(hBoundResult) || NULL == pXMLDOMMemberElement)
            {
                assert(!"エレメント型への変換に失敗");
                return false;
            }
            else {

                // 座標テキスト取得
                BSTR positionText;
                hBoundResult = pXMLDOMMemberElement->get_text(&positionText);
                if (SUCCEEDED(hBoundResult))
                {
                    // BSTR⇒std::string変換
                    std::string posStr = ConvertBSTRToMBS(positionText);

                    // 座標分割
                    vector<std::string> posAry = split(posStr, " ");

                    // 座標格納
                    int posCnt = 0;
                    // 一時座標リスト初期化
                    POSITION wPosition{};

                    for (int x = 0; x < posAry.size(); x++) {

                        if (posCnt == 0) {
                            // doubleに変換
                            wPosition.lat = std::stod(posAry[x]);

                            posCnt++;
                        }
                        else if (posCnt == 1) {
                            // doubleに変換
                            wPosition.lon = std::stod(posAry[x]);

                            posCnt++;
                        }
                        else if (posCnt == 2) {
                            // doubleに変換
                            wPosition.ht = std::stod(posAry[x]);

                            // 一時座標リスト初期化
                            POSITION wPosition{};
                            posCnt = 0;
                        }

                    }

                    // 値を設定
                    upperCorner.lat = wPosition.lat;
                    upperCorner.lon = wPosition.lon;
                    upperCorner.ht = wPosition.ht;
                }
            }
        }
    }

    //xmlオブジェクト解放
    reader.Release();
    //COMの解放
    CoUninitialize();

    return true;
}

// 範囲がメッシュ範囲内にあるかどうかを判定
bool IsAreaInMesh(std::vector<CPoint2D> areaPoints, double meshMinX, double meshMinY, double meshMaxX, double meshMaxY)
{
    // メッシュ範囲
    vector<CPoint2D> point2DList{};
    point2DList.emplace_back(CPoint2D(meshMinX, meshMaxY));
    point2DList.emplace_back(CPoint2D(meshMinX, meshMinY));
    point2DList.emplace_back(CPoint2D(meshMaxX, meshMinY));
    point2DList.emplace_back(CPoint2D(meshMaxX, meshMaxY));
    CPoint2D* pt2D = &point2DList[0];

    for (auto& pt : areaPoints)
    {
        // 解析範囲の頂点で判定
        if (CGeoUtil::IsPointInPolygon(pt, point2DList.size(), pt2D))
        {
            return true;
        }
    }

    return false;
}

// 2D対象面の解析範囲内外判定
bool IsPolygonInArea(std::vector<CPoint2D> areaPoints, std::vector<CPoint2D> targetPoints, CPoint2DPolygon& crsPolygon)
{
    if (areaPoints.size() < 3)  return false;
    if (targetPoints.size() < 3)  return false;

    // ポリゴンを作成
    CPoint2DPolygon areaPolygon, targetPolygon;
    int areaPointsSize = (areaPoints[0] == areaPoints[areaPoints.size() - 1]) ? areaPoints.size() - 1 : areaPoints.size();   // 始点と終点が同じ場合は最後の点を追加しない
    for (int i = 0; i < areaPointsSize; i++)
    {
        areaPolygon.Add(areaPoints[i]);
    }
    int targetPointsSize = (targetPoints[0] == targetPoints[targetPoints.size() - 1]) ? targetPoints.size() - 1 : targetPoints.size();   // 始点と終点が同じ場合は最後の点を追加しない
    for (int i = 0; i < targetPointsSize; i++)
    {
        targetPolygon.Add(targetPoints[i]);
    }

    // 内外判定
    if (areaPolygon.GetCrossingPolygon(targetPolygon, crsPolygon))
    {
        return true;
    }

    return false;
}

// fldデータ取得
// in fld/prefフォルダ
std::vector<FLDRISKLIST> GetFldData(wstring folderPath, string cancelPath)
{
    std::vector<FLDRISKLIST> allRiskList{};

    std::vector<std::string> fileName;

    HRESULT hResult;
    hResult = CoInitialize(NULL); // 初期化

    // フォルダを取得
    std::filesystem::directory_iterator iter(folderPath), end;
    std::error_code err;
    for (; iter != end && !err; iter.increment(err)) {
        const std::filesystem::directory_entry entry = *iter;
        if (!entry.is_directory()) continue;

        std::filesystem::path typedir = entry.path();

        // 洪水浸水リスト
        FLDRISKLIST fldRisk{};

        // 洪水浸水想定区域ごとのフォルダ名を取得
        std::string dirname = typedir.filename().string();
        fldRisk.type = dirname;

        // サブフォルダを取得
        std::filesystem::directory_iterator subiter(typedir), subend;
        std::error_code suberr;
        for (; subiter != subend && !suberr; subiter.increment(suberr)) {
            const std::filesystem::directory_entry subentry = *subiter;
            if (!subentry.is_directory()) continue;

            std::filesystem::path subdir = subentry.path();

            // 洪水浸水想定区域図ごとのフォルダ名を取得
            std::string dirname = subdir.filename().string();
            fldRisk.description = dirname;

            // ファイル名取得
            if (getFileNames(subdir.string(), ".gml", fileName) == true) {

                std::vector<std::string> inAreaFiles;

                for (auto& p : fileName) {
                    // 洪水浸水想定データ
                    FLDRISK result{};

                    // ファイル名からメッシュIDを取得
                    std::string fullpath = p.c_str();
                    int path_i = (int)fullpath.find_last_of("\\") + 1;
                    int ext_i = (int)fullpath.find_last_of(".");
                    std::string filename = fullpath.substr(path_i, (int64_t)ext_i - path_i);
                    std::string meshId = filename.substr(0, 8);
                    result.meshID = meshId;

                    // ファイル名から規模を取得
                    std::vector<std::string> splitstr = split(filename, "_");
                    if (splitstr.size() < 4 || 5 < splitstr.size())
                    {
                        assert(!"対象外のCityGMLファイル名");
                        continue;
                    }
                    result.scale = splitstr[3];

                    // メッシュのバウンディングを取得
                    std::filesystem::path gmlfile = std::filesystem::path(fullpath);
                    if (!GetMeshBounding(gmlfile, result.upperCorner, result.lowerCorner))
                    {
                        continue;
                    }

                    // 平面直角座標に変換
                    double bbMaxX, bbMinX;
                    double bbMaxY, bbMinY;
                    CGeoUtil::LonLatToXY(result.lowerCorner.lon, result.lowerCorner.lat, JPZONE, bbMinX, bbMinY);
                    CGeoUtil::LonLatToXY(result.upperCorner.lon, result.upperCorner.lat, JPZONE, bbMaxX, bbMaxY);
                    vector<CPoint2D> vecMeshXY{
                        {(double)bbMinX, (double)bbMinY},
                        {(double)bbMinX, (double)bbMaxY},
                        {(double)bbMaxX, (double)bbMaxY},
                        {(double)bbMaxX, (double)bbMinY}
                    };

                    // ファイルの読み込み有無を判定
                    bool isInMesh = false;
                    for (auto& area : allAreaList)
                    {
                        if (area.areaID == "A000")
                        {
                            isInMesh = true;
                            break;
                        }

                        // 範囲の内外判定
                        for (const auto& poly : area.polygons)
                        {
                            CPoint2DPolygon tempPolygon;
                            if (IsPolygonInArea(vecMeshXY, poly, tempPolygon))
                            {
                                isInMesh = true;
                                break;
                            }
                        }
                        if (isInMesh) break;
                    }
                    if (!isInMesh)   continue;

                    //xmlオブジェクト生成
                    MSXML2::IXMLDOMDocument2Ptr reader;
                    reader.CreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_INPROC_SERVER);

                    //xpathをサポートするように設定
                    reader->setProperty(CComBSTR(L"SelectionLanguage"), CComVariant(L"XPath"));
                    //namespaceをサポートするように設定
                    std::wstring ns = GetSelectionNamespaces();
                    reader->setProperty(CComBSTR(L"SelectionNamespaces"), CComVariant(ns.c_str()));

                    //ロード
                    VARIANT_BOOL isSuccessful;
                    reader->load(CComVariant(p.c_str()), &isSuccessful);

                    BSTR xp2 = SysAllocString(XPATH2);
                    MSXML2::IXMLDOMNodeListPtr objList = NULL;
                    reader->selectNodes(xp2, &objList);

                    // ノード件数取得
                    long lCountNode = 0;
                    hResult = objList->get_length(&lCountNode);

                    for (int i = 0; i < lCountNode; i++) {

                        // ノードリストのうちの一つのノードの取得
                        MSXML2::IXMLDOMNodePtr pXMLDOMNode = NULL;
                        hResult = objList->get_item(i, &pXMLDOMNode);
                        if (FAILED(hResult))
                        {
                            assert(!"ノードリストのうちの一つのノードの取得に失敗");
                            continue;
                        }

                        // 個体情報取得(ノード)
                        BSTR wtr = SysAllocString(WTR_XPATH1);
                        MSXML2::IXMLDOMNodeListPtr waterBody = NULL;
                        pXMLDOMNode->selectNodes(wtr, &waterBody);

                        // ノード件数取得
                        long lWaterCountNode = 0;
                        hResult = waterBody->get_length(&lWaterCountNode);

                        // 個体数分繰り返す
                        for (int j = 0; j < lWaterCountNode; j++) {
                            // LOD1データ初期化
                            HAZARDRISKLOD1 hrLOD1{};

                            // ノードリストのうちの一つのノードの取得
                            MSXML2::IXMLDOMNodePtr pXMLDOMMemberNode = NULL;
                            hResult = waterBody->get_item(j, &pXMLDOMMemberNode);
                            if (FAILED(hResult))
                            {
                                assert(!"ノードリストのうちの一つのノードの取得に失敗");
                                continue;
                            }

                            // 詳細情報取得(ノード)
                            BSTR txp2 = SysAllocString(WTR_XPATH2);
                            MSXML2::IXMLDOMNodeListPtr surfaceMemberList = NULL;
                            pXMLDOMMemberNode->selectNodes(txp2, &surfaceMemberList);

                            // ノード件数取得
                            long lSurfaceMemberCountNode = 0;
                            hResult = surfaceMemberList->get_length(&lSurfaceMemberCountNode);

                            // 個体詳細数分繰り返す
                            for (int k = 0; k < lSurfaceMemberCountNode; k++) {
                                // ワーク壁詳細初期化
                                SURFACEMEMBERS wSurfaceMembers{};

                                // ノードリストのうちの一つのノードの取得
                                MSXML2::IXMLDOMNodePtr pXMLDOMSurfaceNode = NULL;
                                hResult = surfaceMemberList->get_item(k, &pXMLDOMSurfaceNode);
                                if (FAILED(hResult))
                                {
                                    assert(!"ノードリストのうちの一つのノードの取得に失敗");
                                    continue;
                                }

                                // ポリゴンタグ選択
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
                                                wSurfaceMembers.posList.emplace_back(pt);

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

                                // 詳細リストに追加
                                hrLOD1.wtrSurfaceList.emplace_back(wSurfaceMembers);
                            }

                            // LOD1リストに追加
                            result.fldListLOD1.emplace_back(hrLOD1);

                        }

                    }

                    fldRisk.vecFldRisk.emplace_back(result);

                    //xmlオブジェクト解放
                    reader.Release();

                    // キャンセルファイルチェック
                    if (std::filesystem::exists(cancelPath)) {
                        break;
                    }
                }

                // リストに追加
                allRiskList.emplace_back(fldRisk);

            }

            // キャンセルファイルチェック
            if (std::filesystem::exists(cancelPath)) {
                break;
            }
        }

    }

    //COMの解放
    CoUninitialize();

    return allRiskList;
}

// tnmデータ取得
std::vector<TNMRISKLIST> GetTnmData(wstring folderPath, string cancelPath)
{
    std::vector<TNMRISKLIST> allRiskList{};

    std::vector<std::string> fileName;

    // GMLファイル書き込み処理
    HRESULT hResult;
    hResult = CoInitialize(NULL); // 初期化

    // フォルダを取得
    std::filesystem::directory_iterator iter(folderPath), end;
    std::error_code err;
    for (; iter != end && !err; iter.increment(err)) {
        const std::filesystem::directory_entry entry = *iter;
        if (!entry.is_directory()) continue;

        std::filesystem::path path = entry.path();

        TNMRISKLIST tnmRisk{};

        // 洪水浸水想定区域図ごとのフォルダ名を取得
        std::string dirname = path.filename().string();
        tnmRisk.description = dirname;

        // ファイル名取得
        if (getFileNames(path.string(), ".gml", fileName) == true) {

            std::vector<std::string> inAreaFiles;

            for (auto& p : fileName) {
                TNMRISK result{};

                // ファイル名からメッシュIDを取得
                std::string fullpath = p.c_str();
                int path_i = (int)fullpath.find_last_of("\\") + 1;
                int ext_i = (int)fullpath.find_last_of(".");
                std::string filename = fullpath.substr(path_i, (int64_t)ext_i - path_i);
                std::string meshId = filename.substr(0, 6);
                result.meshID = meshId;

                // メッシュのバウンディングを取得
                std::filesystem::path gmlfile = std::filesystem::path(fullpath);
                if (!GetMeshBounding(gmlfile, result.upperCorner, result.lowerCorner))
                {
                    continue;
                }

                // 平面直角座標に変換
                double bbMaxX, bbMinX;
                double bbMaxY, bbMinY;
                CGeoUtil::LonLatToXY(result.lowerCorner.lon, result.lowerCorner.lat, JPZONE, bbMinX, bbMinY);
                CGeoUtil::LonLatToXY(result.upperCorner.lon, result.upperCorner.lat, JPZONE, bbMaxX, bbMaxY);
                vector<CPoint2D> vecMeshXY{
                    {(double)bbMinX, (double)bbMinY},
                    {(double)bbMinX, (double)bbMaxY},
                    {(double)bbMaxX, (double)bbMaxY},
                    {(double)bbMaxX, (double)bbMinY}
                };

                // ファイルの読み込み有無を判定
                bool isInMesh = false;
                for (auto& area : allAreaList)
                {
                    if (area.areaID == "A000")
                    {
                        isInMesh = true;
                        break;
                    }

                    // 範囲の内外判定
                    for (const auto& poly : area.polygons)
                    {
                        CPoint2DPolygon tempPolygon;
                        if (IsPolygonInArea(vecMeshXY, poly, tempPolygon))
                        {
                            isInMesh = true;
                            break;
                        }
                    }
                    if (isInMesh) break;
                }
                if (!isInMesh)   continue;

                //xmlオブジェクト生成
                MSXML2::IXMLDOMDocument2Ptr reader;
                reader.CreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_INPROC_SERVER);

                //xpathをサポートするように設定
                reader->setProperty(CComBSTR(L"SelectionLanguage"), CComVariant(L"XPath"));
                //namespaceをサポートするように設定
                std::wstring ns = GetSelectionNamespaces();
                reader->setProperty(CComBSTR(L"SelectionNamespaces"), CComVariant(ns.c_str()));

                //ロード
                VARIANT_BOOL isSuccessful;
                reader->load(CComVariant(p.c_str()), &isSuccessful);

                BSTR xp2 = SysAllocString(XPATH2);
                MSXML2::IXMLDOMNodeListPtr objList = NULL;
                reader->selectNodes(xp2, &objList);

                // ノード件数取得
                long lCountNode = 0;
                hResult = objList->get_length(&lCountNode);

                for (int i = 0; i < lCountNode; i++) {

                    // ノードリストのうちの一つのノードの取得
                    MSXML2::IXMLDOMNodePtr pXMLDOMNode = NULL;
                    hResult = objList->get_item(i, &pXMLDOMNode);
                    if (FAILED(hResult))
                    {
                        assert(!"ノードリストのうちの一つのノードの取得に失敗");
                        continue;
                    }

                    // 個体情報取得(ノード)
                    BSTR wtr = SysAllocString(WTR_XPATH1);
                    MSXML2::IXMLDOMNodeListPtr waterBody = NULL;
                    pXMLDOMNode->selectNodes(wtr, &waterBody);

                    // ノード件数取得
                    long lWaterCountNode = 0;
                    hResult = waterBody->get_length(&lWaterCountNode);

                    // 個体数分繰り返す
                    for (int j = 0; j < lWaterCountNode; j++) {
                        // LOD1データ初期化
                        HAZARDRISKLOD1 hrLOD1{};

                        // ノードリストのうちの一つのノードの取得
                        MSXML2::IXMLDOMNodePtr pXMLDOMMemberNode = NULL;
                        hResult = waterBody->get_item(j, &pXMLDOMMemberNode);
                        if (FAILED(hResult))
                        {
                            assert(!"ノードリストのうちの一つのノードの取得に失敗");
                            continue;
                        }

                        // 詳細情報取得(ノード)
                        BSTR txp2 = SysAllocString(WTR_XPATH2);
                        MSXML2::IXMLDOMNodeListPtr surfaceMemberList = NULL;
                        pXMLDOMMemberNode->selectNodes(txp2, &surfaceMemberList);

                        // ノード件数取得
                        long lSurfaceMemberCountNode = 0;
                        hResult = surfaceMemberList->get_length(&lSurfaceMemberCountNode);

                        // 個体詳細数分繰り返す
                        for (int k = 0; k < lSurfaceMemberCountNode; k++) {
                            // ワーク壁詳細初期化
                            SURFACEMEMBERS wSurfaceMembers{};

                            // ノードリストのうちの一つのノードの取得
                            MSXML2::IXMLDOMNodePtr pXMLDOMSurfaceNode = NULL;
                            hResult = surfaceMemberList->get_item(k, &pXMLDOMSurfaceNode);
                            if (FAILED(hResult))
                            {
                                assert(!"ノードリストのうちの一つのノードの取得に失敗");
                                continue;
                            }

                            // ポリゴンタグ選択
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
                                            wSurfaceMembers.posList.emplace_back(pt);

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

                            // 詳細リストに追加
                            hrLOD1.wtrSurfaceList.emplace_back(wSurfaceMembers);
                        }

                        // LOD1リストに追加
                        result.tnmRiskLOD1.emplace_back(hrLOD1);
                    }

                }

                tnmRisk.vecTnmRisk.emplace_back(result);

                //xmlオブジェクト解放
                reader.Release();

                // キャンセルファイルチェック
                if (std::filesystem::exists(cancelPath)) {
                    break;
                }

            }

        }

        // リストに追加
        allRiskList.emplace_back(tnmRisk);

        // キャンセルファイルチェック
        if (std::filesystem::exists(cancelPath)) {
            break;
        }
    }

    //COMの解放
    CoUninitialize();

    return allRiskList;
}

// lsldデータ取得
std::vector<LSLDRISK> GetLsldData(wstring folderPath, string cancelPath)
{
    std::vector<LSLDRISK> allRiskList{};

    std::vector<std::string> fileName;

    // GMLファイル書き込み処理
    HRESULT hResult;
    hResult = CoInitialize(NULL); // 初期化

    std::filesystem::path path = std::filesystem::path(folderPath);

    // ファイル名取得
    if (getFileNames(path.string(), ".gml", fileName) == true) {

        std::vector<std::string> inAreaFiles;

        for (auto& p : fileName) {
            LSLDRISK result{};

            // ファイル名からメッシュIDを取得
            std::string fullpath = p.c_str();
            int path_i = (int)fullpath.find_last_of("\\") + 1;
            int ext_i = (int)fullpath.find_last_of(".");
            std::string filename = fullpath.substr(path_i, (int64_t)ext_i - path_i);
            std::string meshId = filename.substr(0, 6);
            result.meshID = meshId;

            // メッシュのバウンディングを取得
            std::filesystem::path gmlfile = std::filesystem::path(fullpath);
            if (!GetMeshBounding(gmlfile, result.upperCorner, result.lowerCorner))
            {
                continue;
            }

            // 平面直角座標に変換
            double bbMaxX, bbMinX;
            double bbMaxY, bbMinY;
            CGeoUtil::LonLatToXY(result.lowerCorner.lon, result.lowerCorner.lat, JPZONE, bbMinX, bbMinY);
            CGeoUtil::LonLatToXY(result.upperCorner.lon, result.upperCorner.lat, JPZONE, bbMaxX, bbMaxY);
            vector<CPoint2D> vecMeshXY{
                {(double)bbMinX, (double)bbMinY},
                {(double)bbMinX, (double)bbMaxY},
                {(double)bbMaxX, (double)bbMaxY},
                {(double)bbMaxX, (double)bbMinY}
            };

            // ファイルの読み込み有無を判定
            bool isInMesh = false;
            for (auto& area : allAreaList)
            {
                if (area.areaID == "A000")
                {
                    isInMesh = true;
                    break;
                }

                // 範囲の内外判定
                for (const auto& poly : area.polygons)
                {
                    CPoint2DPolygon tempPolygon;
                    if (IsPolygonInArea(vecMeshXY, poly, tempPolygon))
                    {
                        isInMesh = true;
                        break;
                    }
                }

                if (isInMesh) break;
            }
            if (!isInMesh)   continue;

            //xmlオブジェクト生成
            MSXML2::IXMLDOMDocument2Ptr reader;
            reader.CreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_INPROC_SERVER);

            //xpathをサポートするように設定
            reader->setProperty(CComBSTR(L"SelectionLanguage"), CComVariant(L"XPath"));
            //namespaceをサポートするように設定
            std::wstring ns = GetSelectionNamespaces();
            reader->setProperty(CComBSTR(L"SelectionNamespaces"), CComVariant(ns.c_str()));

            //ロード
            VARIANT_BOOL isSuccessful;
            reader->load(CComVariant(p.c_str()), &isSuccessful);

            BSTR xp2 = SysAllocString(XPATH2);
            MSXML2::IXMLDOMNodeListPtr objList = NULL;
            reader->selectNodes(xp2, &objList);

            // ノード件数取得
            long lCountNode = 0;
            hResult = objList->get_length(&lCountNode);

            for (int i = 0; i < lCountNode; i++) {

                // ノードリストのうちの一つのノードの取得
                MSXML2::IXMLDOMNodePtr pXMLDOMNode = NULL;
                hResult = objList->get_item(i, &pXMLDOMNode);
                if (FAILED(hResult))
                {
                    assert(!"ノードリストのうちの一つのノードの取得に失敗");
                    continue;
                }

                // 個体情報取得(ノード)
                BSTR lsld = SysAllocString(LSLD_XPATH1);
                MSXML2::IXMLDOMNodeListPtr area = NULL;
                pXMLDOMNode->selectNodes(lsld, &area);

                // ノード件数取得
                long lAreaCountNode = 0;
                hResult = area->get_length(&lAreaCountNode);

                // 個体数分繰り返す
                for (int j = 0; j < lAreaCountNode; j++) {
                    // LOD1データ初期化
                    HAZARDRISKLOD1 hrLOD1{};

                    // ノードリストのうちの一つのノードの取得
                    MSXML2::IXMLDOMNodePtr pXMLDOMMemberNode = NULL;
                    hResult = area->get_item(j, &pXMLDOMMemberNode);
                    if (FAILED(hResult))
                    {
                        assert(!"ノードリストのうちの一つのノードの取得に失敗");
                        continue;
                    }

                    // 詳細情報取得(ノード)
                    BSTR txp2 = SysAllocString(LSLD_XPATH2);
                    MSXML2::IXMLDOMNodeListPtr surfaceMemberList = NULL;
                    pXMLDOMMemberNode->selectNodes(txp2, &surfaceMemberList);

                    // ノード件数取得
                    long lSurfaceMemberCountNode = 0;
                    hResult = surfaceMemberList->get_length(&lSurfaceMemberCountNode);

                    // 個体詳細数分繰り返す
                    for (int k = 0; k < lSurfaceMemberCountNode; k++) {
                        // ワーク壁詳細初期化
                        SURFACEMEMBERS wSurfaceMembers{};

                        // ノードリストのうちの一つのノードの取得
                        MSXML2::IXMLDOMNodePtr pXMLDOMSurfaceNode = NULL;
                        hResult = surfaceMemberList->get_item(k, &pXMLDOMSurfaceNode);
                        if (FAILED(hResult))
                        {
                            assert(!"ノードリストのうちの一つのノードの取得に失敗");
                            continue;
                        }

                        // ポリゴンタグ選択
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
                                        wSurfaceMembers.posList.emplace_back(pt);

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

                        // 詳細リストに追加
                        hrLOD1.wtrSurfaceList.emplace_back(wSurfaceMembers);
                    }

                    // LOD1リストに追加
                    result.lsldRiskLOD1.emplace_back(hrLOD1);
                }

            }

            // リストに追加
            allRiskList.emplace_back(result);

            //xmlオブジェクト解放
            reader.Release();


            // キャンセルファイルチェック
            if (std::filesystem::exists(cancelPath)) {
                break;
            }
        }

    }

    //COMの解放
    CoUninitialize();

    return allRiskList;
}

void SetBldgHazardRisk(std::string meshId, vector<BUILDING>& result)
{

    // 建物ごとに災害リスクの値を設定
    for (auto& bldg : result)
    {   
        bool allComplete = false;

        for (const auto& surface : bldg.roofSurfaceList)
        {
            for (const auto& bldMember : surface.roofSurfaceList)
            {
                for (const auto& pos : bldMember.posList)
                {
                    CPoint2D pt2D(pos.x, pos.y);

                    // 洪水浸水深
                    if (hazardRiskData.fldRisks.size() > 0)
                    {
                        double dMaxDepth = 0.0;
                        for (const auto& fldList : hazardRiskData.fldRisks)
                        {
                            for (const auto& fld : fldList.vecFldRisk)
                            {
                                if (fld.meshID.find(meshId) == std::string::npos) continue;

                                bool isMaxDepth = false;    // 最大深かどうか
                                for (const auto& lod1 : fld.fldListLOD1)
                                {
                                    for (const auto& member : lod1.wtrSurfaceList)
                                    {
                                        // 内外判定用
                                        vector<CPoint2D> area2DList{};
                                        for (auto& pos : member.posList)
                                        {
                                            CPoint2D p2D = CPoint2D(pos.x, pos.y);
                                            area2DList.emplace_back(p2D);
                                        }
                                        CPoint2D* ptArea2D = &area2DList[0];
                                        int ptSize = (int)area2DList.size();

                                        // LOD1内に建物LOD2があるか
                                        if (CGeoUtil::IsPointInPolygon(pt2D, ptSize, ptArea2D))
                                        {
                                            dMaxDepth = (dMaxDepth < pos.z) ? pos.z : dMaxDepth;
                                            if (fld.upperCorner.ht == pos.z)
                                            {
                                                isMaxDepth = true;
                                                break;
                                            }
                                        }
                                    }
                                    if (isMaxDepth) break; // 最大深の場合は次のデータを処理
                                }
                                if (isMaxDepth) break;
                            }

                            // 洪水浸水深(最大値)を設定
                            bldg.dFloodDepth = dMaxDepth;

                            if (dMaxDepth != 0.0) break;
                        }
                    }

                    // 津波浸水深
                    if (hazardRiskData.tnmRisks.size() > 0)
                    {
                        double dMaxDepth = 0.0;
                        for (const auto& tnmList : hazardRiskData.tnmRisks)
                        {
                            for (const auto& tnm : tnmList.vecTnmRisk)
                            {
                                if (tnm.meshID.find(meshId) == std::string::npos) continue;

                                bool isMaxDepth = false;    // 最大深かどうか
                                for (const auto& lod1 : tnm.tnmRiskLOD1)
                                {
                                    for (const auto& member : lod1.wtrSurfaceList)
                                    {
                                        // 内外判定用
                                        vector<CPoint2D> area2DList{};
                                        for (auto& pos : member.posList)
                                        {
                                            CPoint2D p2D = CPoint2D(pos.x, pos.y);
                                            area2DList.emplace_back(p2D);
                                        }
                                        CPoint2D* ptArea2D = &area2DList[0];
                                        int ptSize = (int)area2DList.size();

                                        // LOD1内に建物LOD2があるか
                                        if (CGeoUtil::IsPointInPolygon(pt2D, ptSize, ptArea2D))
                                        {
                                            dMaxDepth = (dMaxDepth < pos.z) ? pos.z : dMaxDepth;
                                            if (tnm.upperCorner.ht == pos.z)
                                            {
                                                isMaxDepth = true;
                                                break;
                                            }
                                        }
                                    }
                                    if (isMaxDepth) break; // 最大深の場合は次のデータを処理
                                }
                                if (isMaxDepth) break;
                            }

                            // 津波浸水深(最大値)を設定
                            bldg.dTsunamiHeight = dMaxDepth;

                            if (dMaxDepth != 0.0) break;
                        }
                    }

                    // 土砂災害区域
                    if (hazardRiskData.lsldRisks.size() > 0)
                    {
                        double inside = false;
                        for (const auto& lsld : hazardRiskData.lsldRisks)
                        {
                            if (lsld.meshID.find(meshId) == std::string::npos) continue;

                            for (const auto& lod1 : lsld.lsldRiskLOD1)
                            {
                                for (const auto& member : lod1.wtrSurfaceList)
                                {
                                    for (auto& pos : member.posList)
                                    {
                                        // 内外判定用
                                        vector<CPoint2D> area2DList{};
                                        for (auto& pos : member.posList)
                                        {
                                            CPoint2D p2D = CPoint2D(pos.x, pos.y);
                                            area2DList.emplace_back(p2D);
                                        }
                                        CPoint2D* ptArea2D = &area2DList[0];
                                        int ptSize = (int)area2DList.size();

                                        // LOD1内に建物LOD2があるか
                                        if (CGeoUtil::IsPointInPolygon(pt2D, ptSize, ptArea2D))
                                        {
                                            inside = true;
                                            break;
                                        }

                                    }
                                    if (inside) break;  // 最大深の場合は次のデータを処理
                                }
                                if (inside) break;
                            }
                            if (inside) break;
                        }

                        // 洪水浸水深(最大値)を設定
                        bldg.bLandslideArea = inside;
                    }

                    if (bldg.dFloodDepth != 0.0 && bldg.dTsunamiHeight != 0.0 && bldg.bLandslideArea)
                    {
                        // すべて設定済み
                        allComplete = true;
                        break;
                    }
                }
                if (allComplete) break;
            }
            if (allComplete) break;
        }

    }

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
    std::wstring ns = GetSelectionNamespaces();
    reader->setProperty(CComBSTR(L"SelectionNamespaces"), CComVariant(ns.c_str()));

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
            

            // 予測日射量
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

            // 予測発電量
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

            // 光害発生時間(指定日)
            MSXML2::IXMLDOMNodePtr lightPollutionOneday = 0;
            BSTR oneday = SysAllocString(XPATH_aggregateData7);
            pXMLDOMNode->selectSingleNode(oneday, &lightPollutionOneday);
            if (NULL != lightPollutionOneday)
            {
                // 値を取得
                MSXML2::IXMLDOMNodePtr lightPollutionOneDayValue = 0;
                BSTR val = SysAllocString(XPATH_aggregateData2);
                lightPollutionWinter->selectSingleNode(val, &lightPollutionOneDayValue);
                if (NULL != lightPollutionOneDayValue)
                {
                    // ノードタイプ取得
                    MSXML2::DOMNodeType eMemberNodeType;
                    hResult = lightPollutionOneDayValue->get_nodeType(&eMemberNodeType);
                    if (FAILED(hResult))
                    {
                        assert(!"ノードタイプの取得に失敗");
                        continue;
                    }

                    // エレメント型への変換
                    MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                    hResult = lightPollutionOneDayValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
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
                        buildingInfo.dLightPollutionOneDay = stod(valueStr);

                    }
                }
            }

            // 建物リストに追加
            AgtBuildingList.emplace_back(buildingInfo);
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

    // 集計結果
    vector<string> result{};
    // ヘッダー設定
    result.emplace_back(outputHeader);

    // エリアごとに集計
    for (const auto& area : allAreaList)
    {
        std::string areaDirName = CStringEx::Format("%s_%s", area.areaID.c_str(), area.areaName.c_str());
        if (area.areaName == "")	areaDirName.pop_back();	// 名称が無い場合は末尾の"_"を削除
        std::filesystem::path areadir = std::filesystem::path(folderPath) / areaDirName;
        if (!std::filesystem::exists(areadir))  continue;

        std::string areapath = areadir.string();

        // 適地判定結果配列
        vector<JUDGELIST> judgeList{};
        // 適地判定結果配列
        vector<JUDGESUITABLEPLACE> judgeRsultList{};
        // 適地判定
        JUDGELIST tmpJudgeList{};

        // データ設定
        int tmpMeshID = 0;
        for (int i = 0; i < data.size(); i++) {
            if (data[i][0] != area.areaID)  continue;

            if (i == 0) {
                // 初回は値追加
                tmpJudgeList = {};
                tmpMeshID = stoi(data[i][1]);
                vector<JUDGESUITABLEPLACE> judgeRsultList{};
            }
            else if (tmpMeshID != stoi(data[i][1])) {
                // メッシュIDが変更されたらリスト追加
                tmpJudgeList.meshID = tmpMeshID;
                tmpJudgeList.judgeSuitablePlaceList = judgeRsultList;
                judgeList.emplace_back(tmpJudgeList);
                JUDGELIST tmpJudgeList{};
                tmpMeshID = stoi(data[i][1]);
                judgeRsultList = {};
            }

            if (data[i].size() == 15) {
                JUDGESUITABLEPLACE tmpJudge{};

                tmpJudge.strBuildingId = data[i][2];
                tmpJudge.priority = stoi(data[i][3]);
                tmpJudge.condition_1_1_1 = data[i][4];
                tmpJudge.condition_1_1_2 = data[i][5];
                tmpJudge.condition_1_2 = data[i][6];
                tmpJudge.condition_1_3 = data[i][7];
                tmpJudge.condition_2_1 = data[i][8];
                tmpJudge.condition_2_2 = data[i][9];
                tmpJudge.condition_2_3 = data[i][10];
                tmpJudge.condition_2_4 = data[i][11];
                tmpJudge.condition_3_1 = data[i][12];
                tmpJudge.condition_3_2 = data[i][13];
                tmpJudge.condition_3_3 = data[i][14];

                judgeRsultList.emplace_back(tmpJudge);

            }

        }
        // 最後のデータを設定
        if (judgeRsultList.size() > 0) {
            tmpJudgeList.meshID = tmpMeshID;
            tmpJudgeList.judgeSuitablePlaceList = judgeRsultList;
            judgeList.emplace_back(tmpJudgeList);

        }

        // データ取得
        std::vector<std::string> filePath;
        vector<AGTBLDGLIST> allBuiBldgAggregateList{};

        if (getFileNames(areapath, extension_gml, filePath) == true) {

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
                        allBuiBldgAggregateList.emplace_back(wbldgList);

                        // 次のデータへ変更
                        break;
                    }

                }

            }

        }

        // 範囲内建物数
        int building = 0;
        // 予測日射量総計
        double solorRadiationTotal = 0.0;
        // 予測発電量総計
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
            ss << area.areaID.c_str() << "," << building << "," << solorRadiationTotal << "," << electricGenerationTotal << ","
                << lightPollutionBuilding << "," << lightPollutionSummerTotal << "," << lightPollutionSplingTotal << ","
                << lightPollutionWinterTotal << "," << priorityLevel1Count << "," << priorityLevel2Count << ","
                << priorityLevel3Count << "," << priorityLevel4Count << "," << priorityLevel5Count;
            std::string s = ss.str();
            result.emplace_back(s);
        }

    }

    return result;

}

int __cdecl AnalyzeHazardRisk(const char* str, const char* strOut, bool fldrisk, bool tnmrisk, bool lsldrisk)
{
#ifdef _DEBUG
    std::cout << "AnalyzeHazardRisk Start " << std::endl;
    struct __timeb32 timebuffer;
    _ftime32_s(&timebuffer);
    double dStart = timebuffer.time + (double)timebuffer.millitm / 1000.0;
#endif

    if (strlen(str) == 0) {
        std::cout << "str is null." << std::endl;
        return (int)eExitCode::Error;
    }
    if (strlen(strOut) == 0) {
        std::cout << "strOut is null." << std::endl;
        return (int)eExitCode::Error;
    }
    std::vector<std::string> fileName;

    // キャンセルファイルパス
    std::filesystem::path path = std::filesystem::path(strOut) / CANCELFILE;
    std::string cancelPath = path.string();

    HAZARDRISK riskData{};
    vector<FLDRISKLIST> fldList{};
    vector<TNMRISKLIST> tnmList{};
    vector<LSLDRISK> lsldList{};

    // 洪水浸水想定データ読み込み
    if (fldrisk)
    {
        std::filesystem::path fldpath = std::filesystem::path(str) / "fld";
        std::wstring flddir = fldpath.wstring();
        if (std::filesystem::exists(flddir))
        {
            fldList = GetFldData(flddir, cancelPath);
        }
    }

    // 津波浸水想定データ読み込み
    if (tnmrisk)
    {
        std::filesystem::path tnmpath = std::filesystem::path(str) / "tnm";
        std::wstring tnmdir = tnmpath.wstring();
        if (std::filesystem::exists(tnmdir))
        {
            tnmList = GetTnmData(tnmdir, cancelPath);
        }
    }

    // 土砂災害データ読み込み
    if (lsldrisk)
    {
        std::filesystem::path lsldpath = std::filesystem::path(str) / "lsld";
        std::wstring lslddir = lsldpath.wstring();
        if (std::filesystem::exists(lslddir))
        {
            lsldList = GetLsldData(lsldpath, cancelPath);
        }
    }

    riskData.fldRisks = fldList;
    riskData.tnmRisks = tnmList;
    riskData.lsldRisks = lsldList;

    hazardRiskData = riskData;

#ifdef _DEBUG
    _ftime32_s(&timebuffer);
    double dEndStart = timebuffer.time + (double)timebuffer.millitm / 1000.0 - dStart;
    std::cout << "AnalyzeHazardRisk Time: " << dEndStart << " sec" << std::endl;
    dStart = timebuffer.time + (double)timebuffer.millitm / 1000.0;
#endif

    return 0;
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
        return (int)eExitCode::Error;
    }
    if (strlen(strOut) == 0) {
        std::cout << "strOut is null." << std::endl;
        return (int)eExitCode::Error;
    }
    std::vector<std::string> fileName;

    //allList.clear();

    // キャンセルファイルパス
    std::filesystem::path path = std::filesystem::path(strOut) / CANCELFILE;
    std::string cancelPath = path.string();

    // エリアごとに建物を取得
    for (auto& area : allAreaList)
    {
        area.buildList.clear();

        std::string areaDirName = CStringEx::Format("%s_%s", area.areaID.c_str(), area.areaName.c_str());
        if (area.areaName == "")	areaDirName.pop_back();	// 名称が無い場合は末尾の"_"を削除
        std::filesystem::path areadir = std::filesystem::path(str) / areaDirName;
        if (!std::filesystem::exists(areadir))  continue;

        std::string areapath = areadir.string();

        //ファイル名取得
        if (getFileNames(areapath, extension_gml, fileName) == true) {

            // ファイル数繰り返す
            for (auto& p : fileName) {
                vector<BUILDING> result{};
                BLDGLIST wbldgList{};

                // ファイル名からメッシュIDを取得
                std::string fullpath = p.c_str();
                int path_i = (int)fullpath.find_last_of("\\") + 1;
                int ext_i = (int)fullpath.find_last_of(".");
                std::string filename = fullpath.substr(path_i, (int64_t)ext_i - path_i);
                std::string meshId = filename.substr(0, 8);

                // メッシュIDセット
                wbldgList.meshID = stoi(meshId);

                //  Readerの作成
                std::wstring oWString = StringToWString(p.c_str());

                // XMLデータから固定XPathのデータを取得
                result = GetBldgAttribute(oWString);

                // データがない場合は次のファイルへ移動
                if (result.empty()) {
                    continue;
                }
                
                // 災害リスクから洪水浸水深、津波浸水深、土砂災害区域を設定
                SetBldgHazardRisk(meshId, result);

                wbldgList.buildingList = result;

                // データ追加
                area.buildList.emplace_back(wbldgList);

                // キャンセルファイルチェック
                if (std::filesystem::exists(cancelPath)) {
                    return (int)eExitCode::Cancel;
                }
            }
        }
        else {
            return (int)eExitCode::Error;
        }
    }

#ifdef _DEBUG
    _ftime32_s(&timebuffer);
    double dEndStart = timebuffer.time + (double)timebuffer.millitm / 1000.0 - dStart;
    std::cout << "AggregateBldgFiles Time: " << dEndStart << " sec" << std::endl;
    dStart = timebuffer.time + (double)timebuffer.millitm / 1000.0;
#endif

    return (int)eExitCode::Normal;
}

int __cdecl AggregateLandFiles(const char* str, const char* strOut)
{
#ifdef _DEBUG
    std::cout << "AggregateLandFiles Start " << std::endl;
    struct __timeb32 timebuffer;
    _ftime32_s(&timebuffer);
    double dStart = timebuffer.time + (double)timebuffer.millitm / 1000.0;
#endif

    if (strlen(str) == 0) {
        std::cout << "str is null." << std::endl;
        return (int)eExitCode::Error;
    }
    if (strlen(strOut) == 0) {
        std::cout << "strOut is null." << std::endl;
        return (int)eExitCode::Error;
    }
    std::vector<std::string> fileName;

    // キャンセルファイルパス
    std::filesystem::path path = std::filesystem::path(strOut) / CANCELFILE;
    std::string cancelPath = path.string();

    // 読み込むファイル
    std::filesystem::path csvpath = std::filesystem::path(str) / landAnalyzeFile;
    std::string csvfile = csvpath.string();

    // CSVファイル読み込み
    vector<vector<string> > data = csv2vector(csvfile, 1);

    // エリアごとに結果を取得
    for (auto& area : allAreaList)
    {
        LANDSURFACE result{};

        // データ設定
        for (int i = 0; i < data.size(); i++) {
            if (data[i].size() == 10) {

                std::string areaID = data[i][0];
                if (area.areaID == areaID)
                {
                    result.dSolorRadiation = stof(data[i][2]);
                    result.dLandHeight = stof(data[i][9]);
                    break;
                }
            }
        }

        // 内外判定用
        CPoint2D* ptArea2D = &area.pos2dList[0];
        int ptSize = (int)area.pos2dList.size();

        // 災害リスクから洪水浸水深、津波浸水深、土砂災害区域を設定
        // 洪水浸水深
        if (hazardRiskData.fldRisks.size() > 0)
        {
            double dMaxDepth = 0.0;
            for (const auto& fldList : hazardRiskData.fldRisks)
            {
                for (const auto& fld : fldList.vecFldRisk)
                {
                    // 平面直角座標に変換
                    double bbMaxX, bbMinX;
                    double bbMaxY, bbMinY;
                    CGeoUtil::LonLatToXY(fld.lowerCorner.lon, fld.lowerCorner.lat, JPZONE, bbMinX, bbMinY);
                    CGeoUtil::LonLatToXY(fld.upperCorner.lon, fld.upperCorner.lat, JPZONE, bbMaxX, bbMaxY);
                    if (!IsAreaInMesh(area.pos2dList, bbMinX, bbMinY, bbMaxX, bbMaxY))    continue;

                    bool isMaxDepth = false;    // 最大深かどうか
                    for (const auto& lod1 : fld.fldListLOD1)
                    {
                        for (const auto& member : lod1.wtrSurfaceList)
                        {
                            for (auto& pos : member.posList)
                            {
                                CPoint2D pt2D(pos.x, pos.y);
                                if (CGeoUtil::IsPointInPolygon(pt2D, ptSize, ptArea2D))
                                {
                                    dMaxDepth = (dMaxDepth < pos.z) ? pos.z : dMaxDepth;
                                    if (fld.upperCorner.ht == pos.z)
                                    {
                                        isMaxDepth = true;
                                        break;
                                    }
                                }
                            }
                            if (isMaxDepth) break;  // 最大深の場合は次のデータを処理
                        }
                        if (isMaxDepth) break;
                    }
                    if (isMaxDepth) break;
                }
            }

            // 洪水浸水深(最大値)を設定
            result.dFloodDepth = dMaxDepth;
        }

        // 洪水浸水深
        if (hazardRiskData.tnmRisks.size() > 0)
        {
            double dMaxDepth = 0.0;
            for (const auto& tnmList : hazardRiskData.tnmRisks)
            {
                for (const auto& tnm : tnmList.vecTnmRisk)
                {
                    // 平面直角座標に変換
                    double bbMaxX, bbMinX;
                    double bbMaxY, bbMinY;
                    CGeoUtil::LonLatToXY(tnm.lowerCorner.lon, tnm.lowerCorner.lat, JPZONE, bbMinX, bbMinY);
                    CGeoUtil::LonLatToXY(tnm.upperCorner.lon, tnm.upperCorner.lat, JPZONE, bbMaxX, bbMaxY);
                    if (!IsAreaInMesh(area.pos2dList, bbMinX, bbMinY, bbMaxX, bbMaxY))    continue;

                    bool isMaxDepth = false;    // 最大深かどうか
                    for (const auto& lod1 : tnm.tnmRiskLOD1)
                    {
                        for (const auto& member : lod1.wtrSurfaceList)
                        {
                            for (auto& pos : member.posList)
                            {
                                CPoint2D pt2D(pos.x, pos.y);
                                if (CGeoUtil::IsPointInPolygon(pt2D, ptSize, ptArea2D))
                                {
                                    dMaxDepth = (dMaxDepth < pos.z) ? pos.z : dMaxDepth;
                                    if (tnm.upperCorner.ht == pos.z)
                                    {
                                        isMaxDepth = true;
                                        break;
                                    }
                                }
                            }
                            if (isMaxDepth) break;  // 最大深の場合は次のデータを処理
                        }
                        if (isMaxDepth) break;
                    }
                    if (isMaxDepth) break;
                }
            }

            // 洪水浸水深(最大値)を設定
            result.dTsunamiHeight = dMaxDepth;
        }

        // 土砂災害区域
        if (hazardRiskData.lsldRisks.size() > 0)
        {
            double inside = false;
            for (const auto& lsld : hazardRiskData.lsldRisks)
            {
                // 平面直角座標に変換
                double bbMaxX, bbMinX;
                double bbMaxY, bbMinY;
                CGeoUtil::LonLatToXY(lsld.lowerCorner.lon, lsld.lowerCorner.lat, JPZONE, bbMinX, bbMinY);
                CGeoUtil::LonLatToXY(lsld.upperCorner.lon, lsld.upperCorner.lat, JPZONE, bbMaxX, bbMaxY);
                if (!IsAreaInMesh(area.pos2dList, bbMinX, bbMinY, bbMaxX, bbMaxY))    continue;

                for (const auto& lod1 : lsld.lsldRiskLOD1)
                {
                    for (const auto& member : lod1.wtrSurfaceList)
                    {
                        for (auto& pos : member.posList)
                        {
                            CPoint2D pt2D(pos.x, pos.y);
                            if (CGeoUtil::IsPointInPolygon(pt2D, ptSize, ptArea2D))
                            {
                                inside = true;
                                break;
                            }
                        }
                        if (inside) break;  // 最大深の場合は次のデータを処理
                    }
                    if (inside) break;
                }
                if (inside) break;
            }

            // 洪水浸水深(最大値)を設定
            result.bLandslideArea = inside;
        }

        area.landData = result;

    }

#ifdef _DEBUG
    _ftime32_s(&timebuffer);
    double dEndStart = timebuffer.time + (double)timebuffer.millitm / 1000.0 - dStart;
    std::cout << "AggregateLandFiles Time: " << dEndStart << " sec" << std::endl;
    dStart = timebuffer.time + (double)timebuffer.millitm / 1000.0;
#endif

    return (int)eExitCode::Normal;
}

// 集計処理
int __cdecl AggregateAllData(const char* str, const char* strOut) {


    // パスチェック
    if (strlen(str) == 0) {
        std::cout << "str is null." << std::endl;
        return (int)eExitCode::Error;
    }
    if (strlen(strOut) == 0) {
        std::cout << "strOut is null." << std::endl;
        return (int)eExitCode::Error;
    }

    std::vector<std::string> filePath;

    std::vector<std::string> result{};

    // キャンセルファイルパス
    std::filesystem::path path = std::filesystem::path(strOut).parent_path() / CANCELFILE;
    std::string cancelPath = path.string();

    // 参照パス設定
    std::filesystem::path path1 = std::filesystem::path(str) / "citygml" / "bldg";  // 解析結果のgmlデータ
    std::filesystem::path path2 = std::filesystem::path(strOut) / "適地判定";       // 適地判定
    std::string bldgPath = path1.string();
    std::string csvPath = path2.string();

    // ファイル名取得
    if (getFileNames(csvPath, extension_csv, filePath) == true) {

        if (filePath.size() > 0) {
            for (auto& p : filePath) {

                // 適地判定ファイルがあれば処理を実行
                std::string strfile = p.c_str();
                std::filesystem::path filePath = strfile;

                // 建物
                if (filePath.filename() == judgeFile) {

                    result = GetAggregateData(bldgPath, p.c_str());

                    // csv出力する
                    // ファイル名に日付時刻を付与
                    string dateStr = getDatetimeStr();
                    std::filesystem::path outPath = std::filesystem::path(strOut) / "集計" / outputFile;
                    ofstream ofs(outPath);
                    for (int i = 0; i < result.size(); i++) {
                        ofs << result[i] << endl;
                    }
                }

                // キャンセルファイルチェック
                if (std::filesystem::exists(cancelPath)) {
                    return (int)eExitCode::Cancel;
                }
            }

        }
        else {
            return (int)eExitCode::Error;
        }
    }
    else {
        return (int)eExitCode::Error;
    }

    return (int)eExitCode::Normal;
}