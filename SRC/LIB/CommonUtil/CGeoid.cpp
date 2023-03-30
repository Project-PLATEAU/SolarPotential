#include "pch.h"
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cwchar>
#include <string>

#include "CGeoid.h"
#include "CFileUtil.h"


//--------------------------------------------------------------------
// ビッグエンディアン⇔リトルエンディアンの変換
#define BS_BYTE1(x) (  x        & 0xFF )
#define BS_BYTE2(x) ( (x >>  8) & 0xFF )
#define BS_BYTE3(x) ( (x >> 16) & 0xFF )
#define BS_BYTE4(x) ( (x >> 24) & 0xFF )

#define BS_INT16(x) ( (unsigned short)(BS_BYTE1(x)<<8 | BS_BYTE2(x)) )
#define BS_INT32(x) ( BS_BYTE1(x)<<24 | BS_BYTE2(x)<<16 | BS_BYTE3(x)<<8 | BS_BYTE4(x) )

const int GEOID_NODATA = 1;

//-----------------------------------------------------------------------------
CGeoid::CGeoid
(
	int inGeoidType,
	wchar_t* inGeoidPath,
	double inOrgB,
	double inOrgL
)
{
	GeoidType = inGeoidType;
	wcscpy_s(GeoidPath, wcslen(inGeoidPath) * 2, inGeoidPath);

	OriginB = inOrgB;
	OriginL = inOrgL;

	Data = NULL;
}
//-----------------------------------------------------------------------------
CGeoid::~CGeoid()
{
	if (Data != NULL)
	{
		for (int i = 0; i < Height; i++) delete Data[i];
		delete[]Data;
	}
}
//-----------------------------------------------------------------------------
int CGeoid::Load()
{
	// ジオイドファイル存在チェック
	if (!GetFUtil()->IsExistPath(std::wstring(GeoidPath))) return FILE_NONEXIST;

	int ret;

	switch (GeoidType)
	{
		// 日本のジオイド2000
	case GEOID_GEOID2000:
		ret = LoadGeoid2000();
		break;

		// 世界のジオイド（EGM96）
	case GEOID_EGM96:
		ret = LoadEGM96();
		break;

	default:
		ret = FILE_ERROR;
		break;
	}

	return ret;
}

/*-----------------------------------------------------------------------------
■日本のGEOID2000

準拠楕円体：GRS80 / ITRF94

ジオイド・モデルは、北緯20度から50度、東経120度から150度の範囲について、
緯度間隔1分、経度間隔1.5分の格子位置におけるジオイド高として与えられています。

テキスト（ASCII）形式のジオイド高メッシュ・ファイル‘gsigeome.ver4’は、
１行目がヘッダ行、２行目以降に格子点でのジオイド高がつぎの順番に格納されています。

南端の緯線（北緯20度）から北に向かい北端の緯線（北緯50度）の順で、
各緯線について西端（東経120度）から東端（東経150度）の順でジオイド高の数値がｍ単位で格納されています。

データの書式は、つぎのとおりです。

	（ヘッダ行：2F10.5,2F9.6,2I5,I2,A8）
	glamn,glomn,dgla,dglo,nla,nlo,ikind,vern

	glamn　＝ 20.0　：南端の緯度値（北緯20度）
	glomn　＝120.0　：西端の経度値（東経120度）
	dgla　 ＝.016667：緯度間隔（度単位、１分間隔：但し、丸めてあります）
	dglo　 ＝.025000：経度間隔（度単位、1.5分間隔）
	nla　　＝ 1801　：緯線の個数
	nlo　　＝ 1201　：経線の個数
	ikind　＝ 1　　 ：フォーマット識別子（ここでは特に意味はありません）
	vern　 ＝ ver4.0：ジオイド高メッシュ・ファイルのバージョン

	（データ行：28F9.4）
	等緯線ごとに1201個のデータが繰り返し書式(28F9.4)で並んでいます。
-----------------------------------------------------------------------------*/
int CGeoid::LoadGeoid2000()
{
	int i, j;
	FILE* fp;
	wchar_t buf[1024];

	int format;
	wchar_t ver[32];

	// ジオイドファイルオープン	
	if (_wfopen_s(&fp, GeoidPath, L"r"))
	{
		return FILE_UNOPEN;
	}

	int fullH, fullW;
	double orgMinB, orgMinL;
	double orgDB, orgDL;

	// ヘッダー行
	fgetws(buf, 1000, fp);

	int numTokens = swscanf_s(buf, L"%lf %lf %lf %lf %d %d %d %s",
		&orgMinB, &orgMinL, &orgDB, &orgDL, &fullH, &fullW, &format, ver, 32);

	if (numTokens != 8)
	{
		fclose(fp);
		return FILE_ERROR;
	}

	// 例外値セット
	Exception = 999.0;

	DB = 1.0 / 60;   // 緯度方向：1.0分幅
	DL = 1.5 / 60;   // 経度方向：1.5分幅

	// 読み込み範囲決定
	// ここでは、緯度方向全部（30°）、経度方向：±3°を読み込む
	// = 縦1801 × 横241 × 16[byte] ≒ 7[MB]

	// 緯度方向（固定）
	MinB = 20.0;
	MaxB = 50.0;
	Height = 1801;

	// 経度方向
	double orgMaxL = 150.0;
	Width = 241;

	double tempMinL = OriginL - 3.0;
	double tempMaxL = OriginL + 3.0;

	int startCol;

	if (tempMaxL < orgMinL || orgMaxL < tempMinL)
	{ // 必要なデータが範囲外のとき

		fclose(fp);
		return FILE_ERROR;
	}

	if (tempMinL < orgMinL)
	{ // 必要なデータの西端がデータ範囲の西端より西の場合

		startCol = 0;
		MinL = orgMinL;
		MaxL = MinL + 6.0;
	}
	else if (orgMaxL < tempMaxL)
	{ // 必要なデータの東端がデータ範囲の東端より東の場合

		startCol = 960;
		MaxL = orgMaxL;
		MinL = orgMaxL - 6.0;
	}
	else
	{
		startCol = (int)((tempMinL - orgMinL) / DL);
		MinL = orgMinL + DL * startCol;
		MaxL = orgMinL + DL * (startCol + Width - 1);
	}

	Data = new CGeoidData * [Height];
	for (i = 0; i < Height; i++) Data[i] = new CGeoidData[Width];

	CGeoidData* p;
	double dammy;

	int restCol = 1201 - startCol - Width;

	for (i = 0; i < Height; i++)
	{
		p = Data[i];

		for (j = 0; j < startCol; j++) fwscanf_s(fp, L"%lf", &dammy);

		for (j = 0; j < Width; j++)
		{
			fwscanf_s(fp, L"%lf", &p->Value);

			if (fabs(p->Value - Exception) < 0.00001)
			{
				p->Flag = GEOID_NODATA;
			}
			else
			{
				p->Flag = 0;
			}

			p++;
		}

		for (j = 0; j < restCol; j++) fwscanf_s(fp, L"%lf", &dammy);
	}

	fclose(fp);

	return NOERROR_;
}

/*-----------------------------------------------------------------------------
■世界のジオイド（EGM96）

---- FILE: WW15MGH.DAC

The total size of the file is 2,076,480 bytes. This file was created
using an INTEGER*2 data type format and is an unformatted direct access
file. The data on the file is arranged in records from north to south.
There are 721 records on the file starting with record 1 at 90 N. The
last record on the file is at latitude 90 S. For each record, there
are 1,440 15 arc-minute geoid heights arranged by longitude from west to
east starting at the Prime Meridian (0 E) and ending 15 arc-minutes west
of the Prime Meridian (359.75 E). On file, the geoid heights are in units
of centimeters. While retrieving the Integer*2 values on file, divide by
100 and this will produce a geoid height in meters.
-----------------------------------------------------------------------------*/
int CGeoid::LoadEGM96()
{
	int i, j, k;
	FILE* fp;

	// ジオイドファイルオープン
	if (_wfopen_s(&fp, GeoidPath, L"rb"))
	{
		return FILE_UNOPEN;
	}

	DB = 15.0 / 60.0;   // 緯度方向：15分幅
	DL = 15.0 / 60.0;   // 経度方向：15分幅

	// 緯度方向は全て読み込む
	Height = 721;
	MinB = -90.0;
	MaxB = 90.0;

	// 経度方向は、座標系原点±30°(=241メッシュ）
	Width = 241;

	// EGMは0°-360°の範囲なので、
	// 西経の場合は、一時的に座標系原点に360°を足す
	double tempOriginL = OriginL;

	double orgMinL = 0.0;
	double orgMaxL = 359.75;
	double tempMinL = tempOriginL - 30.0;
	double tempMaxL = tempOriginL + 30.0;

	int startCol;

	if (tempMinL < orgMinL)
	{ // 必要なデータの西端が0°より西（西経の場合）

		tempMinL += 360.0;
		startCol = (int)((tempMinL - orgMinL) / DL);

		if (tempMinL < 180.0)
		{ // 日付変更線をまたいでいるとき、
		  // 0 < MinL < 180, 180 < MaxL < 360 とする

			MinL = orgMinL + DL * startCol;
			MaxL = MinL + 60.0;
			CrossDateLine = true;
		}
		else
		{ // 日付変更線をまたいでいないならば、
		  // -180 < MinL, MaxL とする

			MinL = orgMinL + DL * startCol - 360.0;
			MaxL = MinL + 60.0;
			CrossDateLine = false;
		}
	}
	else
	{ // 必要なデータの西端が0°より東（東経）の場合

		startCol = (int)((tempMinL - orgMinL) / DL);
		MinL = orgMinL + DL * startCol;
		MaxL = MinL + 60.0;

		if (180.0 < tempMaxL)
		{ // 必要なデータの東端が東経180°より東の場合、日付変更線をまたぐ

			CrossDateLine = true;
		}
		else
		{
			CrossDateLine = false;
		}
	}

	Data = new CGeoidData * [Height];
	for (i = 0; i < Height; i++) Data[i] = new CGeoidData[Width];

	int orgWidth = 1440;
	short* dat = new short[orgWidth];
	short val;

	for (i = Height - 1; i >= 0; i--) // 北から
	{
		fread(dat, 2, orgWidth, fp);

		for (j = 0, k = startCol; j < Width; j++, k++)
		{
			// 東端（=360°）までくれば、西端（=0°）にもどる
			if (k == orgWidth) k = 0;

			val = BS_INT16(dat[k]);
			Data[i][j].Value = (double)val / 100.0;
		}
	}

	delete[]dat;

	fclose(fp);

	return NOERROR_;
}
//-----------------------------------------------------------------------------
// ジオイド抽出を行う
int CGeoid::Extract
(
	const double& inB,
	const double& inL,
	double* outHg
)
{
	int ret;

	switch (GeoidType)
	{
	case GEOID_GEOID2000:
		return ExtractByBilinear(inB, inL, outHg);

	case GEOID_EGM96:
		return ExtractForEGM96(inB, inL, outHg);

	default:
		ret = FILE_ERROR;
		break;
	}

	return ret;
}
//-----------------------------------------------------------------------------
// バイリニアによる補間でジオイド高を取得する
int CGeoid::ExtractByBilinear
(
	const double& inB,
	const double& inL,
	double* outHg
)
{
	// 南西のメッシュIndex
	int idxRow;
	int idxCol;
	CGeoidData* z11, * z12, * z21, * z22;

	// 南西のメッシュインデックス取得
	idxCol = (int)(floor((inL - MinL) / DL));

	if (idxCol < 0 || Width <= idxCol)
	{
		return TRANS_ERROR;
	}

	idxRow = (int)(floor((inB - MinB) / DB));

	if (idxRow < 0 || Height <= idxRow)
	{
		return TRANS_ERROR;
	}

	z11 = &Data[idxRow][idxCol];
	z12 = &Data[idxRow][idxCol + 1];
	z21 = &Data[idxRow + 1][idxCol];
	z22 = &Data[idxRow + 1][idxCol + 1];

	//
	//    z21-------z22
	//     |     |   |
	//  1-s|     |   |
	//     |-----p---|
	//    s|     |   |
	//     |     |   |
	//    z11-------z12
	//        t   1-t

	double t = (inL - (MinL + DL * idxCol)) / DL;
	double s = (inB - (MinB + DB * idxRow)) / DB;

	if (!(z11->Flag & GEOID_NODATA) &&
		!(z12->Flag & GEOID_NODATA) &&
		!(z21->Flag & GEOID_NODATA) &&
		!(z22->Flag & GEOID_NODATA))
	{ // 例外値がない場合

		*outHg = s * (t * z22->Value + (1.0 - t) * z21->Value) + (1.0 - s) * (t * z12->Value + (1.0 - t) * z11->Value);
	}
	else
	{ // １つでも例外値を含む場合

		// 正常値の存在する箇所のみ使用して、重み付き平均法で算出する
		// ひとつもデータがなければ、0.0を返す
		return CalcValueByWeightedAveMethod(z11, z12, z21, z22, t, s, outHg);
	}

	return NOERROR_;
}
//-----------------------------------------------------------------------------
// EGM96のジオイド抽出を行う（スプライン補間）
int CGeoid::ExtractForEGM96
(
	const double& inB,
	const double& inL,
	double* outHg
)
{
	// EGMは経度範囲は0°～360°とする
	// 0°：グリニッジ子午線
	double tempL = inL;

	if (CrossDateLine)
	{ // 日付変更線をまたいでいるとき

		if (inL < 0.0)
		{ // 西経のとき、+360°する

			tempL += 360.0;
		}
	}

	int	i, j;
	double s, t;
	double gh[4], gf[4], gx[4], a[4], b[4], c[4];

	int idxRow;
	int idxCol;

	// メッシュインデックス取得
	idxCol = (int)(floor((tempL - MinL) / DL));

	if (idxCol < 0 || Width <= idxCol)
	{
		return TRANS_ERROR;
	}

	idxRow = (int)(floor((inB - MinB) / DB));

	if (idxRow < 0 || Height <= idxRow)
	{
		return TRANS_ERROR;
	}

	/*--------------------------

	+: 求める点

		|- t -|

		----------------
		|    |    |    |
		|    |    |    |
		----------------
		|    |    |    |
		|    |+   |    |  ---
 idxRow	----------------   |
		|    |    |    |   s
		|    |    |    |   |
		----------------  ---
		  idxCol

	--------------------------*/

	if (idxRow == 0 || idxRow == Height - 2 || idxCol == 0 || idxCol == Width - 2)
	{ // 端っこのときは、バイリニア補間で取得

		return ExtractByBilinear(inB, inL, outHg);
	}

	s = (inB - MinB - (idxRow - 1) * DB) / DB;
	t = (tempL - MinL - (idxCol - 1) * DL) / DL;

	gx[0] = 0.0;
	gx[1] = 1.0;
	gx[2] = 2.0;
	gx[3] = 3.0;

	for (i = 0; i < 4; i++)
	{
		for (j = 0; j < 4; j++)
		{ // 経度方向の４点の高さを格納

			gh[j] = Data[idxRow + (i - i)][idxCol + (j - 1)].Value;
		}

		// 経度方向にスプライン補間係数の計算
		nSpline4(gx, gh, a, b, c);
		gf[i] = calSpline4(t, gx, gh, a, b, c);
	}

	// 緯度方向にスプライン補間係数の計算
	nSpline4(gx, gf, a, b, c);
	*outHg = calSpline4(s, gx, gf, a, b, c);

	return NOERROR_;
}
//-----------------------------------------------------------------------------
int CGeoid::CalcValueByWeightedAveMethod
(
	CGeoidData* inZ11,
	CGeoidData* inZ12,
	CGeoidData* inZ21,
	CGeoidData* inZ22,
	double& inPx,
	double& inPy,
	double* outHg
)
{
	CGeoidData* g[2][2];
	g[0][0] = inZ11;
	g[0][1] = inZ12;
	g[1][0] = inZ21;
	g[1][1] = inZ22;

	// 4隅から点pへの距離の二乗を計算する
	double r[2][2];
	r[0][0] = inPx * inPx + inPy * inPy;
	r[0][1] = (1 - inPx) * (1 - inPx) + inPy * inPy;
	r[1][0] = inPx * inPx + (1 - inPy) * (1 - inPy);
	r[1][1] = (1 - inPx) * (1 - inPx) + (1 - inPy) * (1 - inPy);

	double sumWZ = 0.0;
	double sumW = 0.0;
	double w;

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			if (g[i][j]->Flag & GEOID_NODATA) continue;

			w = 1.0 / r[i][j];  // 重みは距離の二乗分の１
			sumWZ += w * g[i][j]->Value;
			sumW += w;
		}
	}

	if (sumW == 0.0)
	{
		*outHg = 0.0;
		return TRANS_ERROR;
	}

	*outHg = sumWZ / sumW;

	return NOERROR_;
}

//-----------------------------------------------------------------------------
void CGeoid::nSpline(double* x, double* y, int N, double* a, double* b, double* c)
{
	int i;
	double* h, * v, * u, * tmp;

	h = (double*)malloc(N * sizeof(double));
	v = (double*)malloc(N * sizeof(double));
	u = (double*)malloc(N * sizeof(double));
	tmp = (double*)malloc((N - 2) * sizeof(double));

	for (i = 0; i < N - 1; i++)
	{
		h[i] = x[i + 1] - x[i];
	}

	for (i = 1; i < N - 1; i++)
	{
		a[i - 1] = 2.0 * (h[i - 1] + h[i]);
		v[i - 1] = 6.0 * ((y[i + 1] - y[i]) / h[i] - (y[i] - y[i - 1]) / h[i - 1]);
	}

	for (i = 1; i < N - 1; i++) b[i - 1] = c[i - 1] = h[i];

	f_tri(a, b, c, tmp, v, N - 2);

	for (i = 1; i < N - 1; i++) u[i] = tmp[i - 1];

	u[0] = u[N - 1] = 0.0;

	for (i = 0; i < N - 1; i++)
	{
		a[i] = (u[i + 1] - u[i]) / (6.0 * h[i]);
		b[i] = 0.5 * u[i];
		c[i] = (y[i + 1] - y[i]) / h[i] - h[i] * (2.0 * u[i] + u[i + 1]) / 6.0;
	}

	free(h);
	free(v);
	free(u);
	free(tmp);

	return;
}

//-----------------------------------------------------------------------------
void CGeoid::nSpline4
(
	double* x, // [i/ ] x[0]=0, x[1]=1, x[2]=2, x[3]=3 
	double* y, // [i/ ] x[i]での値y[i] ここでは標高
	double* a, // [ /o] 求める係数a[4]
	double* b, // [ /o] 求める係数b[4]
	double* c  // [ /o] 求める係数c[4]
)
{
	int i;
	double h[4], v[4], u[4], tmp[2];

	for (i = 0; i < 3; i++)
	{
		h[i] = x[i + 1] - x[i];  // h[0]-h[2]
	}

	for (i = 1; i < 3; i++)
	{
		a[i - 1] = 2.0 * (h[i - 1] + h[i]);                                      // a[0]-a[1]
		v[i - 1] = 6.0 * ((y[i + 1] - y[i]) / h[i] - (y[i] - y[i - 1]) / h[i - 1]);  // v[0]-v[1]
	}

	for (i = 1; i < 3; i++) b[i - 1] = c[i - 1] = h[i]; // b[0]-b[1], c[0]-c[1]

	f_tri2(a, b, c, tmp, v);

	for (i = 1; i < 3; i++) u[i] = tmp[i - 1];  // u[1]-u[2]

	u[0] = u[3] = 0.0;

	for (i = 0; i < 3; i++)
	{
		a[i] = (u[i + 1] - u[i]) / (6.0 * (x[i + 1] - x[i]));
		b[i] = 0.5 * u[i];
		c[i] = (y[i + 1] - y[i]) / (x[i + 1] - x[i]) - (x[i + 1] - x[i]) * (2.0 * u[i] + u[i + 1]) / 6.0;
	}

	return;
}

//-----------------------------------------------------------------------------
/* スプライン補間値の計算　*/
double CGeoid::calSpline(double x1, double* x, double* y, double* a, double* b, double* c, int N)
{
	int i;
	double xx, y1;

	// 区間の決定
	for (i = 0; i < N; i++) {
		if (x1 <= x[i + 1]) break;
	}

	if (i == N - 1) i--;

	xx = x1 - x[i];
	y1 = y[i] + (c[i] + (b[i] + a[i] * xx) * xx) * xx;

	return y1;
}
//-----------------------------------------------------------------------------
/* スプライン補間値の計算　*/
double CGeoid::calSpline4
(
	double x1,  // 基準点からの単位距離[mesh] 
	double* x,  // 単位距離 x[0]=0, x[1]=1, x[2]=2, x[3]=3
	double* y,  // x[i]での値y[i]（ここでは、標高）
	double* a,  // 係数a
	double* b,  // 係数b
	double* c   // 係数c
)
{
	int i;
	double xx, y1;

	// 区間の決定
	for (i = 0; i < 3; i++)
	{
		if (x1 <= x[i + 1]) break;
	}

	if (i == 3) i--;

	xx = x1 - x[i];
	y1 = y[i] + (c[i] + (b[i] + a[i] * xx) * xx) * xx;

	return y1;
}

//-----------------------------------------------------------------------------
/* 連立１次方程式を解く　*/
void CGeoid::f_tri(double* a, double* b, double* c, double* x, double* y, int N)
{
	int i;
	double* d, * z;

	d = (double*)malloc(N * sizeof(double));
	z = (double*)malloc(N * sizeof(double));

	/* LU分解　*/
	d[0] = a[0];
	for (i = 1; i < N; i++) {
		x[i] = b[i] / d[i - 1];
		d[i] = a[i] - x[i] * c[i - 1];
	}

	/* 前進代入　*/
	z[0] = y[0];
	for (i = 1; i < N; i++)	z[i] = y[i] - x[i] * z[i - 1];

	/*  後退代入　*/
	x[N - 1] = z[N - 1] / d[N - 1];
	for (i = N - 2; i >= 0; i--) {
		x[i] = (z[i] - c[i] * x[i + 1]) / d[i];
	}

	free(d);
	free(z);

	return;
}
//-----------------------------------------------------------------------------
/* 連立１次方程式を解く　*/
void CGeoid::f_tri2(double* a, double* b, double* c, double* x, double* y)
{
	double d[2], z[2];

	/* LU分解　*/
	d[0] = a[0];
	x[1] = b[1] / d[0];
	d[1] = a[1] - x[1] * c[0];

	/* 前進代入　*/
	z[0] = y[0];
	z[1] = y[1] - x[1] * z[0];

	/*  後退代入　*/
	x[1] = z[1] / d[1];
	x[0] = (z[0] - c[0] * x[1]) / d[0];

	return;
}
