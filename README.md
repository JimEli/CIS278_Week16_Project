# Pima CC CIS278 
## Week #16 Database Exercise

Develop a C++ program to connect to a MS Access database ("Student.accdb" – provided on D2L) and use SQL commands to manipulate its records.

The user will be given a menu to choose from the following options:
```text
  1 - Press (S) to select and display all students records
  2 - Press (D) to delete a student by ID#
  3 - Press (U) to update a student's graduation year
  4 - Press (I) to insert a new student
  5 - Press (Q) to quit
```
For user option #1, first display the header and then display every record on a separate line. See example below:
```text
Student ID  First Name Last Name  Major                   Graduation Year
1           Joe        Doe        Computer Science        2018
2           Emma       Smith      Electrical Engineering  2019
3           Juan       Perez      Marketing               2019
```
For user option #2, confirm back to the user whether the deletion was successful or the ID provided was not found in the students records. For user option #3, prompt the user to enter a student ID, validate if the student ID exists in the database. If it does not exist, then display a message to user that the student ID does not exist. If it exists, then prompt the user to enter the new graduation year. The new graduation year should be no later than 4 years from the current year. For example, if the user enters “2035”, you should display a message to the user saying invalid graduation year. Then prompt again to enter a valid year.

For user option #4, prompt the user to enter the first name, last name, major, and graduation year. Note that the student ID will automatically be generated. DO NOT PROMPT FOR STUDENT ID. Graduation year rules are the same as described in user option #3 above.

After every operation completion, you need to re-display the menu until the user selects option #5 to quit the program.

Technical Tips
C++ language does not have a standard library to connect to and interact with a database management system. Such application is domain and vendor specific. Every major database management system in the market (MS Access, SQL Server, Oracle, etc.) has its own libraries and interfaces. Developers rely on such interfaces as well as the documentation supplied by the vendor when developing applications that interact with a database on the back-end.

For this project, the database is MS Access. Over the years, Microsoft has developed various database communication protocols that were provided in libraries and interfaces (i.e. Dynamic Link Libraries). It is highly recommended that you visit and read the contents of this site https://msdn.microsoft.com/en-us/library/cc811599(v=office.12).aspx before you start working on this project. It is also recommended that you use Direct Data Access Object (Direct DAO) for this project.

Notes:
* Requires MS Access 2007 driver, located here: https://www.microsoft.com/en-us/download/details.aspx?id=23734
* Uses the archaic MS ActiveX Data Object (ADO) API.
* Initally wanted to bind DB recordset to a C++ class, but found that difficult, inefficient and lacking good documentation or examples. However, without it the program is much simplified.
* Program load/init time is excessive, I couldn't determine the cause.
* Needs more input validation.
* Compiled with MS Visual Studio 2017 Community (v141).
* Submitted in partial fulfillment of the requirements of PCC CIS-278.
