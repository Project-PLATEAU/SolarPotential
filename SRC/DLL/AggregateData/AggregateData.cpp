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

// split�֐�
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
/// �W�v�ΏۂƂȂ��͔͈̓f�[�^��ǉ�
/// </summary>
/// <param name="p"></param>
/// <returns></returns>
void __cdecl AddAnalyzeAreaData(AnalyzeAreaData* p)
{
    // ��͔͈͂�ݒ�
    std::vector<CPoint2D> areaPoints;
    // �\���_���擾
    int arrlength = p->nPointCount * 2;
    for (int j = 0; j < arrlength; j += 2)
    {
        // �ܓx�o�x�����ʒ��p���W�n�ɕϊ�
        double dX, dY;
        CGeoUtil::LonLatToXY(p->pPointArray[j + 1], p->pPointArray[j], JPZONE, dX, dY);
        areaPoints.emplace_back(CPoint2D(dX, dY));
    }

    AREADATA areaData{};
    areaData.areaID = p->strAreaId;
    areaData.areaName = p->strAreaName;
    areaData.pos2dList = areaPoints;

    // �G���A�͈͂�ʑ��p�`�ɕ�������
    vector<CPoint2DPolygon> aryPolygons;
    CPoint2DPolygon areaPolygon;
    for (const auto& p : areaData.pos2dList)
    {
        CPoint2D pt2d = CPoint2D(p.x, p.y);
        areaPolygon.Add(pt2d);
    }
    // �������n�_�ɂ���
    areaPolygon.StartLeft();

    // �ʑ��p�`
    if (areaPolygon.IsConvexPolygon())
    {
        aryPolygons.emplace_back(areaPolygon);
    }
    // �����p�`
    else
    {
        // �ʑ��p�`�ɕ�������
        areaPolygon.GetConvexPolygons(aryPolygons);
    }
    areaData.polygons = aryPolygons;

    allAreaList.emplace_back(areaData);
}

// BSTR��std::string�ϊ�����
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
// BSTR��std::string�ϊ�
std::string ConvertBSTRToMBS(BSTR bstr)
{
    int wslen = ::SysStringLen(bstr);
    return ConvertWCSToMBS((wchar_t*)bstr, wslen);
}
/*
    string��wstring�֕ϊ�����
*/
std::wstring StringToWString
(
    std::string oString
)
{
    // SJIS �� wstring
    int iBufferSize = MultiByteToWideChar(CP_ACP, 0, oString.c_str()
        , -1, (wchar_t*)NULL, 0);

    // �o�b�t�@�̎擾
    wchar_t* cpUCS2 = new wchar_t[iBufferSize];

    // SJIS �� wstring
    MultiByteToWideChar(CP_ACP, 0, oString.c_str(), -1, cpUCS2
        , iBufferSize);

    // string�̐���
    std::wstring oRet(cpUCS2, cpUCS2 + iBufferSize - 1);

    // �o�b�t�@�̔j��
    delete[] cpUCS2;

    // �ϊ����ʂ�Ԃ�
    return(oRet);
}
// std::string��wchar_t�ϊ�����
wchar_t* ConvertStringTowchar(std::string str)
{
    const size_t newsizew = str.size() + 1;
    size_t convertedChars = 0;
    wchar_t* wcstring = new wchar_t[newsizew];
    mbstowcs_s(&convertedChars, wcstring, newsizew, str.c_str(), _TRUNCATE);

    return wcstring;
}
/**
* @brief �t�H���_�ȉ��̃t�@�C���ꗗ���擾����֐�
* @param[in]    folderPath  �t�H���_�p�X
* @param[out]   file_names  �t�@�C�����ꗗ
* return        true:����, false:���s
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

    /* �G���[���� */
    if (err) {
        std::cout << err.value() << std::endl;
        std::cout << err.message() << std::endl;
        return false;
    }
    return true;
}
// CSV�ǂݍ���
vector<vector<string> >csv2vector(string filename, int ignore_line_num = 0) {
    //csv�t�@�C���̓ǂݍ���
    ifstream reading_file;
    reading_file.open(filename, ios::in);
    if (!reading_file) {
        vector<vector<string> > data;
        return data;
    }
    string reading_line_buffer;
    //�ŏ���ignore_line_num�s����ǂ݂���
    for (int line = 0; line < ignore_line_num; line++) {
        getline(reading_file, reading_line_buffer);
        if (reading_file.eof()) break;
    }

    //�񎟌���vector���쐬
    vector<vector<string> > data;
    while (getline(reading_file, reading_line_buffer)) {
        if (reading_line_buffer.size() == 0) break;
        vector<string> temp_data;
        temp_data = split(reading_line_buffer, ",");
        data.emplace_back(temp_data);
    }
    return data;
}

// ����������(YYYYMMDDhhmmss)�擾
string getDatetimeStr() {
    time_t t = time(nullptr);
    struct tm nowTime;
    errno_t error;
    error = localtime_s(&nowTime, &t);

    std::stringstream s;
    s << nowTime.tm_year + 1900;
    // setw(),setfill()��0�l��
    s << setw(2) << setfill('0') << nowTime.tm_mon + 1;
    s << setw(2) << setfill('0') << nowTime.tm_mday;
    s << setw(2) << setfill('0') << nowTime.tm_hour;
    s << setw(2) << setfill('0') << nowTime.tm_min;
    s << setw(2) << setfill('0') << nowTime.tm_sec;
    // std::string�ɂ��Ēl��Ԃ�
    return s.str();
}

// ����ID���擾����
bool getBuildId(const MSXML2::IXMLDOMNodePtr& pXMLDOMNode, std::string& strid)
{
    bool bret = false;
    HRESULT hResult;

    // ����ID�̃m�[�h���擾
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
    {   // �擾���s
        return false;
    }

    // �l���擾
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
        // �m�[�h�^�C�v�擾
        MSXML2::DOMNodeType eMemberNodeType;
        hResult = buildingValue->get_nodeType(&eMemberNodeType);
        if (FAILED(hResult))
        {
            assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
            return false;
        }

        // �G�������g�^�ւ̕ϊ�
        MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
        hResult = buildingValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
        if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
        {
            assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
            return false;
        }

        // �l�e�L�X�g�擾
        BSTR valueText;
        hResult = pXMLDOMMemberElement->get_text(&valueText);
        if (SUCCEEDED(hResult))
        {
            // BSTR��std::string�ϊ�
            strid = ConvertBSTRToMBS(valueText);
            bret = true;
        }
    }

    return bret;
}

/// <summary>
/// ���O��Ԃ�ݒ�
/// </summary>
/// <param name="xmldata"></param>
/// <returns></returns>
std::wstring GetSelectionNamespaces()
{
    std::wstring namespaces;

    // ���O��Ԃ̐ݒ�t�@�C����ǂݍ���
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
                strLine.pop_back(); // ���s���폜
                namespaces += strLine + L" ";
            }
        }
        fio.Close();
    }
    namespaces.pop_back();

    return namespaces;

}

// �^���Z�����X�N�̐Z���[���擾����
bool getRiverFloodingRisk(const MSXML2::IXMLDOMNodePtr& pXMLDOMNode, double& depth)
{
    bool bret = false;
    HRESULT hResult;

    // �^���Z���z��̃m�[�h���擾
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

        // �m�[�h����
        hResult = genericAttributeSetList->get_length(&lGenericAttributeCountNode);
        if ( 0 != lGenericAttributeCountNode) {
            break;
        }
    }

    if (version == eCityGMLVersion::End)
    {   // �擾���s
        return false;
    }

    // �^���Z�������J��Ԃ�
    for (int j = 0; j < lGenericAttributeCountNode; j++) {

        // �m�[�h���X�g�̂����̈�̃m�[�h�̎擾
        MSXML2::IXMLDOMNodePtr pXMLDOMGenericNode = NULL;
        hResult = genericAttributeSetList->get_item(j, &pXMLDOMGenericNode);
        if (FAILED(hResult))
        {
            assert(!"�m�[�h���X�g�̂����̈�̃m�[�h�̎擾�Ɏ��s");
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
                // �l���擾
                MSXML2::IXMLDOMNodePtr floodDepthValue = 0;
                BSTR val = SysAllocString(XPATH_genericAttributeSet3);
                floodDepth->selectSingleNode(val, &floodDepthValue);
                if (NULL != floodDepthValue)
                {
                    // �m�[�h�^�C�v�擾
                    MSXML2::DOMNodeType eMemberNodeType;
                    hResult = floodDepthValue->get_nodeType(&eMemberNodeType);
                    if (FAILED(hResult))
                    {
                        assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                        continue;
                    }

                    // �G�������g�^�ւ̕ϊ�
                    MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                    hResult = floodDepthValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                    if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                    {
                        assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                        continue;
                    }

                    // �l�e�L�X�g�擾
                    BSTR valueText;
                    hResult = pXMLDOMMemberElement->get_text(&valueText);
                    if (SUCCEEDED(hResult))
                    {
                        // BSTR��std::string�ϊ�
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
            // �l���擾
            MSXML2::IXMLDOMNodePtr floodDepthValue = 0;
            BSTR val = SysAllocString(XPATH_genericAttributeSet2_2);
            pXMLDOMGenericNode->selectSingleNode(val, &floodDepthValue);
            if (NULL != floodDepthValue)
            {
                // �m�[�h�^�C�v�擾
                MSXML2::DOMNodeType eMemberNodeType;
                hResult = floodDepthValue->get_nodeType(&eMemberNodeType);
                if (FAILED(hResult))
                {
                    assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                    continue;
                }

                // �G�������g�^�ւ̕ϊ�
                MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                hResult = floodDepthValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                {
                    assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                    continue;
                }

                // �l�e�L�X�g�擾
                BSTR valueText;
                hResult = pXMLDOMMemberElement->get_text(&valueText);
                if (SUCCEEDED(hResult))
                {
                    // BSTR��std::string�ϊ�
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

// �Ôg�Z�����X�N�̐Z���[���擾����
bool getTsunamiRisk(const MSXML2::IXMLDOMNodePtr& pXMLDOMNode, double& height)
{
    bool bret = false;
    HRESULT hResult;

    // �Ôg�Z���z��̃m�[�h���擾
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
    {   // �擾���s
        return false;
    }

    switch (version)
    {
    case eCityGMLVersion::VERSION_1:
    {
        // �Z���[
        MSXML2::IXMLDOMNodePtr tsunamiHeight = 0;
        BSTR flood = SysAllocString(XPATH_genericAttributeSet2);
        tsunamiHeightNode->selectSingleNode(flood, &tsunamiHeight);
        if (NULL != tsunamiHeight)
        {
            // �l���擾
            MSXML2::IXMLDOMNodePtr tsunamiHeightValue = 0;
            BSTR val = SysAllocString(XPATH_genericAttributeSet3);
            tsunamiHeight->selectSingleNode(val, &tsunamiHeightValue);
            if (NULL != tsunamiHeightValue)
            {
                // �m�[�h�^�C�v�擾
                MSXML2::DOMNodeType eMemberNodeType;
                hResult = tsunamiHeight->get_nodeType(&eMemberNodeType);
                if (FAILED(hResult))
                {
                    assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                    return false;
                }

                // �G�������g�^�ւ̕ϊ�
                MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                hResult = tsunamiHeight->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                {
                    assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                    return false;
                }

                // �l�e�L�X�g�擾
                BSTR valueText;
                hResult = pXMLDOMMemberElement->get_text(&valueText);
                if (SUCCEEDED(hResult))
                {
                    // BSTR��std::string�ϊ�
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
        // �l���擾
        MSXML2::IXMLDOMNodePtr tsunamiHeightValue = 0;
        BSTR val = SysAllocString(XPATH_genericAttributeSet2_2);
        tsunamiHeightNode->selectSingleNode(val, &tsunamiHeightValue);
        if (NULL != tsunamiHeightValue)
        {
            // �m�[�h�^�C�v�擾
            MSXML2::DOMNodeType eMemberNodeType;
            hResult = tsunamiHeightValue->get_nodeType(&eMemberNodeType);
            if (FAILED(hResult))
            {
                assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                return false;
            }

            // �G�������g�^�ւ̕ϊ�
            MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
            hResult = tsunamiHeightValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
            if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
            {
                assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                return false;
            }

            // �l�e�L�X�g�擾
            BSTR valueText;
            hResult = pXMLDOMMemberElement->get_text(&valueText);
            if (SUCCEEDED(hResult))
            {
                // BSTR��std::string�ϊ�
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

// �y���ЊQ���X�N���擾����
bool getLandSlideRisk(const MSXML2::IXMLDOMNodePtr& pXMLDOMNode)
{
    // �y���ЊQ���X�N�̃m�[�h���擾
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
    {   // �擾���s
        return false;
    }

    return true;
}

// �\������ʂ��擾����
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
    {   // �擾���s
        return false;
    }

    // �m�[�h�^�C�v�擾
    MSXML2::DOMNodeType eMemberNodeType;
    hResult = structureType->get_nodeType(&eMemberNodeType);
    if (FAILED(hResult))
    {
        assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
        return false;
    }

    // �G�������g�^�ւ̕ϊ�
    MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
    hResult = structureType->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
    if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
    {
        assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
        return false;
    }

    // �l�e�L�X�g�擾
    BSTR valueText;
    hResult = pXMLDOMMemberElement->get_text(&valueText);
    if (SUCCEEDED(hResult))
    {
        // BSTR��std::string�ϊ�
        std::string valueStr = ConvertBSTRToMBS(valueText);
        iBldStructureType = stoi(valueStr);
        bret = true;
    }

    return bret;
}

// ���������擾
vector<BUILDING> GetBldgAttribute(wstring xmldata)
{
    //xml�I�u�W�F�N�g����
    MSXML2::IXMLDOMDocument2Ptr reader;
    reader.CreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_INPROC_SERVER);

    //xpath���T�|�[�g����悤�ɐݒ�
    reader->setProperty(CComBSTR(L"SelectionLanguage"), CComVariant(L"XPath"));
    //namespace���T�|�[�g����悤�ɐݒ�
    std::wstring ns = GetSelectionNamespaces();
    reader->setProperty(CComBSTR(L"SelectionNamespaces"), CComVariant(ns.c_str()));

    //���[�h
    VARIANT_BOOL isSuccessful;
    reader->load(CComVariant(xmldata.c_str()), &isSuccessful);

    // �������X�g
    vector<BUILDING> allBuildingList{};

    HRESULT hResult;

    BSTR xp2 = SysAllocString(XPATH2);
    MSXML2::IXMLDOMNodeListPtr buildingList = NULL;
    reader->selectNodes(xp2, &buildingList);

    // �m�[�h�����擾
    long lCountNode = 0;
    hResult = buildingList->get_length(&lCountNode);

    // ���������J��Ԃ�
    for (int i = 0; i < lCountNode; i++) {
        // ������񏉊���
        BUILDING buildingInfo{};

        // �m�[�h���X�g�̂����̈�̃m�[�h�̎擾
        MSXML2::IXMLDOMNodePtr pXMLDOMNode = NULL;
        hResult = buildingList->get_item(i, &pXMLDOMNode);
        if (FAILED(hResult))
        {
            assert(!"�m�[�h���X�g�̂����̈�̃m�[�h�̎擾�Ɏ��s");
            continue;
        }

        // ����ID
        std::string buildId = "";
        if (!getBuildId(pXMLDOMNode, buildId))
        {
            assert(!"����ID�̎擾�Ɏ��s");
            continue;
        }
        buildingInfo.strBuildingId = buildId;

        // �\�����˗�
        MSXML2::IXMLDOMNodePtr solorRadiation = 0;
        BSTR solor = SysAllocString(XPATH_measureAttribute1);
        pXMLDOMNode->selectSingleNode(solor, &solorRadiation);
        if (NULL != solorRadiation)
        {
            // �l���擾
            MSXML2::IXMLDOMNodePtr solorRadiationValue = 0;
            BSTR val = SysAllocString(XPATH_measureAttribute2);
            solorRadiation->selectSingleNode(val, &solorRadiationValue);
            if (NULL != solorRadiationValue)
            {
                // �m�[�h�^�C�v�擾
                MSXML2::DOMNodeType eMemberNodeType;
                hResult = solorRadiationValue->get_nodeType(&eMemberNodeType);
                if (FAILED(hResult))
                {
                    assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                    continue;
                }

                // �G�������g�^�ւ̕ϊ�
                MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                hResult = solorRadiationValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                {
                    assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                    continue;
                }

                // �l�e�L�X�g�擾
                BSTR valueText;
                hResult = pXMLDOMMemberElement->get_text(&valueText);
                if (SUCCEEDED(hResult))
                {
                    // BSTR��std::string�ϊ�
                    std::string valueStr = ConvertBSTRToMBS(valueText);
                    buildingInfo.dSolorRadiation = stod(valueStr);

                }
            }
        }
        else
        {
            continue;
        }

        // ����
        MSXML2::IXMLDOMNodePtr height = 0;
        BSTR ht = SysAllocString(XPATH_measuredHeight1);
        pXMLDOMNode->selectSingleNode(ht, &height);
        if (NULL != height)
        {
            // �m�[�h�^�C�v�擾
            MSXML2::DOMNodeType eMemberNodeType;
            hResult = height->get_nodeType(&eMemberNodeType);
            if (FAILED(hResult))
            {
                assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                continue;
            }

            // �G�������g�^�ւ̕ϊ�
            MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
            hResult = height->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
            if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
            {
                assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                continue;
            }

            // �l�e�L�X�g�擾
            BSTR valueText;
            hResult = pXMLDOMMemberElement->get_text(&valueText);
            if (SUCCEEDED(hResult))
            {
                // BSTR��std::string�ϊ�
                std::string valueStr = ConvertBSTRToMBS(valueText);
                buildingInfo.dBuildingHeight = stod(valueStr);

            }
                
        }

        // �\�����
        int iBldStructureType = 0;
        if (getBuildStructureType(pXMLDOMNode, iBldStructureType))
        {
            buildingInfo.iBldStructureType = iBldStructureType;
        }

        // �s�s���Ƃ̓Ǝ��敪�Ɋ�Â����z�\���̎��
        // �s�s���Ƃ̓Ǝ��敪�Ɋ�Â��n��K���͈̔�
        BSTR xpExtended1 = SysAllocString(XPATH_extendedAttribute1);
        MSXML2::IXMLDOMNodeListPtr extendedAttributeList = NULL;
        pXMLDOMNode->selectNodes(xpExtended1, &extendedAttributeList);

        // �m�[�h�����擾
        long lExtendedAttributeCountNode = 0;
        hResult = extendedAttributeList->get_length(&lExtendedAttributeCountNode);

        // �s�s���Ƃ̓Ǝ��敪�����J��Ԃ�
        for (int j = 0; j < lExtendedAttributeCountNode; j++) {

            // �m�[�h���X�g�̂����̈�̃m�[�h�̎擾
            MSXML2::IXMLDOMNodePtr pXMLDOMExtendedNode = NULL;
            hResult = extendedAttributeList->get_item(j, &pXMLDOMExtendedNode);
            if (FAILED(hResult))
            {
                assert(!"�m�[�h���X�g�̂����̈�̃m�[�h�̎擾�Ɏ��s");
                continue;
            }

            MSXML2::IXMLDOMNodePtr keyValuePair = 0;
            BSTR key = SysAllocString(XPATH_extendedAttribute2);
            pXMLDOMExtendedNode->selectSingleNode(key, &keyValuePair);
            if (NULL != keyValuePair)
            {
                // �l���擾
                    // �m�[�h�^�C�v�擾
                MSXML2::DOMNodeType ekeyValuePairNodeType;
                hResult = keyValuePair->get_nodeType(&ekeyValuePairNodeType);
                if (FAILED(hResult))
                {
                    assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                    continue;
                }

                // �G�������g�^�ւ̕ϊ�
                MSXML2::IXMLDOMElementPtr pXMLDOMkeyValuePairElement = NULL;
                hResult = keyValuePair->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMkeyValuePairElement);
                if (FAILED(hResult) || NULL == pXMLDOMkeyValuePairElement)
                {
                    assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                    continue;
                }

                // �l�e�L�X�g�擾
                BSTR valueText;
                std::string keyValueStr;
                hResult = pXMLDOMkeyValuePairElement->get_text(&valueText);
                if (SUCCEEDED(hResult))
                {
                    // BSTR��std::string�ϊ�
                    keyValueStr = ConvertBSTRToMBS(valueText);
                }

                // ���z�\���̎��or�n��K���͈̔�
                if (keyValueStr == "10" || keyValueStr == "100")
                {
                    // �l���擾
                    MSXML2::IXMLDOMNodePtr codeValue = 0;
                    BSTR val = SysAllocString(XPATH_extendedAttribute3);
                    pXMLDOMExtendedNode->selectSingleNode(val, &codeValue);
                    if (NULL != codeValue)
                    {
                        // �m�[�h�^�C�v�擾
                        MSXML2::DOMNodeType eCodeValueNodeType;
                        hResult = codeValue->get_nodeType(&eCodeValueNodeType);
                        if (FAILED(hResult))
                        {
                            assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                            continue;
                        }

                        // �G�������g�^�ւ̕ϊ�
                        MSXML2::IXMLDOMElementPtr pXMLDOMCodeValueElement = NULL;
                        hResult = codeValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMCodeValueElement);
                        if (FAILED(hResult) || NULL == pXMLDOMCodeValueElement)
                        {
                            assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                            continue;
                        }

                        // �l�e�L�X�g�擾
                        BSTR valueText;
                        hResult = pXMLDOMCodeValueElement->get_text(&valueText);
                        if (SUCCEEDED(hResult))
                        {
                            // BSTR��std::string�ϊ�
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

        // �����ʍ��W
        BSTR xp4 = SysAllocString(XPATH4);
        MSXML2::IXMLDOMNodeListPtr roofSurfaceList = NULL;
        pXMLDOMNode->selectNodes(xp4, &roofSurfaceList);

        // �m�[�h�����擾
        long lRoofSurfaceCountNode = 0;
        hResult = roofSurfaceList->get_length(&lRoofSurfaceCountNode);

        // ���������J��Ԃ�
        for (int j = 0; j < lRoofSurfaceCountNode; j++) {

            // ������񏉊���
            ROOFSURFACES roofSurfaces{};

            // �m�[�h���X�g�̂����̈�̃m�[�h�̎擾
            MSXML2::IXMLDOMNodePtr pXMLDOMRoofNode = NULL;
            hResult = roofSurfaceList->get_item(j, &pXMLDOMRoofNode);
            if (FAILED(hResult))
            {
                assert(!"�m�[�h���X�g�̂����̈�̃m�[�h�̎擾�Ɏ��s");
                continue;
            }

            // �������^�O�I��
            MSXML2::IXMLDOMNodePtr roofSurface = 0;
            BSTR roof = SysAllocString(XPATH5);
            pXMLDOMRoofNode->selectSingleNode(roof, &roofSurface);
            // ������񂪂���Ώ��������s
            if (NULL != roofSurface) {
                // �m�[�h�^�C�v�擾
                MSXML2::DOMNodeType eRoofNodeType;
                hResult = roofSurface->get_nodeType(&eRoofNodeType);
                if (FAILED(hResult))
                {
                    assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                    continue;
                }

                // �G�������g�^�ւ̕ϊ�
                MSXML2::IXMLDOMElementPtr pXMLDOMRoofElement = NULL;
                hResult = roofSurface->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMRoofElement);
                if (FAILED(hResult) || NULL == pXMLDOMRoofElement)
                {
                    assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                    continue;
                }

                // ����ID���擾
                MSXML2::IXMLDOMAttribute* pAttributeRoofNode = NULL;
                CComVariant varValue;
                BSTR id = SysAllocString(L"gml:id");
                hResult = pXMLDOMRoofElement->getAttribute(id, &varValue);
                if (SUCCEEDED(hResult) && VT_BSTR == varValue.vt)
                {
                    // BSTR��std::string�ϊ�
                    std::string roofSurfaceId = ConvertBSTRToMBS(varValue.bstrVal);
                    roofSurfaces.roofSurfaceId = roofSurfaceId;

                }

                // �����ڍ׏��擾(�m�[�h)
                BSTR xp6 = SysAllocString(XPATH6);
                MSXML2::IXMLDOMNodeListPtr surfaceMemberList = NULL;
                pXMLDOMRoofNode->selectNodes(xp6, &surfaceMemberList);

                // �m�[�h�����擾
                long lSurfaceMemberCountNode = 0;
                hResult = surfaceMemberList->get_length(&lSurfaceMemberCountNode);

                // �����ڍא����J��Ԃ�
                for (int k = 0; k < lSurfaceMemberCountNode; k++) {

                    // ���[�N�����ڍ׏�����
                    SURFACEMEMBERS wSurfaceMembers{};

                    // �m�[�h���X�g�̂����̈�̃m�[�h�̎擾
                    MSXML2::IXMLDOMNodePtr pXMLDOMSurfaceNode = NULL;
                    hResult = surfaceMemberList->get_item(k, &pXMLDOMSurfaceNode);
                    if (FAILED(hResult))
                    {
                        assert(!"�m�[�h���X�g�̂����̈�̃m�[�h�̎擾�Ɏ��s");
                        continue;
                    }


                    // ���W�^�O�I��
                    MSXML2::IXMLDOMNodePtr Position = 0;
                    BSTR pos = SysAllocString(XPATH7);
                    pXMLDOMSurfaceNode->selectSingleNode(pos, &Position);

                    if (NULL != Position) {
                        // �m�[�h�^�C�v�擾
                        MSXML2::DOMNodeType eMemberNodeType;
                        hResult = Position->get_nodeType(&eMemberNodeType);
                        if (FAILED(hResult))
                        {
                            assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                            continue;
                        }

                        // �G�������g�^�ւ̕ϊ�
                        MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                        hResult = Position->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                        if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                        {
                            assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                            continue;
                        }

                        // ���W�e�L�X�g�擾
                        BSTR positionText;
                        hResult = pXMLDOMMemberElement->get_text(&positionText);
                        if (SUCCEEDED(hResult))
                        {
                            // BSTR��std::string�ϊ�
                            std::string posStr = ConvertBSTRToMBS(positionText);

                            // ���W����
                            vector<std::string> posAry = split(posStr, " ");

                            // ���W�i�[
                            int posCnt = 0;
                            // �ꎞ���W���X�g������
                            CPointBase wPosition{};

                            // ���ʒ��p�ϊ��p
                            double dEast, dNorth;

                            for (int x = 0; x < posAry.size(); x++) {

                                if (posCnt == 0) {
                                    // double�ɕϊ�
                                    wPosition.y = std::stod(posAry[x]);

                                    posCnt++;
                                }
                                else if (posCnt == 1) {
                                    // double�ɕϊ�
                                    wPosition.x = std::stod(posAry[x]);

                                    posCnt++;
                                }
                                else if (posCnt == 2) {
                                    // double�ɕϊ�
                                    wPosition.z = std::stod(posAry[x]);

                                    // ���ʒ��p���W�ɕϊ�

                                    CGeoUtil::LonLatToXY(wPosition.x, wPosition.y, JPZONE, dEast, dNorth);
                                    CPointBase pt(dEast, dNorth, wPosition.z);

                                    // �����ڍ׃��X�g�����W���X�g�ɒǉ�                                                
                                    wSurfaceMembers.posList.emplace_back(pt);

                                    // �ꎞ���W���X�g������
                                    CPointBase wPosition{};
                                    posCnt = 0;
                                }

                            }
                        }
                    }
                    else {
                        continue;
                    }
                    // �����ڍ׃��X�g�ɒǉ�
                    roofSurfaces.roofSurfaceList.emplace_back(wSurfaceMembers);
                }
                // �������X�g�ɒǉ�
                buildingInfo.roofSurfaceList.emplace_back(roofSurfaces);
            }

        }

        // �������X�g�ɒǉ�
        allBuildingList.emplace_back(buildingInfo);

    }


    //xml�I�u�W�F�N�g���
    reader.Release();

    return allBuildingList;

}

/// <summary>
/// ���b�V���̃o�E���f�B���O���擾
/// </summary>
/// <returns></returns>
bool GetMeshBounding(wstring xmldata, POSITION& upperCorner, POSITION& lowerCorner)
{
    //xml�I�u�W�F�N�g����
    MSXML2::IXMLDOMDocument2Ptr reader;
    reader.CreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_INPROC_SERVER);

    //xpath���T�|�[�g����悤�ɐݒ�
    reader->setProperty(CComBSTR(L"SelectionLanguage"), CComVariant(L"XPath"));
    //namespace���T�|�[�g����悤�ɐݒ�
    std::wstring ns = GetSelectionNamespaces();
    reader->setProperty(CComBSTR(L"SelectionNamespaces"), CComVariant(ns.c_str()));

    //���[�h
    VARIANT_BOOL isSuccessful;
    reader->load(CComVariant(xmldata.c_str()), &isSuccessful);

    // �ŏ��ܓx�o�x�擾
    HRESULT hBoundResult;
    MSXML2::IXMLDOMNodePtr lowerCornerNode = 0;
    BSTR bound = SysAllocString(BOUND_XPATH1);
    reader->selectSingleNode(bound, &lowerCornerNode);

    if (NULL != lowerCornerNode) {
        // �m�[�h�^�C�v�擾
        MSXML2::DOMNodeType eMemberNodeType;
        hBoundResult = lowerCornerNode->get_nodeType(&eMemberNodeType);
        if (FAILED(hBoundResult))
        {
            assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
            return false;
        }
        else {
            // �G�������g�^�ւ̕ϊ�
            MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
            hBoundResult = lowerCornerNode->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
            if (FAILED(hBoundResult) || NULL == pXMLDOMMemberElement)
            {
                assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                return false;
            }
            else {

                // ���W�e�L�X�g�擾
                BSTR positionText;
                hBoundResult = pXMLDOMMemberElement->get_text(&positionText);
                if (SUCCEEDED(hBoundResult))
                {
                    // BSTR��std::string�ϊ�
                    std::string posStr = ConvertBSTRToMBS(positionText);

                    // ���W����
                    vector<std::string> posAry = split(posStr, " ");

                    // ���W�i�[
                    int posCnt = 0;
                    // �ꎞ���W���X�g������
                    POSITION wPosition{};

                    for (int x = 0; x < posAry.size(); x++) {

                        if (posCnt == 0) {
                            // double�ɕϊ�
                            wPosition.lat = std::stod(posAry[x]);

                            posCnt++;
                        }
                        else if (posCnt == 1) {
                            // double�ɕϊ�
                            wPosition.lon = std::stod(posAry[x]);

                            posCnt++;
                        }
                        else if (posCnt == 2) {
                            // double�ɕϊ�
                            wPosition.ht = std::stod(posAry[x]);

                            // �ꎞ���W���X�g������
                            POSITION wPosition{};
                            posCnt = 0;
                        }

                    }

                    // �l��ݒ�
                    lowerCorner.lat = wPosition.lat;
                    lowerCorner.lon = wPosition.lon;
                    lowerCorner.ht = wPosition.ht;
                }
            }
        }
    }

    // �ő�ܓx�o�x�擾
    MSXML2::IXMLDOMNodePtr upperCornerNode = 0;
    bound = SysAllocString(BOUND_XPATH2);
    reader->selectSingleNode(bound, &upperCornerNode);

    if (NULL != upperCornerNode) {
        // �m�[�h�^�C�v�擾
        MSXML2::DOMNodeType eMemberNodeType;
        hBoundResult = upperCornerNode->get_nodeType(&eMemberNodeType);
        if (FAILED(hBoundResult))
        {
            assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
            return false;
        }
        else {
            // �G�������g�^�ւ̕ϊ�
            MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
            hBoundResult = upperCornerNode->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
            if (FAILED(hBoundResult) || NULL == pXMLDOMMemberElement)
            {
                assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                return false;
            }
            else {

                // ���W�e�L�X�g�擾
                BSTR positionText;
                hBoundResult = pXMLDOMMemberElement->get_text(&positionText);
                if (SUCCEEDED(hBoundResult))
                {
                    // BSTR��std::string�ϊ�
                    std::string posStr = ConvertBSTRToMBS(positionText);

                    // ���W����
                    vector<std::string> posAry = split(posStr, " ");

                    // ���W�i�[
                    int posCnt = 0;
                    // �ꎞ���W���X�g������
                    POSITION wPosition{};

                    for (int x = 0; x < posAry.size(); x++) {

                        if (posCnt == 0) {
                            // double�ɕϊ�
                            wPosition.lat = std::stod(posAry[x]);

                            posCnt++;
                        }
                        else if (posCnt == 1) {
                            // double�ɕϊ�
                            wPosition.lon = std::stod(posAry[x]);

                            posCnt++;
                        }
                        else if (posCnt == 2) {
                            // double�ɕϊ�
                            wPosition.ht = std::stod(posAry[x]);

                            // �ꎞ���W���X�g������
                            POSITION wPosition{};
                            posCnt = 0;
                        }

                    }

                    // �l��ݒ�
                    upperCorner.lat = wPosition.lat;
                    upperCorner.lon = wPosition.lon;
                    upperCorner.ht = wPosition.ht;
                }
            }
        }
    }

    //xml�I�u�W�F�N�g���
    reader.Release();
    //COM�̉��
    CoUninitialize();

    return true;
}

// �͈͂����b�V���͈͓��ɂ��邩�ǂ����𔻒�
bool IsAreaInMesh(std::vector<CPoint2D> areaPoints, double meshMinX, double meshMinY, double meshMaxX, double meshMaxY)
{
    // ���b�V���͈�
    vector<CPoint2D> point2DList{};
    point2DList.emplace_back(CPoint2D(meshMinX, meshMaxY));
    point2DList.emplace_back(CPoint2D(meshMinX, meshMinY));
    point2DList.emplace_back(CPoint2D(meshMaxX, meshMinY));
    point2DList.emplace_back(CPoint2D(meshMaxX, meshMaxY));
    CPoint2D* pt2D = &point2DList[0];

    for (auto& pt : areaPoints)
    {
        // ��͔͈͂̒��_�Ŕ���
        if (CGeoUtil::IsPointInPolygon(pt, point2DList.size(), pt2D))
        {
            return true;
        }
    }

    return false;
}

// 2D�Ώۖʂ̉�͔͈͓��O����
bool IsPolygonInArea(std::vector<CPoint2D> areaPoints, std::vector<CPoint2D> targetPoints, CPoint2DPolygon& crsPolygon)
{
    if (areaPoints.size() < 3)  return false;
    if (targetPoints.size() < 3)  return false;

    // �|���S�����쐬
    CPoint2DPolygon areaPolygon, targetPolygon;
    int areaPointsSize = (areaPoints[0] == areaPoints[areaPoints.size() - 1]) ? areaPoints.size() - 1 : areaPoints.size();   // �n�_�ƏI�_�������ꍇ�͍Ō�̓_��ǉ����Ȃ�
    for (int i = 0; i < areaPointsSize; i++)
    {
        areaPolygon.Add(areaPoints[i]);
    }
    int targetPointsSize = (targetPoints[0] == targetPoints[targetPoints.size() - 1]) ? targetPoints.size() - 1 : targetPoints.size();   // �n�_�ƏI�_�������ꍇ�͍Ō�̓_��ǉ����Ȃ�
    for (int i = 0; i < targetPointsSize; i++)
    {
        targetPolygon.Add(targetPoints[i]);
    }

    // ���O����
    if (areaPolygon.GetCrossingPolygon(targetPolygon, crsPolygon))
    {
        return true;
    }

    return false;
}

// fld�f�[�^�擾
// in fld/pref�t�H���_
std::vector<FLDRISKLIST> GetFldData(wstring folderPath, string cancelPath)
{
    std::vector<FLDRISKLIST> allRiskList{};

    std::vector<std::string> fileName;

    HRESULT hResult;
    hResult = CoInitialize(NULL); // ������

    // �t�H���_���擾
    std::filesystem::directory_iterator iter(folderPath), end;
    std::error_code err;
    for (; iter != end && !err; iter.increment(err)) {
        const std::filesystem::directory_entry entry = *iter;
        if (!entry.is_directory()) continue;

        std::filesystem::path typedir = entry.path();

        // �^���Z�����X�g
        FLDRISKLIST fldRisk{};

        // �^���Z���z���悲�Ƃ̃t�H���_�����擾
        std::string dirname = typedir.filename().string();
        fldRisk.type = dirname;

        // �T�u�t�H���_���擾
        std::filesystem::directory_iterator subiter(typedir), subend;
        std::error_code suberr;
        for (; subiter != subend && !suberr; subiter.increment(suberr)) {
            const std::filesystem::directory_entry subentry = *subiter;
            if (!subentry.is_directory()) continue;

            std::filesystem::path subdir = subentry.path();

            // �^���Z���z����}���Ƃ̃t�H���_�����擾
            std::string dirname = subdir.filename().string();
            fldRisk.description = dirname;

            // �t�@�C�����擾
            if (getFileNames(subdir.string(), ".gml", fileName) == true) {

                std::vector<std::string> inAreaFiles;

                for (auto& p : fileName) {
                    // �^���Z���z��f�[�^
                    FLDRISK result{};

                    // �t�@�C�������烁�b�V��ID���擾
                    std::string fullpath = p.c_str();
                    int path_i = (int)fullpath.find_last_of("\\") + 1;
                    int ext_i = (int)fullpath.find_last_of(".");
                    std::string filename = fullpath.substr(path_i, (int64_t)ext_i - path_i);
                    std::string meshId = filename.substr(0, 8);
                    result.meshID = meshId;

                    // �t�@�C��������K�͂��擾
                    std::vector<std::string> splitstr = split(filename, "_");
                    if (splitstr.size() < 4 || 5 < splitstr.size())
                    {
                        assert(!"�ΏۊO��CityGML�t�@�C����");
                        continue;
                    }
                    result.scale = splitstr[3];

                    // ���b�V���̃o�E���f�B���O���擾
                    std::filesystem::path gmlfile = std::filesystem::path(fullpath);
                    if (!GetMeshBounding(gmlfile, result.upperCorner, result.lowerCorner))
                    {
                        continue;
                    }

                    // ���ʒ��p���W�ɕϊ�
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

                    // �t�@�C���̓ǂݍ��ݗL���𔻒�
                    bool isInMesh = false;
                    for (auto& area : allAreaList)
                    {
                        if (area.areaID == "A000")
                        {
                            isInMesh = true;
                            break;
                        }

                        // �͈͂̓��O����
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

                    //xml�I�u�W�F�N�g����
                    MSXML2::IXMLDOMDocument2Ptr reader;
                    reader.CreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_INPROC_SERVER);

                    //xpath���T�|�[�g����悤�ɐݒ�
                    reader->setProperty(CComBSTR(L"SelectionLanguage"), CComVariant(L"XPath"));
                    //namespace���T�|�[�g����悤�ɐݒ�
                    std::wstring ns = GetSelectionNamespaces();
                    reader->setProperty(CComBSTR(L"SelectionNamespaces"), CComVariant(ns.c_str()));

                    //���[�h
                    VARIANT_BOOL isSuccessful;
                    reader->load(CComVariant(p.c_str()), &isSuccessful);

                    BSTR xp2 = SysAllocString(XPATH2);
                    MSXML2::IXMLDOMNodeListPtr objList = NULL;
                    reader->selectNodes(xp2, &objList);

                    // �m�[�h�����擾
                    long lCountNode = 0;
                    hResult = objList->get_length(&lCountNode);

                    for (int i = 0; i < lCountNode; i++) {

                        // �m�[�h���X�g�̂����̈�̃m�[�h�̎擾
                        MSXML2::IXMLDOMNodePtr pXMLDOMNode = NULL;
                        hResult = objList->get_item(i, &pXMLDOMNode);
                        if (FAILED(hResult))
                        {
                            assert(!"�m�[�h���X�g�̂����̈�̃m�[�h�̎擾�Ɏ��s");
                            continue;
                        }

                        // �̏��擾(�m�[�h)
                        BSTR wtr = SysAllocString(WTR_XPATH1);
                        MSXML2::IXMLDOMNodeListPtr waterBody = NULL;
                        pXMLDOMNode->selectNodes(wtr, &waterBody);

                        // �m�[�h�����擾
                        long lWaterCountNode = 0;
                        hResult = waterBody->get_length(&lWaterCountNode);

                        // �̐����J��Ԃ�
                        for (int j = 0; j < lWaterCountNode; j++) {
                            // LOD1�f�[�^������
                            HAZARDRISKLOD1 hrLOD1{};

                            // �m�[�h���X�g�̂����̈�̃m�[�h�̎擾
                            MSXML2::IXMLDOMNodePtr pXMLDOMMemberNode = NULL;
                            hResult = waterBody->get_item(j, &pXMLDOMMemberNode);
                            if (FAILED(hResult))
                            {
                                assert(!"�m�[�h���X�g�̂����̈�̃m�[�h�̎擾�Ɏ��s");
                                continue;
                            }

                            // �ڍ׏��擾(�m�[�h)
                            BSTR txp2 = SysAllocString(WTR_XPATH2);
                            MSXML2::IXMLDOMNodeListPtr surfaceMemberList = NULL;
                            pXMLDOMMemberNode->selectNodes(txp2, &surfaceMemberList);

                            // �m�[�h�����擾
                            long lSurfaceMemberCountNode = 0;
                            hResult = surfaceMemberList->get_length(&lSurfaceMemberCountNode);

                            // �̏ڍא����J��Ԃ�
                            for (int k = 0; k < lSurfaceMemberCountNode; k++) {
                                // ���[�N�Ǐڍ׏�����
                                SURFACEMEMBERS wSurfaceMembers{};

                                // �m�[�h���X�g�̂����̈�̃m�[�h�̎擾
                                MSXML2::IXMLDOMNodePtr pXMLDOMSurfaceNode = NULL;
                                hResult = surfaceMemberList->get_item(k, &pXMLDOMSurfaceNode);
                                if (FAILED(hResult))
                                {
                                    assert(!"�m�[�h���X�g�̂����̈�̃m�[�h�̎擾�Ɏ��s");
                                    continue;
                                }

                                // �|���S���^�O�I��
                                MSXML2::IXMLDOMNodePtr Position = 0;
                                BSTR pos = SysAllocString(XPATH7);
                                pXMLDOMSurfaceNode->selectSingleNode(pos, &Position);

                                if (NULL != Position) {
                                    // �m�[�h�^�C�v�擾
                                    MSXML2::DOMNodeType eMemberNodeType;
                                    hResult = Position->get_nodeType(&eMemberNodeType);
                                    if (FAILED(hResult))
                                    {
                                        assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                                        continue;
                                    }

                                    // �G�������g�^�ւ̕ϊ�
                                    MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                                    hResult = Position->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                                    if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                                    {
                                        assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                                        continue;
                                    }

                                    // ���W�e�L�X�g�擾
                                    BSTR positionText;
                                    hResult = pXMLDOMMemberElement->get_text(&positionText);
                                    if (SUCCEEDED(hResult))
                                    {
                                        // BSTR��std::string�ϊ�
                                        std::string posStr = ConvertBSTRToMBS(positionText);

                                        // ���W����
                                        vector<std::string> posAry = split(posStr, " ");

                                        // ���W�i�[
                                        int posCnt = 0;
                                        // �ꎞ���W���X�g������
                                        CPointBase wPosition{};

                                        // ���ʒ��p�ϊ��p
                                        double dEast, dNorth;

                                        for (int x = 0; x < posAry.size(); x++) {

                                            if (posCnt == 0) {
                                                // double�ɕϊ�
                                                wPosition.y = std::stod(posAry[x]);

                                                posCnt++;
                                            }
                                            else if (posCnt == 1) {
                                                // double�ɕϊ�
                                                wPosition.x = std::stod(posAry[x]);

                                                posCnt++;
                                            }
                                            else if (posCnt == 2) {
                                                // double�ɕϊ�
                                                wPosition.z = std::stod(posAry[x]);

                                                // ���ʒ��p���W�ɕϊ�

                                                CGeoUtil::LonLatToXY(wPosition.x, wPosition.y, JPZONE, dEast, dNorth);
                                                CPointBase pt(dEast, dNorth, wPosition.z);

                                                // �����ڍ׃��X�g�����W���X�g�ɒǉ�                                                
                                                wSurfaceMembers.posList.emplace_back(pt);

                                                // �ꎞ���W���X�g������
                                                CPointBase wPosition{};
                                                posCnt = 0;
                                            }

                                        }
                                    }
                                }
                                else {
                                    continue;
                                }

                                // �ڍ׃��X�g�ɒǉ�
                                hrLOD1.wtrSurfaceList.emplace_back(wSurfaceMembers);
                            }

                            // LOD1���X�g�ɒǉ�
                            result.fldListLOD1.emplace_back(hrLOD1);

                        }

                    }

                    fldRisk.vecFldRisk.emplace_back(result);

                    //xml�I�u�W�F�N�g���
                    reader.Release();

                    // �L�����Z���t�@�C���`�F�b�N
                    if (std::filesystem::exists(cancelPath)) {
                        break;
                    }
                }

                // ���X�g�ɒǉ�
                allRiskList.emplace_back(fldRisk);

            }

            // �L�����Z���t�@�C���`�F�b�N
            if (std::filesystem::exists(cancelPath)) {
                break;
            }
        }

    }

    //COM�̉��
    CoUninitialize();

    return allRiskList;
}

// tnm�f�[�^�擾
std::vector<TNMRISKLIST> GetTnmData(wstring folderPath, string cancelPath)
{
    std::vector<TNMRISKLIST> allRiskList{};

    std::vector<std::string> fileName;

    // GML�t�@�C���������ݏ���
    HRESULT hResult;
    hResult = CoInitialize(NULL); // ������

    // �t�H���_���擾
    std::filesystem::directory_iterator iter(folderPath), end;
    std::error_code err;
    for (; iter != end && !err; iter.increment(err)) {
        const std::filesystem::directory_entry entry = *iter;
        if (!entry.is_directory()) continue;

        std::filesystem::path path = entry.path();

        TNMRISKLIST tnmRisk{};

        // �^���Z���z����}���Ƃ̃t�H���_�����擾
        std::string dirname = path.filename().string();
        tnmRisk.description = dirname;

        // �t�@�C�����擾
        if (getFileNames(path.string(), ".gml", fileName) == true) {

            std::vector<std::string> inAreaFiles;

            for (auto& p : fileName) {
                TNMRISK result{};

                // �t�@�C�������烁�b�V��ID���擾
                std::string fullpath = p.c_str();
                int path_i = (int)fullpath.find_last_of("\\") + 1;
                int ext_i = (int)fullpath.find_last_of(".");
                std::string filename = fullpath.substr(path_i, (int64_t)ext_i - path_i);
                std::string meshId = filename.substr(0, 6);
                result.meshID = meshId;

                // ���b�V���̃o�E���f�B���O���擾
                std::filesystem::path gmlfile = std::filesystem::path(fullpath);
                if (!GetMeshBounding(gmlfile, result.upperCorner, result.lowerCorner))
                {
                    continue;
                }

                // ���ʒ��p���W�ɕϊ�
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

                // �t�@�C���̓ǂݍ��ݗL���𔻒�
                bool isInMesh = false;
                for (auto& area : allAreaList)
                {
                    if (area.areaID == "A000")
                    {
                        isInMesh = true;
                        break;
                    }

                    // �͈͂̓��O����
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

                //xml�I�u�W�F�N�g����
                MSXML2::IXMLDOMDocument2Ptr reader;
                reader.CreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_INPROC_SERVER);

                //xpath���T�|�[�g����悤�ɐݒ�
                reader->setProperty(CComBSTR(L"SelectionLanguage"), CComVariant(L"XPath"));
                //namespace���T�|�[�g����悤�ɐݒ�
                std::wstring ns = GetSelectionNamespaces();
                reader->setProperty(CComBSTR(L"SelectionNamespaces"), CComVariant(ns.c_str()));

                //���[�h
                VARIANT_BOOL isSuccessful;
                reader->load(CComVariant(p.c_str()), &isSuccessful);

                BSTR xp2 = SysAllocString(XPATH2);
                MSXML2::IXMLDOMNodeListPtr objList = NULL;
                reader->selectNodes(xp2, &objList);

                // �m�[�h�����擾
                long lCountNode = 0;
                hResult = objList->get_length(&lCountNode);

                for (int i = 0; i < lCountNode; i++) {

                    // �m�[�h���X�g�̂����̈�̃m�[�h�̎擾
                    MSXML2::IXMLDOMNodePtr pXMLDOMNode = NULL;
                    hResult = objList->get_item(i, &pXMLDOMNode);
                    if (FAILED(hResult))
                    {
                        assert(!"�m�[�h���X�g�̂����̈�̃m�[�h�̎擾�Ɏ��s");
                        continue;
                    }

                    // �̏��擾(�m�[�h)
                    BSTR wtr = SysAllocString(WTR_XPATH1);
                    MSXML2::IXMLDOMNodeListPtr waterBody = NULL;
                    pXMLDOMNode->selectNodes(wtr, &waterBody);

                    // �m�[�h�����擾
                    long lWaterCountNode = 0;
                    hResult = waterBody->get_length(&lWaterCountNode);

                    // �̐����J��Ԃ�
                    for (int j = 0; j < lWaterCountNode; j++) {
                        // LOD1�f�[�^������
                        HAZARDRISKLOD1 hrLOD1{};

                        // �m�[�h���X�g�̂����̈�̃m�[�h�̎擾
                        MSXML2::IXMLDOMNodePtr pXMLDOMMemberNode = NULL;
                        hResult = waterBody->get_item(j, &pXMLDOMMemberNode);
                        if (FAILED(hResult))
                        {
                            assert(!"�m�[�h���X�g�̂����̈�̃m�[�h�̎擾�Ɏ��s");
                            continue;
                        }

                        // �ڍ׏��擾(�m�[�h)
                        BSTR txp2 = SysAllocString(WTR_XPATH2);
                        MSXML2::IXMLDOMNodeListPtr surfaceMemberList = NULL;
                        pXMLDOMMemberNode->selectNodes(txp2, &surfaceMemberList);

                        // �m�[�h�����擾
                        long lSurfaceMemberCountNode = 0;
                        hResult = surfaceMemberList->get_length(&lSurfaceMemberCountNode);

                        // �̏ڍא����J��Ԃ�
                        for (int k = 0; k < lSurfaceMemberCountNode; k++) {
                            // ���[�N�Ǐڍ׏�����
                            SURFACEMEMBERS wSurfaceMembers{};

                            // �m�[�h���X�g�̂����̈�̃m�[�h�̎擾
                            MSXML2::IXMLDOMNodePtr pXMLDOMSurfaceNode = NULL;
                            hResult = surfaceMemberList->get_item(k, &pXMLDOMSurfaceNode);
                            if (FAILED(hResult))
                            {
                                assert(!"�m�[�h���X�g�̂����̈�̃m�[�h�̎擾�Ɏ��s");
                                continue;
                            }

                            // �|���S���^�O�I��
                            MSXML2::IXMLDOMNodePtr Position = 0;
                            BSTR pos = SysAllocString(XPATH7);
                            pXMLDOMSurfaceNode->selectSingleNode(pos, &Position);

                            if (NULL != Position) {
                                // �m�[�h�^�C�v�擾
                                MSXML2::DOMNodeType eMemberNodeType;
                                hResult = Position->get_nodeType(&eMemberNodeType);
                                if (FAILED(hResult))
                                {
                                    assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                                    continue;
                                }

                                // �G�������g�^�ւ̕ϊ�
                                MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                                hResult = Position->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                                if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                                {
                                    assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                                    continue;
                                }

                                // ���W�e�L�X�g�擾
                                BSTR positionText;
                                hResult = pXMLDOMMemberElement->get_text(&positionText);
                                if (SUCCEEDED(hResult))
                                {
                                    // BSTR��std::string�ϊ�
                                    std::string posStr = ConvertBSTRToMBS(positionText);

                                    // ���W����
                                    vector<std::string> posAry = split(posStr, " ");

                                    // ���W�i�[
                                    int posCnt = 0;
                                    // �ꎞ���W���X�g������
                                    CPointBase wPosition{};

                                    // ���ʒ��p�ϊ��p
                                    double dEast, dNorth;

                                    for (int x = 0; x < posAry.size(); x++) {

                                        if (posCnt == 0) {
                                            // double�ɕϊ�
                                            wPosition.y = std::stod(posAry[x]);

                                            posCnt++;
                                        }
                                        else if (posCnt == 1) {
                                            // double�ɕϊ�
                                            wPosition.x = std::stod(posAry[x]);

                                            posCnt++;
                                        }
                                        else if (posCnt == 2) {
                                            // double�ɕϊ�
                                            wPosition.z = std::stod(posAry[x]);

                                            // ���ʒ��p���W�ɕϊ�

                                            CGeoUtil::LonLatToXY(wPosition.x, wPosition.y, JPZONE, dEast, dNorth);
                                            CPointBase pt(dEast, dNorth, wPosition.z);

                                            // �����ڍ׃��X�g�����W���X�g�ɒǉ�                                                
                                            wSurfaceMembers.posList.emplace_back(pt);

                                            // �ꎞ���W���X�g������
                                            CPointBase wPosition{};
                                            posCnt = 0;
                                        }

                                    }
                                }
                            }
                            else {
                                continue;
                            }

                            // �ڍ׃��X�g�ɒǉ�
                            hrLOD1.wtrSurfaceList.emplace_back(wSurfaceMembers);
                        }

                        // LOD1���X�g�ɒǉ�
                        result.tnmRiskLOD1.emplace_back(hrLOD1);
                    }

                }

                tnmRisk.vecTnmRisk.emplace_back(result);

                //xml�I�u�W�F�N�g���
                reader.Release();

                // �L�����Z���t�@�C���`�F�b�N
                if (std::filesystem::exists(cancelPath)) {
                    break;
                }

            }

        }

        // ���X�g�ɒǉ�
        allRiskList.emplace_back(tnmRisk);

        // �L�����Z���t�@�C���`�F�b�N
        if (std::filesystem::exists(cancelPath)) {
            break;
        }
    }

    //COM�̉��
    CoUninitialize();

    return allRiskList;
}

// lsld�f�[�^�擾
std::vector<LSLDRISK> GetLsldData(wstring folderPath, string cancelPath)
{
    std::vector<LSLDRISK> allRiskList{};

    std::vector<std::string> fileName;

    // GML�t�@�C���������ݏ���
    HRESULT hResult;
    hResult = CoInitialize(NULL); // ������

    std::filesystem::path path = std::filesystem::path(folderPath);

    // �t�@�C�����擾
    if (getFileNames(path.string(), ".gml", fileName) == true) {

        std::vector<std::string> inAreaFiles;

        for (auto& p : fileName) {
            LSLDRISK result{};

            // �t�@�C�������烁�b�V��ID���擾
            std::string fullpath = p.c_str();
            int path_i = (int)fullpath.find_last_of("\\") + 1;
            int ext_i = (int)fullpath.find_last_of(".");
            std::string filename = fullpath.substr(path_i, (int64_t)ext_i - path_i);
            std::string meshId = filename.substr(0, 6);
            result.meshID = meshId;

            // ���b�V���̃o�E���f�B���O���擾
            std::filesystem::path gmlfile = std::filesystem::path(fullpath);
            if (!GetMeshBounding(gmlfile, result.upperCorner, result.lowerCorner))
            {
                continue;
            }

            // ���ʒ��p���W�ɕϊ�
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

            // �t�@�C���̓ǂݍ��ݗL���𔻒�
            bool isInMesh = false;
            for (auto& area : allAreaList)
            {
                if (area.areaID == "A000")
                {
                    isInMesh = true;
                    break;
                }

                // �͈͂̓��O����
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

            //xml�I�u�W�F�N�g����
            MSXML2::IXMLDOMDocument2Ptr reader;
            reader.CreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_INPROC_SERVER);

            //xpath���T�|�[�g����悤�ɐݒ�
            reader->setProperty(CComBSTR(L"SelectionLanguage"), CComVariant(L"XPath"));
            //namespace���T�|�[�g����悤�ɐݒ�
            std::wstring ns = GetSelectionNamespaces();
            reader->setProperty(CComBSTR(L"SelectionNamespaces"), CComVariant(ns.c_str()));

            //���[�h
            VARIANT_BOOL isSuccessful;
            reader->load(CComVariant(p.c_str()), &isSuccessful);

            BSTR xp2 = SysAllocString(XPATH2);
            MSXML2::IXMLDOMNodeListPtr objList = NULL;
            reader->selectNodes(xp2, &objList);

            // �m�[�h�����擾
            long lCountNode = 0;
            hResult = objList->get_length(&lCountNode);

            for (int i = 0; i < lCountNode; i++) {

                // �m�[�h���X�g�̂����̈�̃m�[�h�̎擾
                MSXML2::IXMLDOMNodePtr pXMLDOMNode = NULL;
                hResult = objList->get_item(i, &pXMLDOMNode);
                if (FAILED(hResult))
                {
                    assert(!"�m�[�h���X�g�̂����̈�̃m�[�h�̎擾�Ɏ��s");
                    continue;
                }

                // �̏��擾(�m�[�h)
                BSTR lsld = SysAllocString(LSLD_XPATH1);
                MSXML2::IXMLDOMNodeListPtr area = NULL;
                pXMLDOMNode->selectNodes(lsld, &area);

                // �m�[�h�����擾
                long lAreaCountNode = 0;
                hResult = area->get_length(&lAreaCountNode);

                // �̐����J��Ԃ�
                for (int j = 0; j < lAreaCountNode; j++) {
                    // LOD1�f�[�^������
                    HAZARDRISKLOD1 hrLOD1{};

                    // �m�[�h���X�g�̂����̈�̃m�[�h�̎擾
                    MSXML2::IXMLDOMNodePtr pXMLDOMMemberNode = NULL;
                    hResult = area->get_item(j, &pXMLDOMMemberNode);
                    if (FAILED(hResult))
                    {
                        assert(!"�m�[�h���X�g�̂����̈�̃m�[�h�̎擾�Ɏ��s");
                        continue;
                    }

                    // �ڍ׏��擾(�m�[�h)
                    BSTR txp2 = SysAllocString(LSLD_XPATH2);
                    MSXML2::IXMLDOMNodeListPtr surfaceMemberList = NULL;
                    pXMLDOMMemberNode->selectNodes(txp2, &surfaceMemberList);

                    // �m�[�h�����擾
                    long lSurfaceMemberCountNode = 0;
                    hResult = surfaceMemberList->get_length(&lSurfaceMemberCountNode);

                    // �̏ڍא����J��Ԃ�
                    for (int k = 0; k < lSurfaceMemberCountNode; k++) {
                        // ���[�N�Ǐڍ׏�����
                        SURFACEMEMBERS wSurfaceMembers{};

                        // �m�[�h���X�g�̂����̈�̃m�[�h�̎擾
                        MSXML2::IXMLDOMNodePtr pXMLDOMSurfaceNode = NULL;
                        hResult = surfaceMemberList->get_item(k, &pXMLDOMSurfaceNode);
                        if (FAILED(hResult))
                        {
                            assert(!"�m�[�h���X�g�̂����̈�̃m�[�h�̎擾�Ɏ��s");
                            continue;
                        }

                        // �|���S���^�O�I��
                        MSXML2::IXMLDOMNodePtr Position = 0;
                        BSTR pos = SysAllocString(XPATH7);
                        pXMLDOMSurfaceNode->selectSingleNode(pos, &Position);

                        if (NULL != Position) {
                            // �m�[�h�^�C�v�擾
                            MSXML2::DOMNodeType eMemberNodeType;
                            hResult = Position->get_nodeType(&eMemberNodeType);
                            if (FAILED(hResult))
                            {
                                assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                                continue;
                            }

                            // �G�������g�^�ւ̕ϊ�
                            MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                            hResult = Position->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                            if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                            {
                                assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                                continue;
                            }

                            // ���W�e�L�X�g�擾
                            BSTR positionText;
                            hResult = pXMLDOMMemberElement->get_text(&positionText);
                            if (SUCCEEDED(hResult))
                            {
                                // BSTR��std::string�ϊ�
                                std::string posStr = ConvertBSTRToMBS(positionText);

                                // ���W����
                                vector<std::string> posAry = split(posStr, " ");

                                // ���W�i�[
                                int posCnt = 0;
                                // �ꎞ���W���X�g������
                                CPointBase wPosition{};

                                // ���ʒ��p�ϊ��p
                                double dEast, dNorth;

                                for (int x = 0; x < posAry.size(); x++) {

                                    if (posCnt == 0) {
                                        // double�ɕϊ�
                                        wPosition.y = std::stod(posAry[x]);

                                        posCnt++;
                                    }
                                    else if (posCnt == 1) {
                                        // double�ɕϊ�
                                        wPosition.x = std::stod(posAry[x]);

                                        posCnt++;
                                    }
                                    else if (posCnt == 2) {
                                        // double�ɕϊ�
                                        wPosition.z = std::stod(posAry[x]);

                                        // ���ʒ��p���W�ɕϊ�

                                        CGeoUtil::LonLatToXY(wPosition.x, wPosition.y, JPZONE, dEast, dNorth);
                                        CPointBase pt(dEast, dNorth, wPosition.z);

                                        // �����ڍ׃��X�g�����W���X�g�ɒǉ�                                                
                                        wSurfaceMembers.posList.emplace_back(pt);

                                        // �ꎞ���W���X�g������
                                        CPointBase wPosition{};
                                        posCnt = 0;
                                    }

                                }
                            }
                        }
                        else {
                            continue;
                        }

                        // �ڍ׃��X�g�ɒǉ�
                        hrLOD1.wtrSurfaceList.emplace_back(wSurfaceMembers);
                    }

                    // LOD1���X�g�ɒǉ�
                    result.lsldRiskLOD1.emplace_back(hrLOD1);
                }

            }

            // ���X�g�ɒǉ�
            allRiskList.emplace_back(result);

            //xml�I�u�W�F�N�g���
            reader.Release();


            // �L�����Z���t�@�C���`�F�b�N
            if (std::filesystem::exists(cancelPath)) {
                break;
            }
        }

    }

    //COM�̉��
    CoUninitialize();

    return allRiskList;
}

void SetBldgHazardRisk(std::string meshId, vector<BUILDING>& result)
{

    // �������ƂɍЊQ���X�N�̒l��ݒ�
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

                    // �^���Z���[
                    if (hazardRiskData.fldRisks.size() > 0)
                    {
                        double dMaxDepth = 0.0;
                        for (const auto& fldList : hazardRiskData.fldRisks)
                        {
                            for (const auto& fld : fldList.vecFldRisk)
                            {
                                if (fld.meshID.find(meshId) == std::string::npos) continue;

                                bool isMaxDepth = false;    // �ő�[���ǂ���
                                for (const auto& lod1 : fld.fldListLOD1)
                                {
                                    for (const auto& member : lod1.wtrSurfaceList)
                                    {
                                        // ���O����p
                                        vector<CPoint2D> area2DList{};
                                        for (auto& pos : member.posList)
                                        {
                                            CPoint2D p2D = CPoint2D(pos.x, pos.y);
                                            area2DList.emplace_back(p2D);
                                        }
                                        CPoint2D* ptArea2D = &area2DList[0];
                                        int ptSize = (int)area2DList.size();

                                        // LOD1���Ɍ���LOD2�����邩
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
                                    if (isMaxDepth) break; // �ő�[�̏ꍇ�͎��̃f�[�^������
                                }
                                if (isMaxDepth) break;
                            }

                            // �^���Z���[(�ő�l)��ݒ�
                            bldg.dFloodDepth = dMaxDepth;

                            if (dMaxDepth != 0.0) break;
                        }
                    }

                    // �Ôg�Z���[
                    if (hazardRiskData.tnmRisks.size() > 0)
                    {
                        double dMaxDepth = 0.0;
                        for (const auto& tnmList : hazardRiskData.tnmRisks)
                        {
                            for (const auto& tnm : tnmList.vecTnmRisk)
                            {
                                if (tnm.meshID.find(meshId) == std::string::npos) continue;

                                bool isMaxDepth = false;    // �ő�[���ǂ���
                                for (const auto& lod1 : tnm.tnmRiskLOD1)
                                {
                                    for (const auto& member : lod1.wtrSurfaceList)
                                    {
                                        // ���O����p
                                        vector<CPoint2D> area2DList{};
                                        for (auto& pos : member.posList)
                                        {
                                            CPoint2D p2D = CPoint2D(pos.x, pos.y);
                                            area2DList.emplace_back(p2D);
                                        }
                                        CPoint2D* ptArea2D = &area2DList[0];
                                        int ptSize = (int)area2DList.size();

                                        // LOD1���Ɍ���LOD2�����邩
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
                                    if (isMaxDepth) break; // �ő�[�̏ꍇ�͎��̃f�[�^������
                                }
                                if (isMaxDepth) break;
                            }

                            // �Ôg�Z���[(�ő�l)��ݒ�
                            bldg.dTsunamiHeight = dMaxDepth;

                            if (dMaxDepth != 0.0) break;
                        }
                    }

                    // �y���ЊQ���
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
                                        // ���O����p
                                        vector<CPoint2D> area2DList{};
                                        for (auto& pos : member.posList)
                                        {
                                            CPoint2D p2D = CPoint2D(pos.x, pos.y);
                                            area2DList.emplace_back(p2D);
                                        }
                                        CPoint2D* ptArea2D = &area2DList[0];
                                        int ptSize = (int)area2DList.size();

                                        // LOD1���Ɍ���LOD2�����邩
                                        if (CGeoUtil::IsPointInPolygon(pt2D, ptSize, ptArea2D))
                                        {
                                            inside = true;
                                            break;
                                        }

                                    }
                                    if (inside) break;  // �ő�[�̏ꍇ�͎��̃f�[�^������
                                }
                                if (inside) break;
                            }
                            if (inside) break;
                        }

                        // �^���Z���[(�ő�l)��ݒ�
                        bldg.bLandslideArea = inside;
                    }

                    if (bldg.dFloodDepth != 0.0 && bldg.dTsunamiHeight != 0.0 && bldg.bLandslideArea)
                    {
                        // ���ׂĐݒ�ς�
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

// �W�v���f�[�^�擾
vector<AGTBUILDING> GetBldgAggregateData(wstring xmldata, int meshid, vector<JUDGESUITABLEPLACE> judge)
{
    //xml�I�u�W�F�N�g����
    MSXML2::IXMLDOMDocument2Ptr reader;
    reader.CreateInstance(__uuidof(MSXML2::DOMDocument60), NULL, CLSCTX_INPROC_SERVER);

    //xpath���T�|�[�g����悤�ɐݒ�
    reader->setProperty(CComBSTR(L"SelectionLanguage"), CComVariant(L"XPath"));
    //namespace���T�|�[�g����悤�ɐݒ�
    std::wstring ns = GetSelectionNamespaces();
    reader->setProperty(CComBSTR(L"SelectionNamespaces"), CComVariant(ns.c_str()));

    //���[�h
    VARIANT_BOOL isSuccessful;
    reader->load(CComVariant(xmldata.c_str()), &isSuccessful);

    // �������X�g
    vector<AGTBUILDING> AgtBuildingList{};

    HRESULT hResult;

    // ���������J��Ԃ�
    for (int i = 0; i < judge.size(); i++) {
        // ������񏉊���
        AGTBUILDING buildingInfo{};

        // ����ID������
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

            // ����ID�w��̃m�[�h��I��
            BSTR xp2 = SysAllocString(result);
            reader->selectNodes(xp2, &buildingList);
            if (buildingList == NULL)   continue;

            // �m�[�h�����擾
            hResult = buildingList->get_length(&lCountNode);
            if (0 != lCountNode) {
                break;
            }

        }

        if (version == eCityGMLVersion::End)
        {   // �擾���s
            continue;
        }
                
        for (int j = 0; j < lCountNode; j++) {

            // �m�[�h���X�g�̂����̈�̃m�[�h�̎擾
            MSXML2::IXMLDOMNodePtr pXMLDOMNode = NULL;
            hResult = buildingList->get_item(j, &pXMLDOMNode);
            if (FAILED(hResult))
            {
                assert(!"�m�[�h���X�g�̂����̈�̃m�[�h�̎擾�Ɏ��s");
                continue;
            }

            // ����ID���擾
            buildingInfo.strBuildingId = judge[i].strBuildingId;

            // �e�̐e�Ɉړ��@
            MSXML2::IXMLDOMNodePtr pPearentNode = NULL;
            hResult = pXMLDOMNode->get_parentNode(&pPearentNode);
            if (FAILED(hResult))
            {
                assert(!"�e�m�[�h�^�C�v�̎擾�Ɏ��s");
                continue;
            }
            hResult = pPearentNode->get_parentNode(&pXMLDOMNode);
            if (FAILED(hResult))
            {
                assert(!"�e�m�[�h�^�C�v�̎擾�Ɏ��s");
                continue;
            }
            if (version == eCityGMLVersion::VERSION_2)
            {
                MSXML2::IXMLDOMNodePtr pPearentNode2 = pXMLDOMNode;
                hResult = pPearentNode2->get_parentNode(&pXMLDOMNode);
                if (FAILED(hResult))
                {
                    assert(!"�e�m�[�h�^�C�v�̎擾�Ɏ��s");
                    continue;
                }
            }
            

            // �\�����˗�
            MSXML2::IXMLDOMNodePtr solorRadiation = 0;
            BSTR solor = SysAllocString(XPATH_aggregateData1);
            pXMLDOMNode->selectSingleNode(solor, &solorRadiation);
            if (NULL != solorRadiation)
            {
                // �l���擾
                MSXML2::IXMLDOMNodePtr solorRadiationValue = 0;
                BSTR val = SysAllocString(XPATH_aggregateData2);
                solorRadiation->selectSingleNode(val, &solorRadiationValue);
                if (NULL != solorRadiationValue)
                {
                    // �m�[�h�^�C�v�擾
                    MSXML2::DOMNodeType eMemberNodeType;
                    hResult = solorRadiationValue->get_nodeType(&eMemberNodeType);
                    if (FAILED(hResult))
                    {
                        assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                        continue;
                    }

                    // �G�������g�^�ւ̕ϊ�
                    MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                    hResult = solorRadiationValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                    if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                    {
                        assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                        continue;
                    }

                    // �l�e�L�X�g�擾
                    BSTR valueText;
                    hResult = pXMLDOMMemberElement->get_text(&valueText);
                    if (SUCCEEDED(hResult))
                    {
                        // BSTR��std::string�ϊ�
                        std::string valueStr = ConvertBSTRToMBS(valueText);
                        buildingInfo.dSolorRadiation = stod(valueStr);

                    }
                }
            }

            // �\�����d��
            MSXML2::IXMLDOMNodePtr electricGeneration = 0;
            BSTR electric = SysAllocString(XPATH_aggregateData3);
            pXMLDOMNode->selectSingleNode(electric, &electricGeneration);
            if (NULL != electricGeneration)
            {
                // �l���擾
                MSXML2::IXMLDOMNodePtr electricGenerationValue = 0;
                BSTR val = SysAllocString(XPATH_aggregateData2);
                electricGeneration->selectSingleNode(val, &electricGenerationValue);
                if (NULL != electricGenerationValue)
                {
                    // �m�[�h�^�C�v�擾
                    MSXML2::DOMNodeType eMemberNodeType;
                    hResult = electricGenerationValue->get_nodeType(&eMemberNodeType);
                    if (FAILED(hResult))
                    {
                        assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                        continue;
                    }

                    // �G�������g�^�ւ̕ϊ�
                    MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                    hResult = electricGenerationValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                    if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                    {
                        assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                        continue;
                    }

                    // �l�e�L�X�g�擾
                    BSTR valueText;
                    hResult = pXMLDOMMemberElement->get_text(&valueText);
                    if (SUCCEEDED(hResult))
                    {
                        // BSTR��std::string�ϊ�
                        std::string valueStr = ConvertBSTRToMBS(valueText);
                        buildingInfo.dElectricGeneration = stod(valueStr);

                    }
                }
            }

            // ���Q��������(�Ď�)
            MSXML2::IXMLDOMNodePtr lightPollutionSummer = 0;
            BSTR summer = SysAllocString(XPATH_aggregateData4);
            pXMLDOMNode->selectSingleNode(summer, &lightPollutionSummer);
            if (NULL != lightPollutionSummer)
            {
                // �l���擾
                MSXML2::IXMLDOMNodePtr lightPollutionSummerValue = 0;
                BSTR val = SysAllocString(XPATH_aggregateData2);
                lightPollutionSummer->selectSingleNode(val, &lightPollutionSummerValue);
                if (NULL != lightPollutionSummerValue)
                {
                    // �m�[�h�^�C�v�擾
                    MSXML2::DOMNodeType eMemberNodeType;
                    hResult = lightPollutionSummerValue->get_nodeType(&eMemberNodeType);
                    if (FAILED(hResult))
                    {
                        assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                        continue;
                    }

                    // �G�������g�^�ւ̕ϊ�
                    MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                    hResult = lightPollutionSummerValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                    if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                    {
                        assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                        continue;
                    }

                    // �l�e�L�X�g�擾
                    BSTR valueText;
                    hResult = pXMLDOMMemberElement->get_text(&valueText);
                    if (SUCCEEDED(hResult))
                    {
                        // BSTR��std::string�ϊ�
                        std::string valueStr = ConvertBSTRToMBS(valueText);
                        buildingInfo.dLightPollutionSummer = stod(valueStr);

                    }
                }
            }

            // ���Q��������(�t��)
            MSXML2::IXMLDOMNodePtr lightPollutionSpling = 0;
            BSTR spling = SysAllocString(XPATH_aggregateData5);
            pXMLDOMNode->selectSingleNode(spling, &lightPollutionSpling);
            if (NULL != lightPollutionSpling)
            {
                // �l���擾
                MSXML2::IXMLDOMNodePtr lightPollutionSplingValue = 0;
                BSTR val = SysAllocString(XPATH_aggregateData2);
                lightPollutionSpling->selectSingleNode(val, &lightPollutionSplingValue);
                if (NULL != lightPollutionSplingValue)
                {
                    // �m�[�h�^�C�v�擾
                    MSXML2::DOMNodeType eMemberNodeType;
                    hResult = lightPollutionSplingValue->get_nodeType(&eMemberNodeType);
                    if (FAILED(hResult))
                    {
                        assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                        continue;
                    }

                    // �G�������g�^�ւ̕ϊ�
                    MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                    hResult = lightPollutionSplingValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                    if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                    {
                        assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                        continue;
                    }

                    // �l�e�L�X�g�擾
                    BSTR valueText;
                    hResult = pXMLDOMMemberElement->get_text(&valueText);
                    if (SUCCEEDED(hResult))
                    {
                        // BSTR��std::string�ϊ�
                        std::string valueStr = ConvertBSTRToMBS(valueText);
                        buildingInfo.dLightPollutionSpling = stod(valueStr);

                    }
                }
            }

            // ���Q��������(�~��)
            MSXML2::IXMLDOMNodePtr lightPollutionWinter = 0;
            BSTR winter = SysAllocString(XPATH_aggregateData6);
            pXMLDOMNode->selectSingleNode(winter, &lightPollutionWinter);
            if (NULL != lightPollutionWinter)
            {
                // �l���擾
                MSXML2::IXMLDOMNodePtr lightPollutionWinterValue = 0;
                BSTR val = SysAllocString(XPATH_aggregateData2);
                lightPollutionWinter->selectSingleNode(val, &lightPollutionWinterValue);
                if (NULL != lightPollutionWinterValue)
                {
                    // �m�[�h�^�C�v�擾
                    MSXML2::DOMNodeType eMemberNodeType;
                    hResult = lightPollutionWinterValue->get_nodeType(&eMemberNodeType);
                    if (FAILED(hResult))
                    {
                        assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                        continue;
                    }

                    // �G�������g�^�ւ̕ϊ�
                    MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                    hResult = lightPollutionWinterValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                    if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                    {
                        assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                        continue;
                    }

                    // �l�e�L�X�g�擾
                    BSTR valueText;
                    hResult = pXMLDOMMemberElement->get_text(&valueText);
                    if (SUCCEEDED(hResult))
                    {
                        // BSTR��std::string�ϊ�
                        std::string valueStr = ConvertBSTRToMBS(valueText);
                        buildingInfo.dLightPollutionWinter = stod(valueStr);

                    }
                }
            }

            // ���Q��������(�w���)
            MSXML2::IXMLDOMNodePtr lightPollutionOneday = 0;
            BSTR oneday = SysAllocString(XPATH_aggregateData7);
            pXMLDOMNode->selectSingleNode(oneday, &lightPollutionOneday);
            if (NULL != lightPollutionOneday)
            {
                // �l���擾
                MSXML2::IXMLDOMNodePtr lightPollutionOneDayValue = 0;
                BSTR val = SysAllocString(XPATH_aggregateData2);
                lightPollutionOneday->selectSingleNode(val, &lightPollutionOneDayValue);
                if (NULL != lightPollutionOneDayValue)
                {
                    // �m�[�h�^�C�v�擾
                    MSXML2::DOMNodeType eMemberNodeType;
                    hResult = lightPollutionOneDayValue->get_nodeType(&eMemberNodeType);
                    if (FAILED(hResult))
                    {
                        assert(!"�m�[�h�^�C�v�̎擾�Ɏ��s");
                        continue;
                    }

                    // �G�������g�^�ւ̕ϊ�
                    MSXML2::IXMLDOMElementPtr pXMLDOMMemberElement = NULL;
                    hResult = lightPollutionOneDayValue->QueryInterface(IID_IXMLDOMElement, (void**)&pXMLDOMMemberElement);
                    if (FAILED(hResult) || NULL == pXMLDOMMemberElement)
                    {
                        assert(!"�G�������g�^�ւ̕ϊ��Ɏ��s");
                        continue;
                    }

                    // �l�e�L�X�g�擾
                    BSTR valueText;
                    hResult = pXMLDOMMemberElement->get_text(&valueText);
                    if (SUCCEEDED(hResult))
                    {
                        // BSTR��std::string�ϊ�
                        std::string valueStr = ConvertBSTRToMBS(valueText);
                        buildingInfo.dLightPollutionOneDay = stod(valueStr);

                    }
                }
            }

            // �������X�g�ɒǉ�
            AgtBuildingList.emplace_back(buildingInfo);
        }
    }


    //xml�I�u�W�F�N�g���
    reader.Release();
    //COM�̉��
    CoUninitialize();

    return AgtBuildingList;

}

// �W�v�f�[�^�쐬
vector<string> GetAggregateData(string folderPath, string csvfile){

    // CSV�t�@�C���ǂݍ���
    vector<vector<string> > data = csv2vector(csvfile, 1);

    // �W�v����
    vector<string> result{};
    // �w�b�_�[�ݒ�
    result.emplace_back(outputHeader);

    // �G���A���ƂɏW�v
    for (const auto& area : allAreaList)
    {
        std::string areaDirName = CStringEx::Format("%s_%s", area.areaID.c_str(), area.areaName.c_str());
        if (area.areaName == "")	areaDirName.pop_back();	// ���̂������ꍇ�͖�����"_"���폜
        std::filesystem::path areadir = std::filesystem::path(folderPath) / areaDirName;
        if (!std::filesystem::exists(areadir))  continue;

        std::string areapath = areadir.string();

        // �K�n���茋�ʔz��
        vector<JUDGELIST> judgeList{};
        // �K�n���茋�ʔz��
        vector<JUDGESUITABLEPLACE> judgeRsultList{};
        // �K�n����
        JUDGELIST tmpJudgeList{};

        // �f�[�^�ݒ�
        int tmpMeshID = 0;
        for (int i = 0; i < data.size(); i++) {
            if (data[i][0] != area.areaID)  continue;

            if (i == 0) {
                // ����͒l�ǉ�
                tmpJudgeList = {};
                tmpMeshID = stoi(data[i][1]);
                vector<JUDGESUITABLEPLACE> judgeRsultList{};
            }
            else if (tmpMeshID != stoi(data[i][1])) {
                // ���b�V��ID���ύX���ꂽ�烊�X�g�ǉ�
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
        // �Ō�̃f�[�^��ݒ�
        if (judgeRsultList.size() > 0) {
            tmpJudgeList.meshID = tmpMeshID;
            tmpJudgeList.judgeSuitablePlaceList = judgeRsultList;
            judgeList.emplace_back(tmpJudgeList);

        }

        // �f�[�^�擾
        std::vector<std::string> filePath;
        vector<AGTBLDGLIST> allBuiBldgAggregateList{};

        if (getFileNames(areapath, extension_gml, filePath) == true) {

            // ���b�V��ID���ƂɏW�v���f�[�^�쐬
            for (int i = 0; i < judgeList.size(); i++) {
                // �t�@�C�����J��Ԃ�
                for (auto& p : filePath) {
                    vector<AGTBUILDING> result{};
                    AGTBLDGLIST wbldgList{};

                    // �t�@�C�������烁�b�V��ID���擾
                    std::string fullpath = p.c_str();
                    int path_i = (int)fullpath.find_last_of("\\") + 1;
                    int ext_i = (int)fullpath.find_last_of(".");
                    std::string filename = fullpath.substr(path_i, (int64_t)ext_i - path_i);
                    int meshId = stoi(filename.substr(0, 8));

                    // ���b�V��ID����v������f�[�^�쐬
                    if (meshId == judgeList[i].meshID) {
                        // ���b�V��ID�Z�b�g
                        wbldgList.meshID = meshId;

                        //  Reader�̍쐬
                        std::wstring oWString = StringToWString(p.c_str());

                        // XML�f�[�^����Œ�XPath�̃f�[�^���擾
                        result = GetBldgAggregateData(oWString, meshId, judgeList[i].judgeSuitablePlaceList);

                        // �f�[�^���Ȃ��ꍇ�͎��̃t�@�C���ֈړ�
                        if (result.empty()) {
                            continue;
                        }
                        wbldgList.buildingList = result;

                        // �f�[�^�ǉ�
                        allBuiBldgAggregateList.emplace_back(wbldgList);

                        // ���̃f�[�^�֕ύX
                        break;
                    }

                }

            }

        }

        // �͈͓�������
        int building = 0;
        // �\�����˗ʑ��v
        double solorRadiationTotal = 0.0;
        // �\�����d�ʑ��v
        double electricGenerationTotal = 0.0;
        // ���Q�𔭐������錚����
        int lightPollutionBuilding = 0;
        // ���Q�������ԑ��v�i�Ď��j
        int lightPollutionSummerTotal = 0;
        // ���Q�������ԑ��v�i�t���j
        int lightPollutionSplingTotal = 0;
        // ���Q�������ԑ��v�i�~���j
        int lightPollutionWinterTotal = 0;
        // ���Q�������ԑ��v�i�w����j
        int lightPollutionOnedayTotal = 0;
        // �͈͓��D��x1������
        int priorityLevel1Count = 0;
        // �͈͓��D��x2������
        int priorityLevel2Count = 0;
        // �͈͓��D��x3������
        int priorityLevel3Count = 0;
        // �͈͓��D��x4������
        int priorityLevel4Count = 0;
        // �͈͓��D��x5������
        int priorityLevel5Count = 0;


        // �W�v
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
                                    lightPollutionOnedayTotal = lightPollutionOnedayTotal + (int)item2.dLightPollutionOneDay;
                                    // ���Q������
                                    if (item2.dLightPollutionSummer + item2.dLightPollutionSpling + item2.dLightPollutionWinter + item2.dLightPollutionOneDay > 0) {
                                        lightPollutionBuilding++;
                                    }
                                    // �D��x
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

                                    // ��v�����甲����
                                    break;
                                }

                            }
                        }
                    }
                }
            }
            // �W�v�l�ݒ�
            std::ostringstream ss;
            ss << area.areaID.c_str() << "," << building << "," << solorRadiationTotal << "," << electricGenerationTotal << ","
                << lightPollutionBuilding << "," << lightPollutionSummerTotal << "," << lightPollutionSplingTotal << ","
                << lightPollutionWinterTotal << "," << lightPollutionOnedayTotal << ","
                << priorityLevel5Count << "," << priorityLevel4Count << ","
                << priorityLevel3Count << "," << priorityLevel2Count << "," << priorityLevel1Count;
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

    // �L�����Z���t�@�C���p�X
    std::filesystem::path path = std::filesystem::path(strOut) / CANCELFILE;
    std::string cancelPath = path.string();

    HAZARDRISK riskData{};
    vector<FLDRISKLIST> fldList{};
    vector<TNMRISKLIST> tnmList{};
    vector<LSLDRISK> lsldList{};

    // �^���Z���z��f�[�^�ǂݍ���
    if (fldrisk)
    {
        std::filesystem::path fldpath = std::filesystem::path(str) / "fld";
        std::wstring flddir = fldpath.wstring();
        if (std::filesystem::exists(flddir))
        {
            fldList = GetFldData(flddir, cancelPath);
        }
    }

    // �Ôg�Z���z��f�[�^�ǂݍ���
    if (tnmrisk)
    {
        std::filesystem::path tnmpath = std::filesystem::path(str) / "tnm";
        std::wstring tnmdir = tnmpath.wstring();
        if (std::filesystem::exists(tnmdir))
        {
            tnmList = GetTnmData(tnmdir, cancelPath);
        }
    }

    // �y���ЊQ�f�[�^�ǂݍ���
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

    // �L�����Z���t�@�C���p�X
    std::filesystem::path path = std::filesystem::path(strOut) / CANCELFILE;
    std::string cancelPath = path.string();

    // �G���A���ƂɌ������擾
    for (auto& area : allAreaList)
    {
        area.buildList.clear();

        std::string areaDirName = CStringEx::Format("%s_%s", area.areaID.c_str(), area.areaName.c_str());
        if (area.areaName == "")	areaDirName.pop_back();	// ���̂������ꍇ�͖�����"_"���폜
        std::filesystem::path areadir = std::filesystem::path(str) / areaDirName;
        if (!std::filesystem::exists(areadir))  continue;

        std::string areapath = areadir.string();

        std::vector<std::string> fileName;

        //�t�@�C�����擾
        if (getFileNames(areapath, extension_gml, fileName) == true) {

            // �t�@�C�����J��Ԃ�
            for (auto& p : fileName) {
                vector<BUILDING> result{};
                BLDGLIST wbldgList{};

                // �t�@�C�������烁�b�V��ID���擾
                std::string fullpath = p.c_str();
                int path_i = (int)fullpath.find_last_of("\\") + 1;
                int ext_i = (int)fullpath.find_last_of(".");
                std::string filename = fullpath.substr(path_i, (int64_t)ext_i - path_i);
                std::string meshId = filename.substr(0, 8);

                // ���b�V��ID�Z�b�g
                wbldgList.meshID = stoi(meshId);

                //  Reader�̍쐬
                std::wstring oWString = StringToWString(p.c_str());

                // XML�f�[�^����Œ�XPath�̃f�[�^���擾
                result = GetBldgAttribute(oWString);

                // �f�[�^���Ȃ��ꍇ�͎��̃t�@�C���ֈړ�
                if (result.empty()) {
                    continue;
                }
                
                // �ЊQ���X�N����^���Z���[�A�Ôg�Z���[�A�y���ЊQ����ݒ�
                SetBldgHazardRisk(meshId, result);

                wbldgList.buildingList = result;

                // �f�[�^�ǉ�
                area.buildList.emplace_back(wbldgList);

                // �L�����Z���t�@�C���`�F�b�N
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

    // �L�����Z���t�@�C���p�X
    std::filesystem::path path = std::filesystem::path(strOut) / CANCELFILE;
    std::string cancelPath = path.string();

    // �ǂݍ��ރt�@�C��
    std::filesystem::path csvpath = std::filesystem::path(str) / landAnalyzeFile;
    std::string csvfile = csvpath.string();

    // CSV�t�@�C���ǂݍ���
    vector<vector<string> > data = csv2vector(csvfile, 1);

    // �G���A���ƂɌ��ʂ��擾
    for (auto& area : allAreaList)
    {
        LANDSURFACE result{};

        // �f�[�^�ݒ�
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

        // ���O����p
        CPoint2D* ptArea2D = &area.pos2dList[0];
        int ptSize = (int)area.pos2dList.size();

        // �ЊQ���X�N����^���Z���[�A�Ôg�Z���[�A�y���ЊQ����ݒ�
        // �^���Z���[
        if (hazardRiskData.fldRisks.size() > 0)
        {
            double dMaxDepth = 0.0;
            for (const auto& fldList : hazardRiskData.fldRisks)
            {
                for (const auto& fld : fldList.vecFldRisk)
                {
                    // ���ʒ��p���W�ɕϊ�
                    double bbMaxX, bbMinX;
                    double bbMaxY, bbMinY;
                    CGeoUtil::LonLatToXY(fld.lowerCorner.lon, fld.lowerCorner.lat, JPZONE, bbMinX, bbMinY);
                    CGeoUtil::LonLatToXY(fld.upperCorner.lon, fld.upperCorner.lat, JPZONE, bbMaxX, bbMaxY);
                    if (!IsAreaInMesh(area.pos2dList, bbMinX, bbMinY, bbMaxX, bbMaxY))    continue;

                    bool isMaxDepth = false;    // �ő�[���ǂ���
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
                            if (isMaxDepth) break;  // �ő�[�̏ꍇ�͎��̃f�[�^������
                        }
                        if (isMaxDepth) break;
                    }
                    if (isMaxDepth) break;
                }
            }

            // �^���Z���[(�ő�l)��ݒ�
            result.dFloodDepth = dMaxDepth;
        }

        // �^���Z���[
        if (hazardRiskData.tnmRisks.size() > 0)
        {
            double dMaxDepth = 0.0;
            for (const auto& tnmList : hazardRiskData.tnmRisks)
            {
                for (const auto& tnm : tnmList.vecTnmRisk)
                {
                    // ���ʒ��p���W�ɕϊ�
                    double bbMaxX, bbMinX;
                    double bbMaxY, bbMinY;
                    CGeoUtil::LonLatToXY(tnm.lowerCorner.lon, tnm.lowerCorner.lat, JPZONE, bbMinX, bbMinY);
                    CGeoUtil::LonLatToXY(tnm.upperCorner.lon, tnm.upperCorner.lat, JPZONE, bbMaxX, bbMaxY);
                    if (!IsAreaInMesh(area.pos2dList, bbMinX, bbMinY, bbMaxX, bbMaxY))    continue;

                    bool isMaxDepth = false;    // �ő�[���ǂ���
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
                            if (isMaxDepth) break;  // �ő�[�̏ꍇ�͎��̃f�[�^������
                        }
                        if (isMaxDepth) break;
                    }
                    if (isMaxDepth) break;
                }
            }

            // �^���Z���[(�ő�l)��ݒ�
            result.dTsunamiHeight = dMaxDepth;
        }

        // �y���ЊQ���
        if (hazardRiskData.lsldRisks.size() > 0)
        {
            double inside = false;
            for (const auto& lsld : hazardRiskData.lsldRisks)
            {
                // ���ʒ��p���W�ɕϊ�
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
                        if (inside) break;  // �ő�[�̏ꍇ�͎��̃f�[�^������
                    }
                    if (inside) break;
                }
                if (inside) break;
            }

            // �^���Z���[(�ő�l)��ݒ�
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

// �W�v����
int __cdecl AggregateAllData(const char* str, const char* strOut) {


    // �p�X�`�F�b�N
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

    // �L�����Z���t�@�C���p�X
    std::filesystem::path path = std::filesystem::path(strOut).parent_path() / CANCELFILE;
    std::string cancelPath = path.string();

    // �Q�ƃp�X�ݒ�
    std::filesystem::path path1 = std::filesystem::path(str) / "citygml" / "bldg";  // ��͌��ʂ�gml�f�[�^
    std::filesystem::path path2 = std::filesystem::path(strOut) / "�K�n����";       // �K�n����
    std::string bldgPath = path1.string();
    std::string csvPath = path2.string();

    // �t�@�C�����擾
    if (getFileNames(csvPath, extension_csv, filePath) == true) {

        if (filePath.size() > 0) {
            for (auto& p : filePath) {

                // �K�n����t�@�C��������Ώ��������s
                std::string strfile = p.c_str();
                std::filesystem::path filePath = strfile;

                // ����
                if (filePath.filename() == judgeFile) {

                    result = GetAggregateData(bldgPath, p.c_str());

                    // csv�o�͂���
                    // �t�@�C�����ɓ��t������t�^
                    string dateStr = getDatetimeStr();
                    std::filesystem::path outPath = std::filesystem::path(strOut) / "�W�v" / outputFile;
                    ofstream ofs(outPath);
                    for (int i = 0; i < result.size(); i++) {
                        ofs << result[i] << endl;
                    }
                }

                // �L�����Z���t�@�C���`�F�b�N
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