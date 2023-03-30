// AnalyzeData.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"
#include "AnalyzeData.h"
#include "../../../../LIB/CommonUtil/ReadINIParam.h"
#include "../../../../LIB/CommonUtil/StringEx.h"

#include "atlpath.h"
#include <iterator>
#include <iostream>
#include <vector>
#include <list>
#include <stdio.h>

using namespace std;
using std::endl;
using std::ofstream;

using namespace Gdiplus;
GdiplusStartupInput gdiSI;
ULONG_PTR           gdiToken;
CLSID     encoderClsid;

AnalyzeData::AnalyzeData(void)
{
}


AnalyzeData::~AnalyzeData(void)
{
}

// 画像サイズ取得
void get_jpeg_size(TCHAR* szFile, int* x, int* y) {
#ifndef UNICODE
    WCHAR wTitle[256];
    MultiByteToWideChar(932, 0, szFile, -1, wTitle, sizeof(wTitle) / sizeof(TCHAR));
    Image myImage(wTitle);
#else
    Image myImage(szFile);
#endif
    * x = myImage.GetWidth();
    *y = myImage.GetHeight();
}

// double変換チェック
bool ToDoubleValue(double& val, const std::string& str) {
    try {
        val = std::stod(str);
        return true;
    }
    catch (...) {
        return false;
    }
}

bool ToValue(int& val, const std::string& str) {
    try {
        val = std::stoi(str);
        return true;
    }
    catch (...) {
        return false;
    }
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

void __cdecl SetJPZone()
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

vector<MESHPOSITION_XY> GetMeshPositionXY(double maxLat, double minLat, double maxLon, double minLon) {

    // 平面直角変換用
    vector<MESHPOSITION_XY> totalList{};

    // 平面直角座標に変換
    // 小数点を最小は切り上げ、最大は切り捨てして座標を取得
    double leftDownEast, leftDownNorth, rightTopEast, rightTopNorth;
    double leftDownX, leftDownY, rightTopX, rightTopY;
    CGeoUtil::LonLatToXY(minLon, minLat, JPZONE, leftDownEast, leftDownNorth);
    CGeoUtil::LonLatToXY(maxLon, maxLat, JPZONE, rightTopEast, rightTopNorth);
    leftDownX = std::ceil(leftDownEast);
    leftDownY = std::ceil(leftDownNorth);
    rightTopX = std::floor(rightTopEast);
    rightTopY = std::floor(rightTopNorth);

    CPoint2D minPos, maxPos;
    minPos = CPoint2D(leftDownX, leftDownY);
    maxPos = CPoint2D(rightTopX, rightTopY);
    double width = abs(maxPos.x - minPos.x);
    double height = abs(maxPos.y - minPos.y);

    // 範囲内の座標を1m区切りで座標を取得
    for (int x = 0; x < width; x++)
    {
        for (int y = 0; y < height; y++)
        {
            MESHPOSITION_XY tmpPosXY{};

            // 4点の座標を保存
            tmpPosXY.leftTopX = minPos.x + x;
            tmpPosXY.leftTopY = minPos.y + y;
            tmpPosXY.leftDownX = minPos.x + x;
            tmpPosXY.leftDownY = minPos.y + y + 1;
            tmpPosXY.rightTopX = minPos.x + x + 1;
            tmpPosXY.rightTopY = minPos.y + y;
            tmpPosXY.rightDownX = minPos.x + x + 1;
            tmpPosXY.rightDownY = minPos.y + y + 1;

            // リスト追加
            totalList.push_back(tmpPosXY);

        }
    }

    // 取得した座標を返す
    return totalList;

}

// srcで指定するjpegファイルの矩形領域(左上sx,sy 幅 width 高さ height)でトリミングしdtcで指定するjpegファイルに保存
int imgcut(TCHAR* dtc, TCHAR* src, int sx, int sy, int width, int height) {
    Image* srcImage = 0;

#ifdef UNICODE
    srcImage = Bitmap::FromFile(src);
#else
    WCHAR srcFile[MAX_PATH];
    MultiByteToWideChar(932, 0, src, -1, srcFile, sizeof(srcFile) / sizeof(WCHAR));
    srcImage = Bitmap::FromFile(srcFile);
#endif

    if (srcImage == 0 || srcImage->GetLastStatus() != Gdiplus::Ok) {
        srcImage->~Image();
        return -1;
    }
    float px_res = srcImage->GetHorizontalResolution();  //      jpgファイルの解像度DPI(横)
    float py_res = srcImage->GetVerticalResolution();    //      jpgファイルの解像度DPI(縦)
    Bitmap* bmp = new Bitmap(width, height);                //　保存用
    bmp->SetResolution(px_res, py_res);

    Graphics* MyGraphics = Graphics::FromImage(bmp);

    MyGraphics->DrawImage(srcImage, 0, 0, sx, sy, width, height, UnitPixel);
    srcImage->~Image();

    EncoderParameters encoderParams;
    ULONG quality = 100;
    encoderParams.Count = 1;
    encoderParams.Parameter[0].Guid = EncoderQuality;
    encoderParams.Parameter[0].Type = EncoderParameterValueTypeLong;
    encoderParams.Parameter[0].NumberOfValues = 1;
    encoderParams.Parameter[0].Value = &quality;

#ifdef UNICODE
    bmp->Save(dtc, &encoderClsid, &encoderParams);
#else
    WCHAR dtcFile[MAX_PATH];
    MultiByteToWideChar(932, 0, dtc, -1, dtcFile, sizeof(dtcFile) / sizeof(WCHAR));
    bmp->Save(dtcFile, &encoderClsid, &encoderParams);
#endif
    delete bmp;
    delete MyGraphics;
    return 0;
}

// エンコーダーの取得
int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT  num = 0;
    UINT  size = 0;
    ImageCodecInfo* pImageCodecInfo;
    GetImageEncodersSize(&num, &size);
    if (size == 0)
        return -1;
    pImageCodecInfo = (ImageCodecInfo*)new char[size];
    if (pImageCodecInfo == NULL)
        return -1;
    GetImageEncoders(num, size, pImageCodecInfo);
    for (UINT n = 0; n < num; ++n) {
        if (wcscmp(pImageCodecInfo[n].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[n].Clsid;
            delete [] pImageCodecInfo;
            return n;
        }
    }
    delete [] pImageCodecInfo;
    return -1;
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
/// <summary>
/// 画像切り出し
/// </summary>
/// <param name="roofMinX"></param>
/// <param name="roofMinY"></param>
/// <param name="roofMaxX"></param>
/// <param name="roofMaxY"></param>
/// <param name="imagePath">元画像ファイルパス</param>
/// <param name="outFilePath">出力画像ファイルパス</param>
void SetRoofImage(double roofMinX, double roofMinY, double roofMaxX, double roofMaxY, string imagePath, string outFilePath)
{
    // 画像情報が無い場合は処理をしない
    if (jpgWidth != 0 && jpgHeight != 0 && tfw_x != 0.0 && tfw_y != 0.0) {
        // 初期値から最大最小の画像座標を取得
        int minX = (int)((abs(tfw_x) - abs(roofMinX)) / tfw_meshSize);
        int minY = (int)((tfw_y - roofMaxY) / tfw_meshSize);
        int maxX = (int)std::ceil((abs(tfw_x) - abs(roofMaxX)) / tfw_meshSize);
        int maxY = (int)std::ceil((tfw_y - roofMinY) / tfw_meshSize);

        // 開始位置、切り出す高さ、幅を取得
        trimWidth = maxX - minX;
        trimHeight = maxY - minY;

        // 切り出し処理
        std::wstring oWString1 = StringToWString(imagePath);
        std::wstring oWString2 = StringToWString(outFilePath);

        wchar_t* wc1 = oWString1.data();
        wchar_t* wc2 = oWString2.data();

        _tsetlocale(LC_ALL, _TEXT(""));
        GdiplusStartup(&gdiToken, &gdiSI, NULL);
        if (GetEncoderClsid(L"image/jpeg", &encoderClsid) < 0) {
            assert("jpegエンコーダーが取得できませんでした。");
            return;
        }

        if (imgcut(wc2, wc1, minX, minY, trimWidth, trimHeight)) {
            assert("jpegトリミングに失敗しました¥n");
        }

        GdiplusShutdown(gdiToken);

    }

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

BUILDINGSINFO GetBldgNode(wstring xmldata, string meshid)
{
    //xmlオブジェクト生成
    MSXML2::IXMLDOMDocument2Ptr reader;
    reader.CreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_INPROC_SERVER);

    //xpathをサポートするように設定
    reader->setProperty(CComBSTR(L"SelectionLanguage"), CComVariant(L"XPath"));
    //namespaceをサポートするように設定
    reader->setProperty(CComBSTR(L"SelectionNamespaces"), CComVariant(NAME_SPACE));

    //ロード
    VARIANT_BOOL isSuccessful;
    reader->load(CComVariant(xmldata.c_str()), &isSuccessful);

    BUILDINGSINFO buildingInfo;
    // 建物リスト
    vector<BUILDINGS> allBuildingList{};
    // 建物リストLOD1
    vector<BUILDINGSLOD1> allBuildingListLOD1{};

    // 最小緯度経度取得
    HRESULT hBoundResult;
    MSXML2::IXMLDOMNodePtr lowerCorner = 0;
    BSTR bound = SysAllocString(BOUND_XPATH1);
    reader->selectSingleNode(bound, &lowerCorner);

    if (NULL != lowerCorner) {
        // ノードタイプ取得
        MSXML2::DOMNodeType eMemberNodeType;
        hBoundResult = lowerCorner->get_nodeType(&eMemberNodeType);
        if (FAILED(hBoundResult))
        {
            assert(!"ノードタイプの取得に失敗");
        }
        else {
            // エレメント型への変換
            MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
            hBoundResult = lowerCorner->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
            if (FAILED(hBoundResult) || NULL == pXMLDOMMemberElement)
            {
                assert(!"エレメント型への変換に失敗");
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
                    // 最小値設定
                    if (minPosition.x == 0 || minPosition.x > wPosition.lon) {
                        minPosition.x = wPosition.lon;
                    }
                    if (minPosition.y == 0 || minPosition.y > wPosition.lat) {
                        minPosition.y = wPosition.lat;
                    }

                    // 平面直角座標に変換
                    double dEast, dNorth;
                    CGeoUtil::LonLatToXY(wPosition.lon, wPosition.lat, JPZONE, dEast, dNorth);
                    buildingInfo.bbMinX = (int)dEast;
                    buildingInfo.bbMinY = (int)dNorth;
                }
            }
        }
    }

    // 最大緯度経度取得
    MSXML2::IXMLDOMNodePtr upperCorner = 0;
    bound = SysAllocString(BOUND_XPATH2);
    reader->selectSingleNode(bound, &upperCorner);

    if (NULL != upperCorner) {
        // ノードタイプ取得
        MSXML2::DOMNodeType eMemberNodeType;
        hBoundResult = upperCorner->get_nodeType(&eMemberNodeType);
        if (FAILED(hBoundResult))
        {
            assert(!"ノードタイプの取得に失敗");
        }
        else {
            // エレメント型への変換
            MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
            hBoundResult = upperCorner->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
            if (FAILED(hBoundResult) || NULL == pXMLDOMMemberElement)
            {
                assert(!"エレメント型への変換に失敗");
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
                    // 最大値設定
                    if (maxPosition.x == 0 || maxPosition.x < wPosition.lon) {
                        maxPosition.x = wPosition.lon;
                    }
                    if (maxPosition.y == 0 || maxPosition.y < wPosition.lat) {
                        maxPosition.y = wPosition.lat;
                    }
                    // 平面直角座標に変換
                    double dEast, dNorth;
                    CGeoUtil::LonLatToXY(wPosition.lon, wPosition.lat, JPZONE, dEast, dNorth);
                    buildingInfo.bbMaxX = (int)std::ceil(dEast);
                    buildingInfo.bbMaxY = (int)std::ceil(dNorth);


                }
            }
        }

    }

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
        BUILDINGS buildings{};
        BUILDINGSLOD1 buildingsLOD1{};

        // ノードリストのうちの一つのノードの取得
        MSXML2::IXMLDOMNodePtr pXMLDOMNode = NULL;
        hResult = buildingList->get_item(i, &pXMLDOMNode);
        if (FAILED(hResult))
        {
            assert(!"ノードリストのうちの一つのノードの取得に失敗");
            continue;
        }

        //xpathのデータを取得
        MSXML2::IXMLDOMNodePtr node = 0;
        BSTR xp = SysAllocString(XPATH1);
        pXMLDOMNode->selectSingleNode(xp, &node);

        // LOD2場合
        if (NULL != node) {

            std::string buildId = "";
            if (!getBuildId(pXMLDOMNode, buildId))
            {
                assert(!"建物IDの取得に失敗");
                continue;
            }
            buildings.building = buildId;

            // 屋根情報取得(ノード)
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
                // 壁情報初期化
                WALLSURFACES wallSurfaces{};

                // ノードリストのうちの一つのノードの取得
                MSXML2::IXMLDOMNodePtr pXMLDOMRoofNode = NULL;
                hResult = roofSurfaceList->get_item(j, &pXMLDOMRoofNode);
                if (FAILED(hResult))
                {
                    assert(!"ノードリストのうちの一つのノードの取得に失敗");
                    continue;
                }

                // 壁情報タグ選択
                MSXML2::IXMLDOMNodePtr wallSurface = 0;
                BSTR wall = SysAllocString(XPATH10);
                pXMLDOMRoofNode->selectSingleNode(wall, &wallSurface);
                // 壁情報があれば処理を実行
                if (NULL != wallSurface) {
                    // ノードタイプ取得
                    MSXML2::DOMNodeType eWallNodeType;
                    hResult = wallSurface->get_nodeType(&eWallNodeType);
                    if (FAILED(hResult))
                    {
                        assert(!"ノードタイプの取得に失敗");
                        continue;
                    }

                    // エレメント型への変換
                    MSXML2::IXMLDOMElementPtr pXMLDOMWallElement = NULL;
                    hResult = wallSurface->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMWallElement);
                    if (FAILED(hResult) || NULL == pXMLDOMWallElement)
                    {
                        assert(!"エレメント型への変換に失敗");
                        continue;
                    }

                    // 壁IDを取得
                    MSXML2::IXMLDOMAttribute* pAttributeRoofNode = NULL;
                    CComVariant varValue;
                    BSTR id = SysAllocString(L"gml:id");
                    hResult = pXMLDOMWallElement->getAttribute(id, &varValue);
                    if (SUCCEEDED(hResult) && VT_BSTR == varValue.vt)
                    {
                        // BSTR⇒std::string変換
                        std::string wallSurfaceId = ConvertBSTRToMBS(varValue.bstrVal);
                        wallSurfaces.wallSurfaceId = wallSurfaceId;

                    }

                    // 壁詳細情報取得(ノード)
                    BSTR xp11 = SysAllocString(XPATH11);
                    MSXML2::IXMLDOMNodeListPtr surfaceMemberList = NULL;
                    pXMLDOMRoofNode->selectNodes(xp11, &surfaceMemberList);

                    // ノード件数取得
                    long lSurfaceMemberCountNode = 0;
                    hResult = surfaceMemberList->get_length(&lSurfaceMemberCountNode);

                    // 壁詳細数分繰り返す
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
                        MSXML2::IXMLDOMNodePtr surfaceMember = 0;
                        BSTR polygon = SysAllocString(XPATH7);
                        pXMLDOMSurfaceNode->selectSingleNode(polygon, &surfaceMember);

                        // ポリゴン情報があれば処理を実行
                        if (NULL != surfaceMember) {
                            // ノードタイプ取得
                            MSXML2::DOMNodeType eMemberNodeType;
                            hResult = surfaceMember->get_nodeType(&eMemberNodeType);
                            if (FAILED(hResult))
                            {
                                assert(!"ノードタイプの取得に失敗");
                                continue;
                            }

                            // エレメント型への変換
                            MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                            hResult = surfaceMember->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                            if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                            {
                                assert(!"エレメント型への変換に失敗");
                                continue;
                            }

                            // ポリゴンIDを取得
                            MSXML2::IXMLDOMAttribute* pAttributeMemberNode = NULL;
                            CComVariant varValue;
                            BSTR id = SysAllocString(L"gml:id");
                            hResult = pXMLDOMMemberElement->getAttribute(id, &varValue);
                            if (SUCCEEDED(hResult) && VT_BSTR == varValue.vt)
                            {
                                // BSTR⇒std::string変換
                                std::string polygonStr = ConvertBSTRToMBS(varValue.bstrVal);
                                wSurfaceMembers.polygon = polygonStr;
                            }

                            // ラインタグ選択
                            MSXML2::IXMLDOMNodePtr LinearRing = 0;
                            BSTR linear = SysAllocString(XPATH8);
                            pXMLDOMSurfaceNode->selectSingleNode(linear, &LinearRing);

                            if (NULL != LinearRing) {
                                // ノードタイプ取得
                                MSXML2::DOMNodeType eMemberNodeType;
                                hResult = LinearRing->get_nodeType(&eMemberNodeType);
                                if (FAILED(hResult))
                                {
                                    assert(!"ノードタイプの取得に失敗");
                                    continue;
                                }

                                // エレメント型への変換
                                MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                                hResult = LinearRing->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                                if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                                {
                                    assert(!"エレメント型への変換に失敗");
                                    continue;
                                }
                                // ラインIDを取得
                                MSXML2::IXMLDOMAttribute* pAttributeMemberNode = NULL;
                                CComVariant varValue;
                                BSTR id = SysAllocString(L"gml:id");
                                hResult = pXMLDOMMemberElement->getAttribute(id, &varValue);
                                if (SUCCEEDED(hResult) && VT_BSTR == varValue.vt)
                                {
                                    // BSTR⇒std::string変換
                                    std::string linerRingStr = ConvertBSTRToMBS(varValue.bstrVal);
                                    wSurfaceMembers.linearRing = linerRingStr;
                                }

                                // 座標タグ選択
                                MSXML2::IXMLDOMNodePtr Position = 0;
                                BSTR pos = SysAllocString(XPATH9);
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
                                        POSITION wPosition{};

                                        // 平面直角変換用
                                        double dEast, dNorth;

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

                                                // 平面直角座標に変換
                                                CGeoUtil::LonLatToXY(wPosition.lon, wPosition.lat, JPZONE, dEast, dNorth);
                                                CPointBase pt(dEast, dNorth, wPosition.ht);

                                                // 屋根詳細リスト内座標リストに追加                                                
                                                wSurfaceMembers.posList.push_back(pt);

                                                // 一時座標リスト初期化
                                                POSITION wPosition{};
                                                posCnt = 0;
                                            }

                                        }
                                    }


                                }
                                else {
                                    continue;
                                }


                            }
                            else {
                                continue;
                            }


                        }
                        else {
                            continue;
                        }

                        // 壁詳細リストに追加
                        wallSurfaces.wallSurfaceList.push_back(wSurfaceMembers);
                    }

                    // 壁リストに追加
                    buildings.wallSurfaceList.push_back(wallSurfaces);
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


                    double maxLat = 0;
                    double minLat = 0;
                    double maxLon = 0;
                    double minLon = 0;

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

                        // ポリゴンタグ選択
                        MSXML2::IXMLDOMNodePtr surfaceMember = 0;
                        BSTR polygon = SysAllocString(XPATH7);
                        pXMLDOMSurfaceNode->selectSingleNode(polygon, &surfaceMember);

                        // ポリゴン情報があれば処理を実行
                        if (NULL != surfaceMember) {
                            // ノードタイプ取得
                            MSXML2::DOMNodeType eMemberNodeType;
                            hResult = surfaceMember->get_nodeType(&eMemberNodeType);
                            if (FAILED(hResult))
                            {
                                assert(!"ノードタイプの取得に失敗");
                                continue;
                            }

                            // エレメント型への変換
                            MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                            hResult = surfaceMember->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                            if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                            {
                                assert(!"エレメント型への変換に失敗");
                                continue;
                            }

                            // ポリゴンIDを取得
                            MSXML2::IXMLDOMAttribute* pAttributeMemberNode = NULL;
                            CComVariant varValue;
                            BSTR id = SysAllocString(L"gml:id");
                            hResult = pXMLDOMMemberElement->getAttribute(id, &varValue);
                            if (SUCCEEDED(hResult) && VT_BSTR == varValue.vt)
                            {
                                // BSTR⇒std::string変換
                                std::string polygonStr = ConvertBSTRToMBS(varValue.bstrVal);
                                wSurfaceMembers.polygon = polygonStr;
                            }

                            // ラインタグ選択
                            MSXML2::IXMLDOMNodePtr LinearRing = 0;
                            BSTR linear = SysAllocString(XPATH8);
                            pXMLDOMSurfaceNode->selectSingleNode(linear, &LinearRing);

                            if (NULL != LinearRing) {
                                // ノードタイプ取得
                                MSXML2::DOMNodeType eMemberNodeType;
                                hResult = LinearRing->get_nodeType(&eMemberNodeType);
                                if (FAILED(hResult))
                                {
                                    assert(!"ノードタイプの取得に失敗");
                                    continue;
                                }

                                // エレメント型への変換
                                MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                                hResult = LinearRing->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                                if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                                {
                                    assert(!"エレメント型への変換に失敗");
                                    continue;
                                }
                                // ラインIDを取得
                                MSXML2::IXMLDOMAttribute* pAttributeMemberNode = NULL;
                                CComVariant varValue;
                                BSTR id = SysAllocString(L"gml:id");
                                hResult = pXMLDOMMemberElement->getAttribute(id, &varValue);
                                if (SUCCEEDED(hResult) && VT_BSTR == varValue.vt)
                                {
                                    // BSTR⇒std::string変換
                                    std::string linerRingStr = ConvertBSTRToMBS(varValue.bstrVal);
                                    wSurfaceMembers.linearRing = linerRingStr;
                                }

                                // 座標タグ選択
                                MSXML2::IXMLDOMNodePtr Position = 0;
                                BSTR pos = SysAllocString(XPATH9);
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
                                        POSITION wPosition{};

                                        // 平面直角変換用
                                        double dEast, dNorth;

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



                                                // 座標最大、最小を取得
                                                if (maxLat == 0) {
                                                    maxLat = wPosition.lat;
                                                }
                                                else if (maxLat < wPosition.lat) {
                                                    maxLat = wPosition.lat;
                                                }
                                                if (minLat == 0) {
                                                    minLat = wPosition.lat;
                                                }
                                                else if (minLat > wPosition.lat) {
                                                    minLat = wPosition.lat;
                                                }
                                                if (maxLon == 0) {
                                                    maxLon = wPosition.lon;
                                                }
                                                else if (maxLon < wPosition.lon) {
                                                    maxLon = wPosition.lon;
                                                }
                                                if (minLon == 0) {
                                                    minLon = wPosition.lon;
                                                }
                                                else if (minLon > wPosition.lon) {
                                                    minLon = wPosition.lon;
                                                }

                                                // 平面直角座標に変換
                                                CGeoUtil::LonLatToXY(wPosition.lon, wPosition.lat, JPZONE, dEast, dNorth);
                                                CPointBase pt(dEast, dNorth, wPosition.ht);

                                                // 屋根詳細リスト内座標リストに追加                                                
                                                wSurfaceMembers.posList.push_back(pt);

                                                // 一時座標リスト初期化
                                                POSITION wPosition{};
                                                posCnt = 0;
                                            }

                                        }

                                    }

                                }
                                else {
                                    continue;
                                }

                            }
                            else {
                                continue;
                            }

                        }
                        else {
                            continue;
                        }

                        // 屋根詳細リストに追加
                        roofSurfaces.roofSurfaceList.push_back(wSurfaceMembers);

                    }

                    // 1mジオメトリリスト初期化
                    vector<MESHPOSITION_XY> tempMeshPosList{};

                    // メッシュリスト取得
                    vector<MESHPOSITION_XY> resultList = GetMeshPositionXY(maxLat, minLat, maxLon, minLon);

                    for (auto& p : resultList) {

                        // 内外判定用
                        CPoint2D pt1_2D(p.leftTopX, p.leftTopY);
                        CPoint2D pt2_2D(p.rightTopX, p.rightTopY);
                        CPoint2D pt3_2D(p.rightDownX, p.rightDownY);
                        CPoint2D pt4_2D(p.leftDownX, p.leftDownY);

                        bool check1 = false, check2 = false, check3 = false, check4 = false;

                        for (auto& member : roofSurfaces.roofSurfaceList)
                        {
                            // 内外判定用
                            vector<CPoint2D> point2DList{};
                            
                            for (auto& pos : member.posList)
                            {
                                CPoint2D p2D = CPoint2D(pos.x, pos.y);
                                point2DList.push_back(p2D);
                            }

                            CPoint2D* pt2D = &point2DList[0];

                            if (!check1) check1 = CGeoUtil::IsPointInPolygon(pt1_2D, (int)point2DList.size(), pt2D);
                            if (!check2) check2 = CGeoUtil::IsPointInPolygon(pt2_2D, (int)point2DList.size(), pt2D);
                            if (!check3) check3 = CGeoUtil::IsPointInPolygon(pt3_2D, (int)point2DList.size(), pt2D);
                            if (!check4) check4 = CGeoUtil::IsPointInPolygon(pt4_2D, (int)point2DList.size(), pt2D);

                        }

                        // 屋根面判定(1mメッシュが含まれるか判定)
                        if (check1 && check2 && check3 && check4) {

                            // 屋根面座標変換
                            tempMeshPosList.push_back(p);
                        }

                    }

                    // メッシュリスト設定
                    roofSurfaces.meshPosList = tempMeshPosList;

                    // 屋根リストに追加
                    buildings.roofSurfaceList.push_back(roofSurfaces);

                }
                else {
                    continue;
                }
            }

            // 建物リスト追加
            allBuildingList.push_back(buildings);
        }
        else {

            // 建物ID
            std::string buildId = "";
            if (!getBuildId(pXMLDOMNode, buildId))
            {
                assert(!"建物IDの取得に失敗");
                continue;
            }
            buildingsLOD1.building = buildId;

            // 個体情報取得(ノード)
            BSTR xp12 = SysAllocString(XPATH12);
            MSXML2::IXMLDOMNodeListPtr compositeSurfaceList = NULL;
            pXMLDOMNode->selectNodes(xp12, &compositeSurfaceList);

            // ノード件数取得
            long lCompositeSurfaceCountNode = 0;
            hResult = compositeSurfaceList->get_length(&lCompositeSurfaceCountNode);

            // 個体数分繰り返す
            for (int j = 0; j < lCompositeSurfaceCountNode; j++) {
                // 壁情報初期化
                WALLSURFACES wallSurfaces{};

                // ノードリストのうちの一つのノードの取得
                MSXML2::IXMLDOMNodePtr pXMLDOMRoofNode = NULL;
                hResult = compositeSurfaceList->get_item(j, &pXMLDOMRoofNode);
                if (FAILED(hResult))
                {
                    assert(!"ノードリストのうちの一つのノードの取得に失敗");
                    continue;
                }

                // 壁詳細情報取得(ノード)
                BSTR xp13 = SysAllocString(XPATH13);
                MSXML2::IXMLDOMNodeListPtr surfaceMemberList = NULL;
                pXMLDOMRoofNode->selectNodes(xp13, &surfaceMemberList);

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
                    MSXML2::IXMLDOMNodePtr surfaceMember = 0;
                    BSTR polygon = SysAllocString(XPATH7);
                    pXMLDOMSurfaceNode->selectSingleNode(polygon, &surfaceMember);

                    // ポリゴン情報があれば処理を実行
                    if (NULL != surfaceMember) {
                        // ノードタイプ取得
                        MSXML2::DOMNodeType eMemberNodeType;
                        hResult = surfaceMember->get_nodeType(&eMemberNodeType);
                        if (FAILED(hResult))
                        {
                            assert(!"ノードタイプの取得に失敗");
                            continue;
                        }

                        // エレメント型への変換
                        MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                        hResult = surfaceMember->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                        if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                        {
                            assert(!"エレメント型への変換に失敗");
                            continue;
                        }


                        // ラインタグ選択
                        MSXML2::IXMLDOMNodePtr LinearRing = 0;
                        BSTR linear = SysAllocString(XPATH8);
                        pXMLDOMSurfaceNode->selectSingleNode(linear, &LinearRing);

                        if (NULL != LinearRing) {
                            // ノードタイプ取得
                            MSXML2::DOMNodeType eMemberNodeType;
                            hResult = LinearRing->get_nodeType(&eMemberNodeType);
                            if (FAILED(hResult))
                            {
                                assert(!"ノードタイプの取得に失敗");
                                continue;
                            }

                            // エレメント型への変換
                            MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                            hResult = LinearRing->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                            if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                            {
                                assert(!"エレメント型への変換に失敗");
                                continue;
                            }

                            // 座標タグ選択
                            MSXML2::IXMLDOMNodePtr Position = 0;
                            BSTR pos = SysAllocString(XPATH9);
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
                                    POSITION wPosition{};

                                    // 平面直角変換用
                                    double dEast, dNorth;

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

                                            // 平面直角座標に変換
                                            CGeoUtil::LonLatToXY(wPosition.lon, wPosition.lat, JPZONE, dEast, dNorth);
                                            CPointBase pt(dEast, dNorth, wPosition.ht);

                                            // 屋根詳細リスト内座標リストに追加                                                
                                            wSurfaceMembers.posList.push_back(pt);

                                            // 一時座標リスト初期化
                                            POSITION wPosition{};
                                            posCnt = 0;
                                        }

                                    }
                                }


                            }
                            else {
                                continue;
                            }


                        }
                        else {
                            continue;
                        }


                    }
                    else {
                        continue;
                    }

                    // 壁詳細リストに追加
                    wallSurfaces.wallSurfaceList.push_back(wSurfaceMembers);
                }
                // 壁リストに追加
                buildingsLOD1.wallSurfaceList.push_back(wallSurfaces);

            }

        // 建物リスト追加
        allBuildingListLOD1.push_back(buildingsLOD1);
        }


    }

    buildingInfo.buildingList = allBuildingList;
    buildingInfo.buildingListLOD1 = allBuildingListLOD1;


    //xmlオブジェクト解放
    reader.Release();
    //COMの解放
    CoUninitialize();
    
    return buildingInfo;
}

vector<DEMLIST> GetDemNode(wstring xmldata)
{
    HRESULT hResult;

    //COMの初期化
    hResult = CoInitialize(0);
    //xmlオブジェクト生成
    MSXML2::IXMLDOMDocument2Ptr reader;
    reader.CreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_INPROC_SERVER);

    //xpathをサポートするように設定
    reader->setProperty(CComBSTR(L"SelectionLanguage"), CComVariant(L"XPath"));
    //namespaceをサポートするように設定
    reader->setProperty(CComBSTR(L"SelectionNamespaces"), CComVariant(DEM_NAME_SPACE));

    //ロード
    VARIANT_BOOL isSuccessful;
    reader->load(CComVariant(xmldata.c_str()), &isSuccessful);

    vector<DEMLIST> alldemList;

    BSTR xp2 = SysAllocString(XPATH2);
    MSXML2::IXMLDOMNodeListPtr demMemberList = NULL;
    reader->selectNodes(xp2, &demMemberList);

    // ノード件数取得
    long lCountNode = 0;
    hResult = demMemberList->get_length(&lCountNode);

    // demデータ数分繰り返す
    for (int i = 0; i < lCountNode; i++) {

        DEMLIST demList;
        // DEM座標リスト
        vector<CTriangle> allDemTriangleList{};

        // ノードリストのうちの一つのノードの取得
        MSXML2::IXMLDOMNodePtr pXMLDOMNode = NULL;
        hResult = demMemberList->get_item(i, &pXMLDOMNode);
        if (FAILED(hResult))
        {
            assert(!"ノードリストのうちの一つのノードの取得に失敗");
            continue;
        }

        // メッシュIDを取得
        std::string strid = "";
        MSXML2::IXMLDOMNodePtr pMeshNode = 0;
        BSTR dem_xp3 = SysAllocString(DEM_XPATH3);
        pXMLDOMNode->selectSingleNode(dem_xp3, &pMeshNode);
        if (NULL != pMeshNode)
        {
            // ノードタイプ取得
            MSXML2::DOMNodeType eMemberNodeType;
            hResult = pMeshNode->get_nodeType(&eMemberNodeType);
            if (FAILED(hResult))
            {
                assert(!"ノードタイプの取得に失敗");
                continue;
            }

            // エレメント型への変換
            MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
            hResult = pMeshNode->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
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
                strid = ConvertBSTRToMBS(valueText);
            }
        }
        else
        {
            // ファイル名からメッシュIDを取得
            int path_i = (int)xmldata.find_last_of(L"\\") + 1;
            int ext_i = (int)xmldata.find_last_of(L".");
            std::wstring filename = xmldata.substr(path_i, (int64_t)ext_i - path_i);
            std::wstring meshId = filename.substr(0, 8);

            strid = CStringEx::ToString(meshId);

        }
        if (strid == "")
        {
            assert(!"メッシュIDの取得に失敗");
            continue;
        }
        demList.meshID = strid;

        BSTR dem_xp = SysAllocString(DEM_XPATH1);
        MSXML2::IXMLDOMNodeListPtr triangleList = NULL;
        pXMLDOMNode->selectNodes(dem_xp, &triangleList);

        // ノード件数取得
        long lCountTriangle = 0;
        hResult = triangleList->get_length(&lCountTriangle);
        
        for (int j = 0; j < lCountTriangle; j++) {

            // ノードリストのうちの一つのノードの取得
            MSXML2::IXMLDOMNodePtr pTriangle = NULL;
            hResult = triangleList->get_item(j, &pTriangle);
            if (FAILED(hResult))
            {
                assert(!"ノードリストのうちの一つのノードの取得に失敗");
                continue;
            }

            // 座標情報タグ選択
            MSXML2::IXMLDOMNodePtr Position = 0;
            BSTR pos = SysAllocString(DEM_XPATH2);
            pTriangle->selectSingleNode(pos, &Position);

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
                    DEMPOSITION wPosition{};
                    // 平面直角変換
                    double dEast, dNorth;

                    // 三角形座標一時格納用
                    vector<CPointBase> vecPos;

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

                            // 平面直角座標に変換
                            CGeoUtil::LonLatToXY(wPosition.lon, wPosition.lat, JPZONE, dEast, dNorth);
                            CPointBase pt(dEast, dNorth, wPosition.ht);

                            // 屋根詳細リスト内座標リストに追加                                                
                            vecPos.push_back(pt);
                            // 一時座標リスト初期化
                            POSITION wPosition{};
                            posCnt = 0;
                        }

                        // 最終座標は取得しない
                        if (x == posAry.size() - 3) {
                            break;
                        }

                    }

                    if (vecPos.size() == 3)
                    {
                        CTriangle triangle(vecPos[0], vecPos[1], vecPos[2]);
                        allDemTriangleList.push_back(triangle);
                    }
                }

            }
            else {
                continue;
            }

        }
        if (allDemTriangleList.empty()) {
            continue;
        }

        demList.posTriangleList = allDemTriangleList;
        alldemList.push_back(demList);
    }

    //xmlオブジェクト解放
    reader.Release();
    //COMの解放
    CoUninitialize();

    return alldemList;
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

int __cdecl AnalizeBldgFiles(const char* str, const char* strOut)
{

    // パスチェック
    if (strlen(str) == 0) {
        std::cout << "str is null." << std::endl;
        return 1;
    }
    if (strlen(strOut) == 0) {
        std::cout << "strOut is null." << std::endl;
        return 1;
    }

    std::vector<std::string> fileName;
    
    // 全体リスト
    allList.clear();

    // キャンセルファイルパス
    std::filesystem::path path = std::filesystem::path(strOut).parent_path() / CANCELFILE;
    std::string cancelPath = path.string();

    // ファイル名取得
    if(getFileNames(str, ".gml", fileName) == true) {

        // ファイル数繰り返す
        for (auto& p : fileName) {
            BUILDINGSINFO result;
            BLDGLIST wbldgList{};

            // ファイル名からメッシュIDを取得
            std::string fullpath = p.c_str();
            int path_i = (int)fullpath.find_last_of("\\") + 1;
            int ext_i = (int)fullpath.find_last_of(".");
            std::string filename = fullpath.substr(path_i, (int64_t)ext_i - path_i);
            std::string meshId = filename.substr(0, 8);

            // メッシュIDセット
            wbldgList.meshID= meshId;

            //  Readerの作成
            std::wstring oWString = StringToWString(p.c_str());

            // XMLデータから固定XPathのデータを取得
            result = GetBldgNode(oWString, meshId);

            // データがない場合は次のファイルへ移動
            if (result.buildingList.empty() && result.buildingListLOD1.empty()) {
                continue;
            }
            // データ設定
            wbldgList.buildingList = result.buildingList;
            wbldgList.buildingListLOD1 = result.buildingListLOD1;
            wbldgList.bbMaxX = result.bbMaxX;
            wbldgList.bbMaxY = result.bbMaxY;
            wbldgList.bbMinX = result.bbMinX;
            wbldgList.bbMinY = result.bbMinY;

            // データ追加
            allList.push_back(wbldgList);

            // キャンセルファイルチェック
            if (std::filesystem::exists(cancelPath)) {
                return 2;
            }

        }

        // ファイルがあれば最大緯度経度出力
        if (0 < fileName.size()) {
            // 出力フォルダに出力
            std::filesystem::path p = std::filesystem::path(strOut) / OUTPUTFILE;
            ofstream ofs(p);
            ofs << std::fixed;
            ofs << std::setprecision(13);
            ofs << "top:" << maxPosition.y << endl;
            ofs << "bottom:" << minPosition.y << endl;
            ofs << "left:" << minPosition.x << endl;
            ofs << "right:" << maxPosition.x << endl;
        }
    }
    else {
        return 1;
    }
    return 0;
}
int __cdecl AnalizeDemFiles(const char* str, const char* strOut)
{

    if (strlen(str) == 0) {
        std::cout << "str is null." << std::endl;
        return 1;
    }
    if (strlen(strOut) == 0) {
        std::cout << "strOut is null." << std::endl;
        return 1;
    }

    std::vector<std::string> fileName;
    
    // キャンセルファイルパス
    std::filesystem::path path = std::filesystem::path(strOut) / CANCELFILE;
    std::string cancelPath = path.string();

    // ファイル名取得
    if (getFileNames(str, ".gml", fileName) == true) {

        // ファイル数繰り返す
        for (auto& p : fileName) {
            // Readerの作成
            std::wstring oWString = StringToWString(p.c_str());
            AnalyzeData xml;

            // XMLデータから固定XPathのデータを取得
            vector<DEMLIST> result = GetDemNode(oWString);

            // データがない場合は次のファイルへ移動
            if (result.empty()) {
                continue;
            }

            // データ追加
            allDemList.insert(allDemList.end(), result.begin(), result.end());

            // キャンセルファイルチェック
            if (std::filesystem::exists(cancelPath)) {
                return 2;
            }

        }

    }
    else {
        return 1;
    }
    return 0;

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
/// <summary>
/// UV座標に変換した座標文字列を取得する
/// </summary>
/// <param name="posList">平面直角座標配列</param>
/// <returns></returns>
std::string GetUVPositon(double roofMinX, double roofMinY, std::vector<CPointBase> posList) {

    // UV座標変換
    std::string strUV;

    // 画像が無い場合は処理をしない
    if (jpgWidth != 0 && jpgHeight != 0 && tfw_x != 0 && tfw_y != 0) {
        for (int i = 0; i < posList.size(); i++) {

            // 座標値を取得
            double x = (abs(roofMinX) - abs(posList[i].x)) / tfw_meshSize;
            double y = (posList[i].y - roofMinY) / tfw_meshSize;
            double UVx = x / trimWidth;
            double UVy = y / trimHeight;

            // 変換した座標を文字列に追加
            strUV += std::to_string(UVx) + ' ';
            strUV += std::to_string(UVy) + ' ';
        }
        // 最後の文字削除
        strUV.pop_back();
    }

    return strUV;
}

/// <summary>
/// 建物ノードに出力値を追加
/// </summary>
/// <param name="reader">対象xml</param>
/// <param name="buildingID">建物ID</param>
/// <param name="outputItem">出力値</param>
/// <param name="buildingList">対象建物リスト</param>
/// <param name="folderName">画像出力パス</param>
/// <param name="imagePath">画像元パス</param>
/// <param name="folderPath">画像出力パス</param>
void SetBldgNode(MSXML2::IXMLDOMDocument2Ptr reader , string buildingID , LOD2OUTLIST outputItem, std::vector<BUILDINGS> buildingList, string folderName, string imagePath, string folderPath)
{
    MSXML2::IXMLDOMElementPtr  pElement0;
    MSXML2::IXMLDOMElementPtr  pElement1;
    MSXML2::IXMLDOMElementPtr  pElement2;
    MSXML2::IXMLDOMTextPtr     pTextItem;
    MSXML2::IXMLDOMTextPtr     pTextItemEnd;


    MSXML2::IXMLDOMElementPtr  pTexElement1;
    MSXML2::IXMLDOMElementPtr  pTexElement2;
    MSXML2::IXMLDOMElementPtr  pTexElement3;
    MSXML2::IXMLDOMElementPtr  pTexElement4;
    MSXML2::IXMLDOMElementPtr  pTexElement5;
    MSXML2::IXMLDOMElementPtr  pTexElement6;
    MSXML2::IXMLDOMElementPtr  pTexElement7;
    MSXML2::IXMLDOMTextPtr     pTexTextItem1;
    MSXML2::IXMLDOMTextPtr     pTexTextItem2;
    MSXML2::IXMLDOMTextPtr     pTexTextItem3;
    MSXML2::IXMLDOMTextPtr     pTexTextItemEnd;

    HRESULT hResult;

    // 建物IDを検索
    MSXML2::IXMLDOMNodePtr building = NULL;
    eCityGMLVersion version;
    for (version = eCityGMLVersion::VERSION_1; version != eCityGMLVersion::End; ++version)
    {        
        const wchar_t* result = L"";

        switch (version)
        {
        case eCityGMLVersion::VERSION_1:
        {
            const std::string bldgPath = "core:CityModel/core:cityObjectMember/bldg:Building/gen:stringAttribute/gen:value";
            string buildingNode = bldgPath + "[contains(text(),'" + buildingID + "')]";
            wstring wide_string = wstring(buildingNode.begin(), buildingNode.end());
            result = wide_string.c_str();
            break;
        }

        case eCityGMLVersion::VERSION_2:
        {
            const std::string bldgPath = "core:CityModel/core:cityObjectMember/bldg:Building/uro:buildingIDAttribute/uro:BuildingIDAttribute/uro:buildingID";
            string buildingNode = bldgPath + "[contains(text(),'" + buildingID + "')]";
            wstring wide_string = wstring(buildingNode.begin(), buildingNode.end());
            result = wide_string.c_str();
            break;
        }

        default:
            break;
        }

        // 建物ID指定のノードを選択
        BSTR xp2 = SysAllocString(result);
        reader->selectSingleNode(xp2, &building);
        if (building != NULL)   break;

    }

    if (version == eCityGMLVersion::End)
    {   // 取得失敗
        return;
    }

    // 建物IDで検索
    std::size_t bilIndex;
    std::vector<ROOFSURFACES> wRoof{};  // 格納用
    auto bilfi = std::find_if(buildingList.begin(), buildingList.end(),
        [&](const auto& row) {
            return((row.building == buildingID));
        });
    // 存在する場合、屋根リストをセット
    if (bilfi != buildingList.end()) {
        // 存在する場合は登録済みの配列に値をセット
        bilIndex = std::distance(std::begin(buildingList), bilfi);
        wRoof = buildingList[bilIndex].roofSurfaceList;
    }

    // 出力画像ファイル名前
    std::string outFileName = folderName + "/" + buildingID + ".jpg";

    // 要素タグ 
    const char* TagType = "";
    const char* TagName1 = "gen:measureAttribute";

    // 要素名
    const char* element1 = "name";
    const char* elementValue1_1 = "年間予測日射量";
    const char* elementValue1_2 = "年間予測発電量";
    const char* elementValue1_3 = "光害発生時間（夏至）";
    const char* elementValue1_4 = "光害発生時間（春分）";
    const char* elementValue1_5 = "光害発生時間（冬至）";

    // 値タグ
    const char* TagName2 = "gen:value";
    const char* element2 = "uom";
    const char* elementValue2_1 = "kWh/(m2・年)";
    const char* elementValue2_2 = "kWh/年";
    const char* elementValue2_3 = "h";

    // テクスチャ要素タグ
    const char* texTagType = "";
    const char* texTagName1 = "app:appearanceMember";
    const char* texTagName2 = "app:Appearance";
    const char* texTagName3 = "app:theme";
    const char* texTagName4 = "app:surfaceDataMember";
    const char* texTagName5 = "app:ParameterizedTexture";
    const char* texTagName6 = "app:imageURI";
    const char* texTagName7 = "app:mimeType";
    const char* texTagName8 = "app:target";
    const char* texTagName9 = "app:TexCoordList";
    const char* texTagName10 = "app:textureCoordinates";

    // テクスチャ要素名
    const char* texElement1 = "uri";
    const char* texElement2 = "ring";

    const char* textValue1 = "rgbTexture";
    const char* textValue2 = "image/jpg";



    // 建物IDが無ければ終了
    MSXML2::IXMLDOMNodePtr pPearentNode = NULL;
    MSXML2::IXMLDOMNode* pXMLNextNode = NULL;
    MSXML2::IXMLDOMNode* pXMLNodeCur;
    CComVariant varRef = NULL;

    if (NULL != building) {
        // 親の親に移動　
        hResult = building->get_parentNode(&pPearentNode);
        if (FAILED(hResult))
        {
            assert(!"親ノードタイプの取得に失敗");
            return;
        }
        hResult = pPearentNode->get_parentNode(&building);
        if (FAILED(hResult))
        {
            assert(!"親ノードタイプの取得に失敗");
            return;
        }
        if (version == eCityGMLVersion::VERSION_2)
        {
            MSXML2::IXMLDOMNodePtr pPearentNode2 = building;
            hResult = pPearentNode2->get_parentNode(&building);
            if (FAILED(hResult))
            {
                assert(!"親ノードタイプの取得に失敗");
                return;
            }
        }
        // 建物IDの次のノード取得
        hResult = pPearentNode->get_nextSibling(&pXMLNextNode);
        if (FAILED(hResult))
        {
            assert(!"ノードリストのうちの一つのノードの取得に失敗");
            return;
        }
    }
    else {
        return;
    }

    // ■年間予測日射量
    if (!outputItem.solarInsolation.empty()){

        // 追加ノードを作成
        reader->createElement(_bstr_t(TagName1), &pElement1);                                   // タグ　<gen:measureAttribute>
        reader->createElement(_bstr_t(TagName2), &pElement2);                                   // タグ　<gen:measureAttribute>

        // 属性、要素
        pElement1->setAttribute(_bstr_t(element1), _variant_t(elementValue1_1));                // 要素名　年間予測日射量
        pElement2->setAttribute(_bstr_t(element2), _variant_t(elementValue2_1));                // 値タグ　kWh/(m2・年)

        // 値
        reader->createTextNode(_bstr_t(outputItem.solarInsolation.c_str()), &pTextItem);        // 値

        // 作成したノードを追加
        pElement2->appendChild(pTextItem, NULL);
        pElement1->appendChild(pElement2, NULL);

        // 建物IDノードに追加
        varRef = pXMLNextNode;
        hResult = building->insertBefore(pElement1, varRef, &pXMLNodeCur);
        varRef.Clear();

    }

    // ■年間予測発電量
    if (!outputItem.solarPowerGeneration.empty()) {

        reader->createElement(_bstr_t(TagName1), &pElement1);                                   // タグ　<gen:measureAttribute>
        reader->createElement(_bstr_t(TagName2), &pElement2);                                   // タグ　<gen:measureAttribute>

        // 属性、要素
        pElement1->setAttribute(_bstr_t(element1), _variant_t(elementValue1_2));                // 要素名　年間予測発電量
        pElement2->setAttribute(_bstr_t(element2), _variant_t(elementValue2_2));                // 値タグ　kWh/年

        // 値
        reader->createTextNode(_bstr_t(outputItem.solarPowerGeneration.c_str()), &pTextItem);    // 値

        // 作成したノードを追加
        pElement2->appendChild(pTextItem, NULL);
        pElement1->appendChild(pElement2, NULL);

        // 建物IDノードに追加
        varRef = pXMLNextNode;
        hResult = building->insertBefore(pElement1, varRef, &pXMLNodeCur);
        varRef.Clear();

    }
    // ■公害発生時間（夏至）
    if (!outputItem.lightPollutionTimeSummer.empty()) {

        reader->createElement(_bstr_t(TagName1), &pElement1);
        reader->createElement(_bstr_t(TagName2), &pElement2);

        // 属性、要素
        pElement1->setAttribute(_bstr_t(element1), _variant_t(elementValue1_3));                    // 要素名　公害発生時間（夏至）
        pElement2->setAttribute(_bstr_t(element2), _variant_t(elementValue2_3));                    // 値タグ　h

        // 値
        reader->createTextNode(_bstr_t(outputItem.lightPollutionTimeSummer.c_str()), &pTextItem);   // 値


        // 作成したノードを追加
        pElement2->appendChild(pTextItem, NULL);
        pElement1->appendChild(pElement2, NULL);

        // 建物IDノードに追加
        varRef = pXMLNextNode;
        hResult = building->insertBefore(pElement1, varRef, &pXMLNodeCur);
        varRef.Clear();

    }
    // ■公害発生時間（春至）
    if (!outputItem.lightPollutionTimeSpring.empty()) {

        reader->createElement(_bstr_t(TagName1), &pElement1);
        reader->createElement(_bstr_t(TagName2), &pElement2);

        // 属性、要素
        pElement1->setAttribute(_bstr_t(element1), _variant_t(elementValue1_4));        // 要素名　公害発生時間（春至）
        pElement2->setAttribute(_bstr_t(element2), _variant_t(elementValue2_3));        // 値タグ　h

        // 値
        reader->createTextNode(_bstr_t(outputItem.lightPollutionTimeSpring.c_str()), &pTextItem);    // 値


        // 作成したノードを追加
        pElement2->appendChild(pTextItem, NULL);
        pElement1->appendChild(pElement2, NULL);

        // 建物IDノードに追加
        varRef = pXMLNextNode;
        hResult = building->insertBefore(pElement1, varRef, &pXMLNodeCur);
        varRef.Clear();

    }

    // ■公害発生時間（冬至）
    if (!outputItem.lightPollutionTimeWinter.empty()) {

        reader->createElement(_bstr_t(TagName1), &pElement1);
        reader->createElement(_bstr_t(TagName2), &pElement2);

        // 属性、要素
        pElement1->setAttribute(_bstr_t(element1), _variant_t(elementValue1_5));                    // 要素名　公害発生時間（冬至）
        pElement2->setAttribute(_bstr_t(element2), _variant_t(elementValue2_3));                    // 値タグ　h

        // 値
        reader->createTextNode(_bstr_t(outputItem.lightPollutionTimeWinter.c_str()), &pTextItem);    // 値


        // 作成したノードを追加
        pElement2->appendChild(pTextItem, NULL);
        pElement1->appendChild(pElement2, NULL);

        // 建物IDノードに追加
        varRef = pXMLNextNode;
        hResult = building->insertBefore(pElement1, varRef, &pXMLNodeCur);
        varRef.Clear();

    }

    // テクスチャノード追加
    if (!outputItem.solarInsolation.empty()) {
        BSTR tex_xp = SysAllocString(TEX_XPATH1);
        MSXML2::IXMLDOMNodePtr cityModel = NULL;
        reader->selectSingleNode(tex_xp, &cityModel);

        // 対象屋根面がある場合は設定
        if (wRoof.size() > 0) {

            BSTR tex_xp2 = SysAllocString(TEX_XPATH2);
            MSXML2::IXMLDOMNodePtr appear = NULL;
            reader->selectSingleNode(tex_xp2, &appear);

            // 追加ノードを作成
            reader->createElement(_bstr_t(texTagName4), &pTexElement1);                                   // タグ　<app:surfaceDataMember>
            reader->createElement(_bstr_t(texTagName5), &pTexElement2);                                   // タグ　<app:ParameterizedTexture>
            reader->createElement(_bstr_t(texTagName6), &pTexElement3);                                   // タグ　<app:imageURI>
            reader->createElement(_bstr_t(texTagName7), &pTexElement4);                                   // タグ　<app:mimeType>


            // 値
            reader->createTextNode(_bstr_t(outFileName.c_str()), &pTexTextItem1);        // 値
            reader->createTextNode(_bstr_t(textValue2), &pTexTextItem2);        // 値

            // 作成したノードを追加
            pTexElement3->appendChild(pTexTextItem1, NULL);
            pTexElement4->appendChild(pTexTextItem2, NULL);
            pTexElement1->appendChild(pTexElement2, NULL);
            pTexElement2->appendChild(pTexElement3, NULL);
            pTexElement2->appendChild(pTexElement4, NULL);

            // 屋根面ごとの最大最小
            double roofMinX = 0.0, roofMinY = 0.0;
            double roofMaxX = 0.0, roofMaxY = 0.0;

            for (int i = 0; i < wRoof.size(); i++) {

                for (int j = 0; j < wRoof[i].roofSurfaceList.size(); j++) {
                    // 選択値
                    SURFACEMEMBERS wSurface = wRoof[i].roofSurfaceList[j];

                    for (int k = 0; k < wSurface.posList.size(); k++) {
                        // 最小値設定
                        if (roofMinX == 0 || roofMinX > wSurface.posList[k].x) {
                            roofMinX = wSurface.posList[k].x;
                        }
                        if (roofMinY == 0 || roofMinY > wSurface.posList[k].y) {
                            roofMinY = wSurface.posList[k].y;
                        }
                        // 最大値設定
                        if (roofMaxX == 0 || roofMaxX < wSurface.posList[k].x) {
                            roofMaxX = wSurface.posList[k].x;
                        }
                        if (roofMaxY == 0 || roofMaxY < wSurface.posList[k].y) {
                            roofMaxY = wSurface.posList[k].y;
                        }
                    }
                }
            }

            // 画像切り出し
            std::string outFilePath = folderPath + "/" + buildingID + ".jpg";
            SetRoofImage(roofMinX, roofMinY, roofMaxX, roofMaxY, imagePath, outFilePath);

            for (int i = 0; i < wRoof.size(); i++) {

                for (int j = 0; j < wRoof[i].roofSurfaceList.size(); j++) {
                    // 選択値
                    SURFACEMEMBERS wSurface = wRoof[i].roofSurfaceList[j];

                    // 追加ノードを作成
                    reader->createElement(_bstr_t(texTagName8), &pTexElement5);                                   // タグ　<app:target>
                    reader->createElement(_bstr_t(texTagName9), &pTexElement6);                                   // タグ　<app:TexCoordList>
                    reader->createElement(_bstr_t(texTagName10), &pTexElement7);                                  // タグ　<app:textureCoordinates>

                    // 属性、要素
                    string strPolygon = "#" + wSurface.polygon;
                    string strLinearRing = "#" + wSurface.linearRing;
                    pTexElement5->setAttribute(_bstr_t(texElement1), _variant_t(strPolygon.c_str()));              // 要素名　uri
                    pTexElement7->setAttribute(_bstr_t(texElement2), _variant_t(strLinearRing.c_str()));           // 要素名　ring


                    // UV座標変換
                    std::string position = GetUVPositon(roofMinX, roofMinY, wSurface.posList);

                    // 値
                    reader->createTextNode(_bstr_t(position.c_str()), &pTexTextItem2);    // 値

                    // 作成したノードを追加
                    pTexElement7->appendChild(pTexTextItem2, NULL);
                    pTexElement2->appendChild(pTexElement5, NULL);
                    pTexElement5->appendChild(pTexElement6, NULL);
                    pTexElement6->appendChild(pTexElement7, NULL);

                }
            }



            appear->appendChild(pTexElement1, NULL);
        }
    }
}


/// <summary>
/// テクスチャノード(先頭ノード)を追加
/// </summary>
/// <param name="reader">対象xml</param>
void SetTexNode(MSXML2::IXMLDOMDocument2Ptr reader, string buildingID, std::vector<BUILDINGS> buildingList)
{

    // 建物IDで検索
    std::size_t bilIndex;
    std::vector<ROOFSURFACES> wRoof{};  // 格納用
    auto bilfi = std::find_if(buildingList.begin(), buildingList.end(),
        [&](const auto& row) {
            return((row.building == buildingID));
        });
    // 存在する場合、屋根リストをセット
    if (bilfi != buildingList.end()) {
        // 存在する場合は登録済みの配列に値をセット
        bilIndex = std::distance(std::begin(buildingList), bilfi);
        wRoof = buildingList[bilIndex].roofSurfaceList;
    }

    MSXML2::IXMLDOMElementPtr  pTexElement1;
    MSXML2::IXMLDOMElementPtr  pTexElement2;
    MSXML2::IXMLDOMElementPtr  pTexElement3;
    MSXML2::IXMLDOMTextPtr     pTexTextItem1;
    MSXML2::IXMLDOMTextPtr     pTexTextItemEnd;

    // テクスチャ要素タグ
    const char* texTagType = "";
    const char* texTagName1 = "app:appearanceMember";
    const char* texTagName2 = "app:Appearance";
    const char* texTagName3 = "app:theme";

    const char* textValue1 = "rgbTexture";

    // 建物IDが無ければ終了
    MSXML2::IXMLDOMNodePtr pPearentNode = NULL;
    MSXML2::IXMLDOMNode* pXMLNextNode = NULL;
    CComVariant varRef = NULL;

    //テクスチャノード存在チェック
    MSXML2::IXMLDOMNodePtr node = 0;
    BSTR app = SysAllocString(TEX_XPATH2);
    reader->selectSingleNode(app, &node);
    

    // 対象屋根面がある場合は設定
    if (wRoof.size() > 0 && NULL == node) {
            // テクスチャノード追加
            BSTR tex_xp = SysAllocString(TEX_XPATH1);
            MSXML2::IXMLDOMNodePtr cityModel = NULL;
            reader->selectSingleNode(tex_xp, &cityModel);

            // 追加ノードを作成
            reader->createElement(_bstr_t(texTagName1), &pTexElement1);                                   // タグ　<app:appearanceMember>
            reader->createElement(_bstr_t(texTagName2), &pTexElement2);                                   // タグ　<app:Appearance>
            reader->createElement(_bstr_t(texTagName3), &pTexElement3);                                   // タグ　<app:theme>

            // 値
            reader->createTextNode(_bstr_t(textValue1), &pTexTextItem1);

            // 作成したノードを追加
            pTexElement3->appendChild(pTexTextItem1, NULL);
            pTexElement1->appendChild(pTexElement2, NULL);
            pTexElement2->appendChild(pTexElement3, NULL);

            // 建物IDノードに追加
            cityModel->appendChild(pTexElement1, NULL);
    }

}

/// <summary>
/// フォーマットして出力
/// </summary>
/// <param name="pDoc"></param>
/// <param name="omstrFilePath"></param>
/// <returns></returns>
bool FormatDOMDocument(MSXML2::IXMLDOMDocument2Ptr pDoc, char* omstrFilePath)
{
    MSXML2::ISAXXMLReaderPtr pSaxXmlReader = NULL;
    pSaxXmlReader.CreateInstance(__uuidof(MSXML2::SAXXMLReader60));

    CComPtr<IStream> pStream;
    DWORD grfMode = STGM_WRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE;

    if (SHCreateStreamOnFile((LPCTSTR)omstrFilePath, grfMode, &pStream) == S_OK)
    {
        MSXML2::IMXWriterPtr pImxWriter;
        pImxWriter.CreateInstance(__uuidof(MSXML2::MXXMLWriter60));
        pImxWriter->put_output(CComVariant(pStream));
        pSaxXmlReader->putContentHandler((MSXML2::ISAXContentHandlerPtr)pImxWriter);
        pSaxXmlReader->putErrorHandler((MSXML2::ISAXErrorHandlerPtr)pImxWriter);
        pSaxXmlReader->putDTDHandler((MSXML2::ISAXDTDHandlerPtr)pImxWriter);
        pImxWriter->put_encoding(_bstr_t("UTF-8"));
        pImxWriter->put_indent(VARIANT_TRUE);

        pSaxXmlReader->parse((_variant_t)(pDoc.GetInterfacePtr()));
        pImxWriter->flush();
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

int __cdecl LOD2DataOut(const char* str, const char* strOut)
{

    if (strlen(str) == 0) {
        std::cout << "str is null." << std::endl;
        return 1;
    }
    if (strlen(strOut) == 0) {
        std::cout << "strOut is null." << std::endl;
        return 1;
    }

    // CSVファイル読み込み
    std::filesystem::path path1 = std::filesystem::path(strOut) / "data" /INPUTFILE1;       // 予測光害発生時間
    std::filesystem::path path2 = std::filesystem::path(strOut) / "data" /INPUTFILE2;       // 年間予測日射量・年間予測発電量
    std::string csvFile1 = path1.string();
    std::string csvFile2 = path2.string();
    // キャンセルファイルパス
    std::filesystem::path path3 = std::filesystem::path(strOut) / CANCELFILE;
    std::string cancelPath = path3.string();

    // 年間日射量、年間発電量のCSV内容を配列に取得
    vector<vector<string>> yearPredictionList = csv2vector(csvFile2, 1);
    // 公害発生時間（春分、夏至、冬至）のCSV内容を配列に取得
    vector<vector<string>> lightpollutionList = csv2vector(csvFile1, 1);


    // 引数で受け取ったGMLファイルのディレクトリ
    std::string dir(str, 0, strlen(str));

    std::vector<std::string> fileName;
    std::string  test;

    // 全体リスト
    vector<LOD2OUTLIST> outputList{};

    // 処理用配列作成：年間予測日射量,年間予測発電量CSVデータ格納
    for (int i = 0; i < yearPredictionList.size(); i++) {
        LOD2OUTLIST wOutputItem{};  // 格納用
        // 処理用配列に追加
        wOutputItem.meshID = yearPredictionList[i][static_cast<int>(yearPrediction::meshID)];
        wOutputItem.building = yearPredictionList[i][static_cast<int>(yearPrediction::building)];
        wOutputItem.solarInsolation = yearPredictionList[i][static_cast<int>(yearPrediction::solarInsolation)];
        wOutputItem.solarPowerGeneration = yearPredictionList[i][static_cast<int>(yearPrediction::solarPowerGeneration)];
        // データ追加
        outputList.push_back(wOutputItem);
    }
    // 処理用配列作成：光害発生時間総数CSVデータ格納
    std::size_t index;
    for (int i = 0; i < lightpollutionList.size(); i++) {
        LOD2OUTLIST wOutputItem{};  // 格納用
        // メッシュID、建物IDで検索
        auto fi = std::find_if(outputList.begin(), outputList.end(),
            [&](const auto& row) {
                return((row.meshID == lightpollutionList[i][static_cast<int>(lightPollution::meshID)])
                    && (row.building == lightpollutionList[i][static_cast<int>(lightPollution::building)]));
            });

        if (fi != outputList.end()) {

            // 存在する場合は登録済みの配列に値をセット
            index = std::distance(std::begin(outputList), fi);
            // 既に追加済みの配列に追記する
            outputList[index].lightPollutionTimeSummer = lightpollutionList[i][static_cast<int>(lightPollution::summer)];
            outputList[index].lightPollutionTimeSpring = lightpollutionList[i][static_cast<int>(lightPollution::spling)];
            outputList[index].lightPollutionTimeWinter = lightpollutionList[i][static_cast<int>(lightPollution::winter)];
        }
        else {
            // 見つからない場合は配列に追加
            wOutputItem.meshID = lightpollutionList[i][static_cast<int>(lightPollution::meshID)];
            wOutputItem.building = lightpollutionList[i][static_cast<int>(lightPollution::building)];
            wOutputItem.lightPollutionTimeSummer = lightpollutionList[i][static_cast<int>(lightPollution::summer)];
            wOutputItem.lightPollutionTimeSpring = lightpollutionList[i][static_cast<int>(lightPollution::spling)];
            wOutputItem.lightPollutionTimeWinter = lightpollutionList[i][static_cast<int>(lightPollution::winter)];
            // データ追加
            outputList.push_back(wOutputItem);
        }
    }

    // キャンセルファイルチェック
    if (std::filesystem::exists(cancelPath)) {
        return 2;
    }

    // 処理対象が存在する場合は後続処理を実施
    if (outputList.size() == 0) {
        // 対象がない場合は後続処理を実施しない
        return 1;
    }
    
    // メッシュIDでソート
    sort(outputList.begin(), outputList.end());

    // 入力フォルダのgmlファイルを出力フォルダにコピーする
    std::filesystem::path copydir = std::filesystem::path(strOut) / "output" / "bldg";         // コピー先ディレクトリ

    try {
        // 全てのファイル
        // ファイル名取得
        if (getFileNames(str, ".gml", fileName) == true) {

            // ファイル数繰り返す
            for (auto& p : fileName) {

                // ファイル名を取得
                std::string fullpath = p.c_str();
                int path_i = (int)fullpath.find_last_of("\\") + 1;
                int ext_i = (int)fullpath.find_last_of(".");
                std::string filename = fullpath.substr(path_i, (int64_t)ext_i + path_i);

                // CSVファイル読み込み
                std::filesystem::path file = std::filesystem::path(str) / filename;             // コピー元
                std::filesystem::path copyfile = std::filesystem::path(copydir) / filename;    // コピー先

                //コピー
                CopyFile((LPCWSTR)file.c_str(), (LPCWSTR)copyfile.c_str(), TRUE);//FALSEだとファイルの上書き

            }
        }
        else {
            return 1;
        }
    }
    catch (std::out_of_range& e)
    {
        std::cout << "範囲外へのアクセスです" << std::endl;
        std::cout << e.what() << std::endl;
    }


    // キャンセルファイルチェック
    if (std::filesystem::exists(cancelPath)) {
        return 2;
    }

    // GMLファイル書き込み処理
    HRESULT hResult;
    hResult = CoInitialize(NULL); // 初期化

    std::string meshId;
    //xmlオブジェクト生成
    MSXML2::IXMLDOMDocument2Ptr reader;

    // 対象GMLファイル
    // ファイル名を取得
    std::string fullpath = fileName[0];
    int path_i = (int)fullpath.find_last_of("\\") + 1;
    int ext_i = (int)fullpath.find_last_of(".");
    std::string gmlfilename = fullpath.substr(path_i, (int64_t)ext_i + path_i);
    // ファイル名のメッシュID部分を置換
    gmlfilename.replace(0, 8, outputList[0].meshID);
    std::string gmlfoldername = outputList[0].meshID + "_bldg_6697_appearance";
    std::filesystem::path path = std::filesystem::path(copydir) / gmlfilename;             // コピー元
    std::filesystem::path fdpath = std::filesystem::path(copydir) / gmlfoldername;
    std::filesystem::path imgpath = std::filesystem::path(strOut) / "data" / outputList[0].meshID;
    std::string imgfilename = "日射量" + outputList[0].meshID + ".tif";
    std::string tfwfilename = "日射量" + outputList[0].meshID + ".tfw";
    std::filesystem::path imgfilepath = std::filesystem::path(imgpath) / imgfilename;
    std::filesystem::path tfwfilepath = std::filesystem::path(imgpath) / tfwfilename;
    std::string filepath = path.string();
    std::string folderpath = fdpath.string();
    std::string imagepath = imgfilepath.string();
    std::string tfwpath = tfwfilepath.string();

    std::wstring oWString = StringToWString(path.string());
    reader.CreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_INPROC_SERVER);
    //xpathをサポートするように設定
    reader->setProperty(CComBSTR(L"SelectionLanguage"), CComVariant(L"XPath"));
    //namespaceをサポートするように設定
    reader->setProperty(CComBSTR(L"SelectionNamespaces"), CComVariant(NAME_SPACE));
    reader->setProperty(CComBSTR(L"XmlWriterProperty_Indent"), CComVariant(TRUE));
    // 初回ファイル読み込み
    VARIANT_BOOL isSuccessful;
    reader->load(CComVariant(oWString.c_str()), &isSuccessful);

    // 初回メッシュIDで検索
    std::size_t allIndex;
    std::vector<BUILDINGS> wBuildingList{};  // 格納用
    auto allfi = std::find_if(allList.begin(), allList.end(),
        [&](const auto& row) {
            return((row.meshID == outputList[0].meshID));
        });
    // 存在する場合、建物リストをセット
    if (allfi != allList.end()) {
        // 存在する場合は登録済みの配列に値をセット
        allIndex = std::distance(std::begin(allList), allfi);
        wBuildingList = allList[allIndex].buildingList;

    }

    // jpgファイルサイズ取得
    std::wstring wImgString = StringToWString(imagepath);
    wchar_t* img = wImgString.data();
    get_jpeg_size(img, &jpgWidth, &jpgHeight);

    // ワールドファイル読込み
    vector<vector<string>> tfwList = csv2vector(tfwpath, 0);

    // サイズチェック
    if (tfwList.size() == 7) {
        // 変換チェック
        if (!ToDoubleValue(tfw_x, tfwList[4][0])) {
            tfw_x = 0.0;
        }
        if (!ToDoubleValue(tfw_y, tfwList[5][0])) {
            tfw_y = 0.0;
        }
        if (!ToDoubleValue(tfw_meshSize, tfwList[0][0])) {
            tfw_meshSize = 0.0;
        }
    }
    else {
        tfw_x = 0.0;
        tfw_y = 0.0;
        tfw_meshSize = 0.0;
    }

    // ピクセル中心座標なので左下に補正する
    tfw_x = tfw_x - tfw_meshSize * 0.5;
    tfw_y = tfw_y - tfw_meshSize * 0.5;

    // 既存のテクスチャノードを削除
    MSXML2::IXMLDOMNodePtr texNode = NULL;
    BSTR tex_xp3 = SysAllocString(TEX_XPATH3);
    reader->selectSingleNode(tex_xp3, &texNode);
    MSXML2::IXMLDOMNodePtr pPearentNode = 0;
    if (texNode != NULL) {
        hResult = texNode->get_parentNode(&pPearentNode);
        if (FAILED(hResult))
        {
            assert(!"親ノードタイプの取得に失敗");
            return 1;
        }
    }
    MSXML2::IXMLDOMNodePtr removeNode = 0;
    pPearentNode->removeChild(texNode, &removeNode);

    // テクスチャノード追加
    if (wBuildingList.size() > 0) {
        // 建物IDで検索
        auto bilfi = std::find_if(wBuildingList.begin(), wBuildingList.end(),
            [&](const auto& row) {
                return((row.building == outputList[0].building));
            });
        // 存在する場合、テクスチャノード追加
        if (bilfi != wBuildingList.end()) {
            SetTexNode(reader, outputList[0].building, wBuildingList);
            reader->save(_variant_t(oWString.c_str()));
            reader->load(CComVariant(oWString.c_str()), &isSuccessful);

            // フォルダが無ければ作成
            bool isExists = std::filesystem::exists(folderpath);
            if (!isExists) {
                std::filesystem::create_directory(folderpath);
            }
        }
    }


    // 作成した処理用配列でループ
    for (int i = 0; i < outputList.size(); ++i)
    {
        // ファイル切り替え時に書き込み（初回は書き込みしない）
        if (outputList[i].meshID != meshId && meshId != ""){

            //書き込み
            reader->save(_variant_t(oWString.c_str()));
            // フォーマット
            FormatDOMDocument(reader, (char*)oWString.c_str());

            //xmlオブジェクト解放
            reader.Release();

            // メッシュIDが切り替わり=ファイルの切り替わりタイミング
            gmlfilename.replace(0, 8, outputList[i].meshID);
            gmlfoldername = outputList[i].meshID + "_bldg_6697_appearance";
            imgpath = std::filesystem::path(strOut) / "data" / outputList[i].meshID;
            imgfilename = "日射量" + outputList[i].meshID + ".tif";
            tfwfilename = "日射量" + outputList[i].meshID + ".tfw";
            imgfilepath = std::filesystem::path(imgpath) / imgfilename;
            tfwfilepath = std::filesystem::path(imgpath) / tfwfilename;

            path = std::filesystem::path(copydir) / gmlfilename;
            fdpath = std::filesystem::path(copydir) / gmlfoldername;
            oWString = StringToWString(path.string());
            folderpath = fdpath.string();
            imagepath = imgfilepath.string();
            tfwpath = tfwfilepath.string();

            reader.CreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_INPROC_SERVER);
            //xpathをサポートするように設定
            reader->setProperty(CComBSTR(L"SelectionLanguage"), CComVariant(L"XPath"));
            //namespaceをサポートするように設定
            reader->setProperty(CComBSTR(L"SelectionNamespaces"), CComVariant(NAME_SPACE));
            reader->setProperty(CComBSTR(L"XmlWriterProperty_Indent"), CComVariant(TRUE));

            // 対象gmlファイルの読み込み
            reader->load(CComVariant(oWString.c_str()), &isSuccessful);

            wBuildingList.clear();  // 格納用
            auto allfi = std::find_if(allList.begin(), allList.end(),
                [&](const auto& row) {
                    return((row.meshID == outputList[i].meshID));
                });
            // 存在する場合、建物リストをセット
            if (allfi != allList.end()) {
                // 存在する場合は登録済みの配列に値をセット
                allIndex = std::distance(std::begin(allList), allfi);
                wBuildingList = allList[allIndex].buildingList;
            }

            // jpgファイルサイズ取得
            wImgString = StringToWString(imagepath);
            img = wImgString.data();
            get_jpeg_size(img, &jpgWidth, &jpgHeight);
            // ワールドファイル読込み
            vector<vector<string>> tfwList = csv2vector(tfwpath, 0);

            // サイズチェック
            if (tfwList.size() == 7) {
                // 変換チェック
                if (!ToDoubleValue(tfw_x, tfwList[4][0])) {
                    tfw_x = 0.0;
                }
                if (!ToDoubleValue(tfw_y, tfwList[5][0])) {
                    tfw_x = 0.0;
                }
            }
            else {
                tfw_x = 0.0;
                tfw_x = 0.0;
            }

            // 既存のテクスチャノードを削除
            texNode = 0;
            BSTR tex_xp3 = SysAllocString(TEX_XPATH3);
            reader->selectSingleNode(tex_xp3, &texNode);
            pPearentNode = 0;
            if (texNode != NULL) {
                hResult = texNode->get_parentNode(&pPearentNode);
                if (FAILED(hResult))
                {
                    assert(!"親ノードタイプの取得に失敗");
                    continue;
                }
            }
            removeNode = 0;
            pPearentNode->removeChild(texNode, &removeNode);

            // テクスチャノード追加
            if (wBuildingList.size() > 0) {
                // 建物IDで検索
                auto bilfi = std::find_if(wBuildingList.begin(), wBuildingList.end(),
                    [&](const auto& row) {
                        return((row.building == outputList[i].building));
                    });
                // 存在する場合、テクスチャノード追加
                if (bilfi != wBuildingList.end()) {
                    SetTexNode(reader, outputList[i].building, wBuildingList);
                    reader->save(_variant_t(oWString.c_str()));
                    reader->load(CComVariant(oWString.c_str()), &isSuccessful);

                    // フォルダが無ければ作成
                    bool isExists = std::filesystem::exists(folderpath);
                    if (!isExists) {
                        std::filesystem::create_directory(folderpath);
                    }
                }

            }
        }
        // 対象のGMLファイル、建物IDに必要ば情報を追記する
        SetBldgNode(reader, outputList[i].building, outputList[i], wBuildingList, gmlfoldername, imagepath, folderpath);

        // メッシュIDの更新
        meshId = outputList[i].meshID;

        // キャンセルファイルチェック
        if (std::filesystem::exists(cancelPath)) {
            return 2;
        }
    }

     //最終ファイル分書き込み
     //書き込み
    reader->save(_variant_t(oWString.c_str()));
    // フォーマット
    FormatDOMDocument(reader, (char*)oWString.c_str());
   
    //xmlオブジェクト解放
    reader.Release();

    //COMの解放
    CoUninitialize();

    return 0;

}
