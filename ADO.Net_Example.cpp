/*************************************************************************
* Title: CIS278 Week16 C++/CLI ADO.Net OleDb Example
* File: main.cpp
* Author: James Eli
* Date: 5/3/2018
*
* Notes:
*
*  (1) student.accdb converted to older mdb format. Open the .accdb file 
*      in Access. On the "File" tab of the ribbon, choose "Save & Publish", 
*      select the type of .mdb file you want to create (Access 2000 or 
*      Access 2002-2003) and click the "Save As" button.
*
*  (2) Place the student.mdb db file inside the project folder.
*
*  (3) Add to project properties>General:
*       Common Langauge Runtime Support: (/clr)
*       .NET Target Framework Version: v4.5
*
*  (4) In the Solution Explorer>References Add:
*       System
*       System.Data
*      Or, add to project properties>commandline:
*       /FU "C:\Program Files (x86)\Reference Assemblies\Microsoft\Framework\
*       .NETFramework\v4.5\System.dll" 
*       /FU "C:\Program Files (x86)\Reference Assemblies\Microsoft\Framework\
*       .NETFramework\v4.5\System.Data.dll"
*
*  (5) Compiled with MS Visual Studio 2017 Community (v141).
*
*  (6) Additional information can be found here:
*      https://www.codeproject.com/Articles/8477/Using-ADO-NET-for-beginners
*      https://www.codeproject.com/Articles/9325/Database-Manipulation-with-ADO-NET-for-beginners
*************************************************************************
* Change Log:
*   05/03/2018: Initial release. JME
*************************************************************************/
#include <iostream>

// ADO.NET definitions
using namespace System::Data;
using namespace System::Data::Common;

int main()
{
	// Get the factory object for OleDb provider type.
	DbProviderFactory^ fac = DbProviderFactories::GetFactory("System.Data.OleDb");
	DbConnection^ conn = nullptr;

	try
	{
		// Open db connection.
		conn = fac->CreateConnection();
		conn->ConnectionString = "Provider=Microsoft.Jet.OLEDB.4.0;Data Source=student.mdb";
		conn->Open();
		std::cout << "Connection opened.\n";

		// Count the entries.
		DbCommand^ cmd = fac->CreateCommand();
		cmd->CommandText = "SELECT COUNT(*) FROM student";
		cmd->CommandType = CommandType::Text;
		cmd->Connection = conn;
		int numberOfEntries = (int)cmd->ExecuteScalar();
		std::cout << "Number of entries is " << numberOfEntries << std::endl;

		// Get the entries.
		cmd->CommandText = "SELECT * FROM student ORDER BY id";
		DbDataReader^ reader = cmd->ExecuteReader();
		while (reader->Read())
			System::Console::WriteLine("{0}: {1} {2}", reader->GetInt32(0), reader->GetString(1), reader->GetString(2));
		reader->Close();
	}
	catch (System::Exception^ ex)
	{
		System::Console::WriteLine("Error: {0}", ex->Message);
	}
	finally
	{
		if (conn != nullptr)
			conn->Close();
		std::cout << "Connection closed.\n";
	}

	return 0;
}
