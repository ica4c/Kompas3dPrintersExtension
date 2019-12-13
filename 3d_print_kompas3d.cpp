#include "stdafx.h"
#include <afxdllx.h>
#include <string>
#include "3d_print_kompas3d.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//-------------------------------------------------------------------------------
// ����������� ��������� ������������ � ������� ������������� DLL
// ---
static AFX_EXTENSION_MODULE Kompas3DExtension = { NULL, NULL };
HINSTANCE g_hInstance = NULL;
ksAPI7::IApplicationPtr newKompasAPI( NULL );               

void OnProcessDetach();                 // ���������� ����������

// ���������� ����������
int oType = o3d_edge;										// ��� �������� ��� ������� ����������


//-------------------------------------------------------------------------------
// ����������� ����� �����
// ������������� � ���������� DLL
// ---
extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
  g_hInstance = hInstance;

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("STEP2_API7_3D.AWX Initializing!\n");

		AfxInitExtensionModule(Kompas3DExtension, hInstance);

		new CDynLinkLibrary(Kompas3DExtension);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("STEP2_API7_3D.AWX Terminating!\n");
    OnProcessDetach();
		AfxTermExtensionModule(Kompas3DExtension);
	}
	return 1;   // ok
}


//-------------------------------------------------------------------------------
// �������� ������ � ������ API
// ---
void GetNewKompasAPI() 
{
  if ( !( ksAPI7::IApplication * )newKompasAPI ) 
  {
    CString filename;
    
    if( ::GetModuleFileName(NULL, filename.GetBuffer(255), 255) ) 
    {
      filename.ReleaseBuffer( 255 );
      CString libname;
      
      libname = LoadStr( IDR_API7 );    // kAPI7.dll
      filename.Replace( filename.Right(filename.GetLength() - (filename.ReverseFind('\\') + 1)), 
        libname );
      
      HINSTANCE hAppAuto = LoadLibrary( filename ); // ������������� kAPI7.dll
      
      if(  hAppAuto ) 
      {
        // ��������� �� ������� ������������ ��������� KompasApplication  
        typedef LPDISPATCH ( WINAPI * FCreateKompasApplication )(); 
        
        FCreateKompasApplication pCreateKompasApplication = 
          (FCreateKompasApplication)GetProcAddress( hAppAuto, "CreateKompasApplication" );	
        
        if ( pCreateKompasApplication )
          newKompasAPI = IDispatchPtr ( pCreateKompasApplication(), false /*AddRef*/); // �������� ��������� Application
        FreeLibrary( hAppAuto );  
      }
    }
  }
}


//-------------------------------------------------------------------------------
// ������ ������������� ��������
// ---
unsigned int WINAPI LIBRARYID()
{
  return IDR_LIBID;
}


//-------------------------------------------------------------------------------
// �������� ������� ��������
// ---
ksAPI7::IKompasDocument3DPtr GetCurrentDocument()
{
	ksAPI7::IKompasDocument3DPtr doc = NULL;
	
	// ���� �� ������� ��������� ���������� IApplication � ���
  if ( !(bool)newKompasAPI )
		// �������� ��������� ���������� 
    GetNewKompasAPI();
	
	// ���� ������� ��������� ���������� IApplication � ���
  if ( newKompasAPI ) 
		// �������� �������� ��������
    doc = newKompasAPI->GetActiveDocument();
	
	return doc;
}


//-------------------------------------------------------------------------------
// �������� ������������ �������
// ---
void OpenInSlic3r( ksAPI7::IKompasDocument3DPtr & doc );
void PrintInSlic3r( ksAPI7::IKompasDocument3DPtr & doc );


//-------------------------------------------------------------------------------
// �������� ������� ����������
// ---
void WINAPI LIBRARYENTRY( unsigned int comm )
{
	// �������� ������� ��������
	ksAPI7::IKompasDocument3DPtr doc( GetCurrentDocument() );
	
	if ( doc )
	{
		switch ( comm )
		{
			case 1: 
				OpenInSlic3r( doc );				
				break; 
		}
	}
	else
		MessageT( (LPTSTR)(LPCTSTR)LoadStr(IDS_NODOC) );
}

//-----------------------------------------------------------------------------
// ������� ����������
// ---
BOOL __stdcall  UserFilterProc( IEntity * e)
{
  if( e && (!oType || e->GetType() == oType) )
    return TRUE;
  else
    return FALSE;
}

LPWSTR convertWStringToLPWSTR(std::wstring base) {
	LPWSTR result = new wchar_t[base.size() + 1];
	std::copy(base.begin(), base.end(), result);
	result[base.size()] = 0;

	return result;
}

std::wstring getTempSTLFileLocation(std::wstring fileName) {
	WCHAR bTmpPath[MAX_PATH] = {};

	if (GetTempPathW(MAX_PATH, bTmpPath)) {
		return bTmpPath + fileName;
	}

	return false;
}

std::wstring replaceExtensionOnFileName(std::wstring fileName, std::wstring extension) {
	std::wstring cSFileName = fileName;
	size_t dotPosition = cSFileName.find('.');

	if (dotPosition == std::string::npos) {
		return false;
	}

	return cSFileName.substr(0, dotPosition) + extension;
}

void OpenInSlic3r( ksAPI7::IKompasDocument3DPtr & doc )
{
	LPWSTR documentName = doc->GetName();
	std::wstring tmpFilePath = getTempSTLFileLocation(replaceExtensionOnFileName(documentName, L".stl"));
	BSTR bTmpFilePath = SysAllocStringLen(tmpFilePath.data(), tmpFilePath.size());

	if (doc->SaveAs(bTmpFilePath) == S_OK) {
		std::wstring slic3rCMD = L"Slic3r-console.exe --gui " + tmpFilePath;
		_wsystem(convertWStringToLPWSTR(slic3rCMD));
	}
	else {
		MessageT((LPTSTR)(LPCTSTR)LoadStr(IDS_FAILED_TO_STORE_TMP_FILE));
	}
}


//-------------------------------------------------------------------------------
// ��������� �� ��������� �������� �������� 3D
// ---
void PrintInSlic3r( ksAPI7::IKompasDocument3DPtr & doc )
{
	MessageT(L"Test function 2");
}

//-------------------------------------------------------------------------------
// ���������� ����������
// ---
void OnProcessDetach()
{
  newKompasAPI = NULL;
}


//------------------------------------------------------------------------------
// ��������� ������ �� �������
// ---
CString LoadStr( int strID ) 
{
	TCHAR temp[_MAX_PATH];
	LoadString( Kompas3DExtension.hModule, strID, temp, _MAX_PATH ); 
	return temp;
}


