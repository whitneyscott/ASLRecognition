#include <Windows.h>
#include <WindowsX.h>
#include "stdafx.h"
#include <ios>
#include <iostream>
#include "sqlite3.h"
#include <string>
#include <sstream>
#include "resource1.h"

void SetSQLiteData(HWND hwndDlg, TCHAR *line);
using namespace std;

//IDC_SQLITEDISPLAY
//SetSQLiteData(hwndDlg, sqlArr);
int SQLiteOut(int argc, _TCHAR* argv[])
{
	TCHAR *wszEditText = NULL;
	static TCHAR sqlArr[50000];
	std::ostringstream s;
	s << "";
	std::string sAlerts(s.str());

	
	int rc;
	char *error;

	// Open Database
	cout << "Opening MyDb.db ..." << endl;
	sAlerts += "Opening MyDb.db ...";
	sAlerts += "\r\n";

	sqlite3 *db;
	rc = sqlite3_open("MyDb.db", &db);
	if (rc)
	{
		cerr << "Error opening SQLite3 database: " << sqlite3_errmsg(db) << endl << endl;
		sAlerts += "Error opening SQLite3 database: ";
		sAlerts += "\r\n";
		sAlerts += "\r\n";
		sqlite3_close(db);
		return 1;
	}
	else
	{
		cout << "Opened MyDb.db." << endl << endl;
		sAlerts += "Opened MyDb.db.";
		sAlerts += "\r\n";
		sAlerts += "\r\n";
	}

	// Execute SQL
	cout << "Creating MyTable ..." << endl;
	const char *sqlCreateTable = "CREATE TABLE MyTable (id INTEGER PRIMARY KEY, value STRING);";
	rc = sqlite3_exec(db, sqlCreateTable, NULL, NULL, &error);
	if (rc)
	{
		cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << endl << endl;
		sAlerts += "Error executing SQLite3 statement: ";
		sAlerts += "\r\n";
		sAlerts += "\r\n";
		sqlite3_free(error);
	}
	else
	{
		cout << "Created MyTable." << endl << endl;
		sAlerts += "Created MyTable.";
		sAlerts += "\r\n";
		sAlerts += "\r\n";
	}

	// Execute SQL
	cout << "Inserting a value into MyTable ..." << endl;
	const char *sqlInsert = "INSERT INTO MyTable VALUES(NULL, 'A Value');";
	rc = sqlite3_exec(db, sqlInsert, NULL, NULL, &error);
	if (rc)
	{
		cerr << "Error executing SQLite3 statement: " << sqlite3_errmsg(db) << endl << endl;
		sAlerts += "Error executing SQLite3 statement: ";
		sAlerts += "\r\n";
		sAlerts += "\r\n";
		sqlite3_free(error);
	}
	else
	{
		cout << "Inserted a value into MyTable." << endl << endl;
		sAlerts += "Inserted a value into MyTable.";
		sAlerts += "\r\n";
		sAlerts += "\r\n";
	}

	// Display MyTable
	cout << "Retrieving values in MyTable ..." << endl;
	const char *sqlSelect = "SELECT * FROM MyTable;";
	char **results = NULL;
	int rows, columns;
	sqlite3_get_table(db, sqlSelect, &results, &rows, &columns, &error);
	if (rc)
	{
		cerr << "Error executing SQLite3 query: " << sqlite3_errmsg(db) << endl << endl;
		sAlerts += "Error executing SQLite3 query: ";
		sAlerts += "\r\n";
		sAlerts += "\r\n";
		sqlite3_free(error);
	}
	else
	{
		// Display Table
		for (int rowCtr = 0; rowCtr <= rows; ++rowCtr)
		{
			for (int colCtr = 0; colCtr < columns; ++colCtr)
			{
				// Determine Cell Position
				int cellPosition = (rowCtr * columns) + colCtr;

				// Display Cell Value
				cout.width(12);
				cout.setf(ios::left);
				cout << results[cellPosition] << " ";
			}

			// End Line
			cout << endl;

			// Display Separator For Header
			if (0 == rowCtr)
			{
				for (int colCtr = 0; colCtr < columns; ++colCtr)
				{
					cout.width(12);
					cout.setf(ios::left);
					cout << "~~~~~~~~~~~~ ";
					sAlerts += "~~~~~~~~~~~~ ";
				}
				cout << endl;
			}
		}
	}
	sqlite3_free_table(results);

	// Close Database
	cout << "Closing MyDb.db ..." << endl;
	sqlite3_close(db);
	cout << "Closed MyDb.db" << endl << endl;

	// Wait For User To Close Program
	cout << "Please press any key to exit the program ..." << endl;
	cin.get();
	//string2wchar_t(sqlArr, sAlerts);
	int index = 0;
	while (index < (int)sAlerts.size())
	{
		sqlArr[index] = (wchar_t)sAlerts[index];
		++index;
	}
	sqlArr[index] = 0;
	HWND hwndDlg = GetDlgItem(hwndDlg,IDC_SQLITEDISPLAY);
	SetSQLiteData(hwndDlg, sqlArr);
	return 0;
}