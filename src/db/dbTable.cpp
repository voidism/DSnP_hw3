/****************************************************************************
  FileName     [ dbTable.cpp ]
  PackageName  [ db ]
  Synopsis     [ Define database Table member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2015-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iomanip>
#include <string>
#include <cctype>
#include <cassert>
#include <set>
#include <algorithm>
#include "dbTable.h"
#include "util.h"

using namespace std;

/*****************************************/
/*          Global Functions             */
/*****************************************/
ostream& operator << (ostream& os, const DBRow& r)
{
   // TODO: to print out a row.
   // - Data are seperated by a space. No trailing space at the end.
   // - Null cells are printed as '.'
   return os;
}

ostream& operator << (ostream& os, const DBTable& t)
{
   // TODO: to print out a table
   // - Data are seperated by setw(6) and aligned right.
   // - Null cells are printed as '.'
   for(unsigned int i=0;i<t.nRows();i++){
    for(unsigned int j=0;j<t.nCols();j++){
         if(t._table[i][j]==INT_MAX){ cout << setw(6) <<'.'; }
         else os << setw(4) <<t._table[i][j];
     }
     os << endl;
 }
   return os;
}

void cta(string letter)
{
    for (unsigned int i = 0; i < letter.length(); i++)
    {
        char x = letter.at(i);
        cout << int(x) << "\t";
    }
    cout << endl;
}

bool in(int item,vector<int> bag){
    for(unsigned int i=0;i<bag.size();i++){
        if(item==bag[i]){
            return 1;
        }
    }
    return 0;
}

char distinguish_enter_type(const fstream& ifs){
    stringstream ss;
    copy(istreambuf_iterator<char>(ifs),
     istreambuf_iterator<char>(),
     ostreambuf_iterator<char>(ss));
    string record = ss.str();
    if(record.find("\r\n")!=string::npos)
        return '\n';
    if(record.find("\r")!=string::npos)
        return '\r';
    if(record.find("\n")!=string::npos)
        return '\n';
    return '\n';
}

ifstream& operator >> (ifstream& ifs, DBTable& t)
{
   // TODO: to read in data from csv file and store them in a table
   // - You can assume the input file is with correct csv file format
   // - NO NEED to handle error file format
   string line;
   char dtp;
   dtp = distinguish_enter_type(ifs);
   while(getline(ifs, line, dtp)){
       stringstream ss(line);
       string item;
       vector<int> vec;
       while(getline(ss, item, ',')){
           if(item==""||item=="\r"){
               vec.push_back(INT_MAX);
           }
           else{
               vec.push_back(atoi(item.c_str()));
           }
       }
       if((ss.rdbuf()->in_avail()==0)&&(item.empty())){
           vec.push_back(INT_MAX);
       }
       DBRow temp(vec);
       t._table.push_back(temp);
   }
   return ifs;
}

/*****************************************/
/*   Member Functions for class DBRow    */
/*****************************************/
void
DBRow::removeCell(size_t c)
{
   // TODO
}

/*****************************************/
/*   Member Functions for struct DBSort  */
/*****************************************/
bool
DBSort::operator() (const DBRow& r1, const DBRow& r2) const
{
   // TODO: called as a functional object that compares the data in r1 and r2
   //       based on the order defined in _sortOrder
   return false;
}

/*****************************************/
/*   Member Functions for class DBTable  */
/*****************************************/
void
DBTable::reset()
{
   // TODO
}

void
DBTable::addCol(const vector<int>& d)
{
   // TODO: add a column to the right of the table. Data are in 'd'.
}

void
DBTable::delRow(int c)
{
   // TODO: delete row #c. Note #0 is the first row.
}

void
DBTable::delCol(int c)
{
   // delete col #c. Note #0 is the first row.
   for (size_t i = 0, n = _table.size(); i < n; ++i)
      _table[i].removeCell(c);
}

// For the following getXXX() functions...  (except for getCount())
// - Ignore null cells
// - If all the cells in column #c are null, return NAN
// - Return "float" because NAN is a float.
float
DBTable::getMax(size_t c) const
{
   // TODO: get the max data in column #c
   return 0.0;
}

float
DBTable::getMin(size_t c) const
{
   // TODO: get the min data in column #c
   return 0.0;
}

float 
DBTable::getSum(size_t c) const
{
   // TODO: compute the sum of data in column #c
   return 0.0;
}

int
DBTable::getCount(size_t c) const
{
   // TODO: compute the number of distinct data in column #c
   // - Ignore null cells
   return 0;
}

float
DBTable::getAve(size_t c) const
{
   // TODO: compute the average of data in column #c
   return 0.0;
}

void
DBTable::sort(const struct DBSort& s)
{
   // TODO: sort the data according to the order of columns in 's'
}

void
DBTable::printCol(size_t c) const
{
   // TODO: to print out a column.
   // - Data are seperated by a space. No trailing space at the end.
   // - Null cells are printed as '.'
}

void
DBTable::printSummary() const
{
   size_t nr = nRows(), nc = nCols(), nv = 0;
   for (size_t i = 0; i < nr; ++i)
      for (size_t j = 0; j < nc; ++j)
         if (_table[i][j] != INT_MAX) ++nv;
   cout << "(#rows, #cols, #data) = (" << nr << ", " << nc << ", "
        << nv << ")" << endl;
}

