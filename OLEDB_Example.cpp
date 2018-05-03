// OLE DB per: https://msdn.microsoft.com/en-us/library/ms810892.aspx
#define DBINITCONSTANTS // Initialize OLE constants...
#define INITGUID
#include <windows.h>
#include <stdio.h>
#include <oledb.h>      // OLE DB include files
#include <oledberr.h> 
#include <msdaguid.h>   // ODBC provider include files
#include <msdasql.h>
#include <iostream>

// Macros--number of row identifiers to retrieve
#define NUMROWS_CHUNK 35

// Global task memory allocator
IMalloc *g_pIMalloc = nullptr;

// Get the characteristics of the rowset (the ColumnsInfo interface).
HRESULT myGetColumnsInfo(
	IRowset      *pIRowset,         // [in]
	ULONG        *pnCols,           // [out]
	DBCOLUMNINFO **ppColumnsInfo,   // [out]
	OLECHAR      **ppColumnStrings  // [out]
) {
	IColumnsInfo *pIColumnsInfo;
	HRESULT      hr;

	if (FAILED(pIRowset->QueryInterface(IID_IColumnsInfo, (void**)&pIColumnsInfo)))
	{
		std::cout << "Query rowset interface for IColumnsInfo failed\n";
		return (E_FAIL);
	}
	
	hr = pIColumnsInfo->GetColumnInfo(pnCols, ppColumnsInfo, ppColumnStrings);
	if (FAILED(hr))
	{
		std::cout << "GetColumnInfo failed.\n";
		*pnCols = 0;
	}
	
	pIColumnsInfo->Release();
	
	return (hr);
}

/********************************************************************
* Create binding structures from column information. Binding
* structures will be used to create an accessor that allows row value
* retrieval.
********************************************************************/
void myCreateDBBindings
(
	ULONG        nCols,          // [in]
	DBCOLUMNINFO *pColumnsInfo,  // [in]
	DBBINDING    **ppDBBindings, // [out]
	char         **ppRowValues   // [out]
) {
	ULONG        nCol;
	ULONG        cbRow = 0;
	DBBINDING    *pDBBindings;
	char*        pRowValues;

	pDBBindings = new DBBINDING[nCols];

	for (nCol = 0; nCol < nCols; nCol++)
	{
		pDBBindings[nCol].iOrdinal = nCol + 1;
		pDBBindings[nCol].obValue = cbRow;
		pDBBindings[nCol].obLength = 0;
		pDBBindings[nCol].obStatus = 0;
		pDBBindings[nCol].pTypeInfo = NULL;
		pDBBindings[nCol].pObject = NULL;
		pDBBindings[nCol].pBindExt = NULL;
		pDBBindings[nCol].dwPart = DBPART_VALUE;
		pDBBindings[nCol].dwMemOwner = DBMEMOWNER_CLIENTOWNED;
		pDBBindings[nCol].eParamIO = DBPARAMIO_NOTPARAM;
		pDBBindings[nCol].cbMaxLen = pColumnsInfo[nCol].ulColumnSize;
		pDBBindings[nCol].dwFlags = 0;
		pDBBindings[nCol].wType = pColumnsInfo[nCol].wType;
		pDBBindings[nCol].bPrecision = pColumnsInfo[nCol].bPrecision;
		pDBBindings[nCol].bScale = pColumnsInfo[nCol].bScale;

		cbRow += pDBBindings[nCol].cbMaxLen;
	}

	pRowValues = new char[cbRow];

	*ppDBBindings = pDBBindings;
	*ppRowValues = pRowValues;

	return;
}

// Retrieve data from a rowset.
void myGetData(IRowset *pIRowset) {
	ULONG        nCols;
	DBCOLUMNINFO *pColumnsInfo = nullptr;
	OLECHAR      *pColumnStrings = nullptr;
	ULONG        nCol;
	ULONG        cRowsObtained;          // Number of rows obtained
	ULONG        iRow;                   // Row count
	HROW         rghRows[NUMROWS_CHUNK]; // Row handles
	HROW         *pRows = &rghRows[0];   // Pointer to the row 
										 // handles
	IAccessor    *pIAccessor;            // Pointer to the accessor
	HACCESSOR    hAccessor;              // Accessor handle
	DBBINDSTATUS *pDBBindStatus = nullptr;
	DBBINDING    *pDBBindings = nullptr;
	char         *pRowValues;

	// Get the description of the rowset for use in binding structure creation.
	if (FAILED(myGetColumnsInfo(pIRowset, &nCols, &pColumnsInfo, &pColumnStrings)))
		return;

	// Create the binding structures.
	myCreateDBBindings(nCols, pColumnsInfo, &pDBBindings, &pRowValues);
	pDBBindStatus = new DBBINDSTATUS[nCols];

	// Create the accessor.
	pIRowset->QueryInterface(IID_IAccessor, (void**)&pIAccessor);
	pIAccessor->CreateAccessor(
		DBACCESSOR_ROWDATA, // Accessor will be used to retrieve row data 
		nCols,              // Number of columns being bound
		pDBBindings,        // Structure containing bind info
		0,                  // Not used for row accessors 
		&hAccessor,         // Returned accessor handle
		pDBBindStatus       // Information about binding validity
	);

	// Process all the rows, NUMROWS_CHUNK rows at a time
	while (TRUE)
	{
		pIRowset->GetNextRows(
			0,              // Reserved
			0,              // cRowsToSkip
			NUMROWS_CHUNK,  // cRowsDesired
			&cRowsObtained, // cRowsObtained
			&pRows
		); // Filled in w/row handles.

		// All done; there is no more rows left to get.
		if (cRowsObtained == 0)
			break;

		// Loop over rows obtained, getting data for each.
		for (iRow = 0; iRow < cRowsObtained; iRow++)
		{
			pIRowset->GetData(rghRows[iRow], hAccessor, pRowValues);
			for (nCol = 0; nCol < nCols; nCol++)
			{
				std::wcout << pColumnsInfo[nCol].pwszName;
				std::cout << ": ";
				// Kludge...
				if ((int)pRowValues[pDBBindings[nCol].obValue] < 32 ||
					(int)pRowValues[pDBBindings[nCol].obValue] > 127)
					std::cout << (int)pRowValues[pDBBindings[nCol].obValue];
				else
					std::wcout << (wchar_t *)&pRowValues[pDBBindings[nCol].obValue];
				std::cout << "  ";
			}
			std::cout << std::endl;
		}

		// Release row handles.
		pIRowset->ReleaseRows(cRowsObtained, rghRows, NULL, NULL, NULL);
	}  // End while

	// Release the accessor.
	pIAccessor->ReleaseAccessor(hAccessor, NULL);
	pIAccessor->Release();

	delete[] pDBBindings;
	delete[] pDBBindStatus;

	g_pIMalloc->Free(pColumnsInfo);
	g_pIMalloc->Free(pColumnStrings);

	return;
}

// Execute a command, retrieve a rowset interface pointer.
HRESULT myCommand(IDBInitialize *pIDBInitialize, IRowset **ppIRowset) {
	IDBCreateSession *pIDBCreateSession;
	IDBCreateCommand *pIDBCreateCommand;
	IRowset          *pIRowset;
	ICommandText     *pICommandText;
	LPCTSTR          wSQLString = OLESTR("SELECT * FROM student ORDER by ID");
	//	LPCTSTR        wSQLString = OLESTR("SELECT * FROM student WHERE ID=8");
	LONG             cRowsAffected;
	HRESULT          hr;

	// Get the DB session object.
	if (FAILED(pIDBInitialize->QueryInterface(IID_IDBCreateSession, (void**)&pIDBCreateSession)))
	{
		std::cout << "Session initialization failed.\n";
		return (E_FAIL);
	}
	
	// Create the session, getting an interface for command creation.
	hr = pIDBCreateSession->CreateSession(NULL, IID_IDBCreateCommand, (IUnknown**)&pIDBCreateCommand);
	pIDBCreateSession->Release();
	if (FAILED(hr))
	{
		std::cout << "Create session failed.\n";
		return (hr);
	}
	
	// Create the command object.
	hr = pIDBCreateCommand->CreateCommand(NULL, IID_ICommandText, (IUnknown**)&pICommandText);
	if (FAILED(hr))
	{
		std::cout << "Create command failed.\n";
		return (hr);
	}
	pIDBCreateCommand->Release();
	
	// The command requires the actual text as well as an indicator
	// of its language and dialect.
	pICommandText->SetCommandText(DBGUID_DBSQL, wSQLString);
	
	// Execute the command.
	hr = pICommandText->Execute(NULL, IID_IRowset, NULL, &cRowsAffected, (IUnknown**)&pIRowset);
	if (FAILED(hr))
		std::cout << "Command execution failed.\n";
	
	pICommandText->Release();
	*ppIRowset = pIRowset;
	
	return (hr);
}

// Set initialization properties on a data source.
HRESULT mySetInitProps(IDBInitialize *pIDBInitialize) {
	const ULONG   nProps = 4;
	IDBProperties *pIDBProperties;
	DBPROP        InitProperties[nProps];
	DBPROPSET     rgInitPropSet;
	HRESULT       hr;

	// Initialize common property options.
	for (ULONG i = 0; i < nProps; i++)
	{
		VariantInit(&InitProperties[i].vValue);
		InitProperties[i].dwOptions = DBPROPOPTIONS_REQUIRED;
		InitProperties[i].colid = DB_NULLID;
	}

	// Level of prompting that will be done to complete the connection process
	InitProperties[0].dwPropertyID = DBPROP_INIT_PROMPT;
	InitProperties[0].vValue.vt = VT_I2;
	InitProperties[0].vValue.iVal = DBPROMPT_NOPROMPT;

	// Data source name--see the sample source included with the OLE DB SDK.
	InitProperties[1].dwPropertyID = DBPROP_INIT_DATASOURCE;
	InitProperties[1].vValue.vt = VT_BSTR;
	InitProperties[1].vValue.bstrVal = SysAllocString(OLESTR("StudentDB"));
	//SysAllocString(OLESTR("OLE_DB_NWind_Jet"));

	// User ID
	//InitProperties[2].dwPropertyID = DBPROP_AUTH_USERID;
	//InitProperties[2].vValue.vt = VT_BSTR;
	//InitProperties[2].vValue.bstrVal = SysAllocString(OLESTR(""));

	// Password
	//InitProperties[3].dwPropertyID = DBPROP_AUTH_PASSWORD;
	//InitProperties[3].vValue.vt = VT_BSTR;
	//InitProperties[3].vValue.bstrVal = SysAllocString(OLESTR(""));

	rgInitPropSet.guidPropertySet = DBPROPSET_DBINIT;
	rgInitPropSet.cProperties = nProps;
	rgInitPropSet.rgProperties = InitProperties;

	// Set initialization properties.
	pIDBInitialize->QueryInterface(IID_IDBProperties, (void**)&pIDBProperties);
	hr = pIDBProperties->SetProperties(1, &rgInitPropSet);

	SysFreeString(InitProperties[1].vValue.bstrVal);
	SysFreeString(InitProperties[2].vValue.bstrVal);
	SysFreeString(InitProperties[3].vValue.bstrVal);

	pIDBProperties->Release();

	if (FAILED(hr))
		std::cout << "Set properties failed.\n";

	return (hr);
}

// Initialize the data source.
HRESULT myInitDSO(IDBInitialize** ppIDBInitialize) {
	// Create an instance of the MSDASQL (ODBC) provider.
	CoCreateInstance(CLSID_MSDASQL, NULL, CLSCTX_INPROC_SERVER,	IID_IDBInitialize, (void**)ppIDBInitialize);
	
	if (*ppIDBInitialize == NULL)
		return (E_FAIL);
	
	if (FAILED(mySetInitProps(*ppIDBInitialize)))
		return (E_FAIL);
	
	if (FAILED((*ppIDBInitialize)->Initialize()))
	{
		std::cout << "IDBInitialze->Initialize failed.\n";
		return (E_FAIL);
	}
	
	return (NOERROR);
}

// General OLE DB application main()
int main()
{
	IDBInitialize *pIDBInitialize = nullptr;
	IRowset *pIRowset = nullptr;

	// Init OLE and set up the DLLs
	CoInitialize(NULL);
	
	// Get the task memory allocator.
	if (!FAILED(CoGetMalloc(MEMCTX_TASK, &g_pIMalloc)) &&
		// Connect to the data source.
		!FAILED(myInitDSO(&pIDBInitialize)) &&
		// Get a session, set and execute a command.
		!FAILED(myCommand(pIDBInitialize, &pIRowset)))
			// Retrieve data from rowset.
			myGetData(pIRowset);

	// Clean up and disconnect.
	if (pIRowset != NULL)
		pIRowset->Release();
	
	if (pIDBInitialize != NULL)
	{
		if (FAILED(pIDBInitialize->Uninitialize()))
		{
			// Uninitialize is not required, but it will fail if an 
			// interface has not been released; we can use it for debugging.
			std::cout << "Someone forgot to release something!\n";
		}
		pIDBInitialize->Release();
	}
	
	if (g_pIMalloc != NULL)
		g_pIMalloc->Release();
	
	CoUninitialize();
	
	return (0);
}

/*
// This code doesn't work. Compiles and accesses the db, but the sql query crashes. Can't figure out why.
// This example uses Active Template Library (ATL) by including
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#include <stdio.h>
#include <atlbase.h>
#include <atlstr.h>
#include <atlcoll.h>
#include <atldbcli.h>
#include <atldbsch.h>
#include <iostream>

using namespace ATL;
using namespace std;

// The following example fetches the schema and data.
int main()
{
	// Uses ATL's string conversion macros to convert between character encodings.
	USES_CONVERSION;

	// Open the connection and initialize the data source specified by the passed
	// initialization string.
	// The connection information uses the Microsoft.ACE.OLEDB.12.0 data provider implemented in aceoledb.dll.
	LPCOLESTR lpcOleConnect = L"Provider=Microsoft.ACE.OLEDB.12.0;Data Source=student.accdb;User Id=;Password=;";
	// To initialize the connection to a database using an OLE DB provider, 
	// two ATL classes are needed: CDataSource and CSession.
	CDataSource dbDataSource;
	CSession dbSession;

	HRESULT hr = CoInitialize(NULL);
	hr = dbDataSource.OpenFromInitializationString(lpcOleConnect);
	if (FAILED(hr))
		std::cout << "Unable to connect to data source." << endl;
	else
	{
		hr = dbSession.Open(dbDataSource);
		if (FAILED(hr))
			std::cout << "Couldn't create session on data source." << endl;
		else
		{
			CComVariant var;
			hr = dbDataSource.GetProperty(DBPROPSET_DATASOURCEINFO, DBPROP_DATASOURCENAME, &var);
			if (FAILED(hr) || (var.vt == VT_EMPTY))
				std::cout << "No Data Source Name Specified." << endl;
			else
			{
				std::cout << "Successfully connected to database. Data source name:\n  " << OLE2CA(var.bstrVal) << endl;

				// Prepare SQL query.
				//LPCOLESTR query = L"select * from STUDENT order by ID";
				LPCOLESTR query = L"SELECT * FROM student;";
				std::cout << "SQL query:\n  " << OLE2CA(query) << endl;

				// Excecute the query and create a record set.
				CCommand<CDynamicStringAccessor> cmd;
				// open the dataset
				hr = cmd.Open(dbSession, query);
				DBORDINAL colCount = cmd.GetColumnCount();

				if (SUCCEEDED(hr) && colCount > 0)
				{
					DBORDINAL cColumns;
					DBCOLUMNINFO* rgInfo = NULL;
					OLECHAR* pStringsBuffer = NULL;

					std::cout << "Retrieve schema info for the given result set: " << endl;
					cmd.GetColumnInfo(&cColumns, &rgInfo, &pStringsBuffer);
					for (int col = 0; col < (int)colCount; col++)
						cout << " | " << OLE2CA(rgInfo[col].pwszName);
					cout << endl;

					cout << "Fetch the actual data: " << endl;
					int rowCount = 0;
					CRowset<CDynamicStringAccessor> *pRS = (CRowset<CDynamicStringAccessor>*)&cmd;

					// Loop through the rows in the result set.
					while (pRS->MoveNext() == S_OK)
					{
						for (int col = 1; col <= (int)colCount; col++)
						{
							WCHAR* szValue = cmd.GetString(col);
							cout << " | " << szValue;
						}
						cout << endl;
						rowCount++;
					}
					cout << "Total Row Count: " << rowCount << endl;
				}
				else
					std::cout << "Error: Number of fields in the result set is 0."<<endl;
			}  
		}
	}

	dbDataSource.Close();
	dbSession.Close();

	std::cout << "Cleanup. Done." << endl;
}
*/
