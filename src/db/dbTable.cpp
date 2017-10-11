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
#include <stdlib.h>
#include <typeinfo>
#include <algorithm>
#include <sstream>
#include "dbTable.h"
#include "util.h"
#include <algorithm>

using namespace std;

/*****************************************/
/*          Global Functions             */
/*****************************************/
ostream& operator << (ostream& os, const DBRow& r)
{
   // TODO...Done: to print out a row.
   // - Data are seperated by a space. No trailing space at the end.
   // - Null cells are printed as '.'
   for(unsigned int j=0;j<r.size();j++){
    if(r[j]==INT_MAX){ os << setw(6) <<'.'; }
    else os << setw(6) <<r[j];
}
   return os;
}

ostream& operator << (ostream& os, const DBTable& t)
{
   // TODO...Done: to print out a table
   // - Data are seperated by setw(6) and aligned right.
   // - Null cells are printed as '.'
   for(unsigned int i=0;i<t.nRows();i++){/* 
    for(unsigned int j=0;j<t.nCols();j++){
         if(t._table[i][j]==INT_MAX){ os << setw(6) <<'.'; }
         else os << setw(6) <<t._table[i][j];
     } */
     os << t._table[i] << endl;
 }
   return os;
}

void string_to_ascii(string letter)
{
    for (unsigned int i = 0; i < letter.length(); i++)
    {
        char x = letter.at(i);
        cout << int(x) << "\t";
    }
    cout << endl;
}

/* char dis(ifstream& ifs){
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
} */
/* int dis(ifstream &file)
{
	stringstream buffer;
	buffer << file.rdbuf();
	string str = buffer.str();
	if (str.find("\r\n") != string::npos)return 26;
    if (str.find("\r") != string::npos)return 2;
    if (str.find("\n") != string::npos)return 6;
	return 6;
} */

ifstream& operator >> (ifstream& ifs, DBTable& t)
{
   // TODO: to read in data from csv file and store them in a table
   // - You can assume the input file is with correct csv file format
   // - NO NEED to handle error file format
   
   
   if (ifs.is_open()) {
    string line;
    stringstream buffer;
    std::streampos p = ifs.tellg();
    buffer << ifs.rdbuf();
    ifs.seekg(p);
    string str = buffer.str();
    char dtp = '0';
    //if (dtp == '0')
    //cout << "fuck\n";
    if (str.find("\r\n") != string::npos) dtp = '\n';
    else if (str.find("\r") != string::npos) dtp = '\r';
    else if (str.find("\n") != string::npos) dtp = '\n';
    if (dtp == '0')
        cout << "fuck\n";
    while (getline(ifs, line, dtp))
    {
        //cout << "loop\n";
        stringstream ss(line);
        string item;
        //vector<int> vec;
        DBRow *tmprow = new DBRow();
        while(getline(ss, item, ',')){
            if(item==""){
                tmprow->addData(INT_MAX);
            }
            else if(item=="\r"){
             tmprow->addData(INT_MAX);
         }
            else{
                tmprow->addData(atoi(item.c_str()));
            }
        }
        if((ss.rdbuf()->in_avail()==0)&&(item.empty())){
              tmprow->addData(INT_MAX);
        }
        t.addRow(*tmprow);
        //cout << "add row";
        delete tmprow;
    }
   }
      if(!t) cout << "read fail.\n";

   return ifs;
}

/*****************************************/
/*   Member Functions for class DBRow    */
/*****************************************/
void
DBRow::removeCell(size_t c)
{
   // TODO...Done
   for (size_t i = c, n = _data.size(); i < n-1; ++i)
   {
       _data[i] = _data[i + 1];
   }
   _data.pop_back();

}

/*****************************************/
/*   Member Functions for struct DBSort  */
/*****************************************/
bool
DBSort::operator() (const DBRow& r1, const DBRow& r2) const
{
   // TODO: called as a functional object that compares the data in r1 and r2
   //       based on the order defined in _sortOrder
   for(unsigned int i = 0; i < _sortOrder.size(); i++){
    if(r1[ _sortOrder[i] ] < r2[ _sortOrder[i] ]) return true;
    else if(r1[ _sortOrder[i] ] > r2[ _sortOrder[i] ]) return false;
   }
   return false;
}

/*****************************************/
/*   Member Functions for class DBTable  */
/*****************************************/
void
DBTable::reset()
{
   // TODO...Done
   _table.clear();
}

void
DBTable::addCol(const vector<int>& d)
{
   // TODO...Done: add a column to the right of the table. Data are in 'd'.
   size_t d_size = d.size();
   for (size_t i = 0, n = _table.size(); i < n; ++i)
   {
    if (i<d_size) _table[i].addData(d[i]);
    else _table[i].addData(INT_MAX);
   }
}

void
DBTable::delRow(int c)
{
   // TODO...Done: delete row #c. Note #0 is the first row.
   for (size_t i = c, n = _table.size(); i < n-1; ++i)
   {
       _table[i] = _table[i + 1];
   }
   _table.pop_back();
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
   // TODO...Done: get the max data in column #c
   int max = INT_MIN;
   bool flag = 0;
   for (size_t i = 0, n = _table.size(); i < n; ++i)
   {
       if(_table[i][c]!=INT_MAX&&_table[i][c]>max){
           max = _table[i][c];
           flag = 1;
       }
   }
   if (flag ==0)
       return std::numeric_limits<float>::quiet_NaN();
   return (float)max;
}

float
DBTable::getMin(size_t c) const
{
   // TODO...Done: get the min data in column #c
   int min = INT_MAX;
   bool flag = 0;
   for (size_t i = 0, n = _table.size(); i < n; ++i)
   {
       if(_table[i][c]<min){
           min = _table[i][c];
           flag = 1;
       }
   }
   if (flag ==0)
       return std::numeric_limits<float>::quiet_NaN();
   return (float)min;
}

float 
DBTable::getSum(size_t c) const
{
   // TODO...Done: compute the sum of data in column #c
   float sum = 0.0;
   bool flag = 0;
   for (size_t i = 0, n = _table.size(); i < n; ++i)
   {
       if(_table[i][c]!=INT_MAX){
           sum += (float)_table[i][c];
           flag = 1;
       }
   }
   if (flag ==0)
       return std::numeric_limits<float>::quiet_NaN();
   return sum;
}

bool in(int item,vector<int> bag){
    for(unsigned int i=0;i<bag.size();i++){
        if(item==bag[i]){
            return 1;
        }
    }
    return 0;
}

int
DBTable::getCount(size_t c) const
{
   // TODO...Done: compute the number of distinct data in column #c
   // - Ignore null cells
   vector<int> bag;
   bool flag = 0;
   for (size_t i = 0, n = _table.size(); i < n; ++i)
   {
       if(_table[i][c]!=INT_MAX&&!in(_table[i][c],bag)){
           bag.push_back(_table[i][c]);
           flag = 1;
       }
   }
   if (flag ==0)
       return std::numeric_limits<int>::quiet_NaN();
   return bag.size();
}

float
DBTable::getAve(size_t c) const
{
   // TODO...Done: compute the average of data in column #c
   float sum = 0.0;
   float itm = 0.0;
   bool flag = 0;
   for (size_t i = 0, n = _table.size(); i < n; ++i)
   {
       if(_table[i][c]!=INT_MAX){
           sum += (float)_table[i][c];
           itm += 1.0;
           flag = 1;
       }
   }
   if (flag ==0)
       return std::numeric_limits<float>::quiet_NaN();
   return sum/itm;
   return 0.0;
}

void
DBTable::sort(const struct DBSort& s)
{
   // TODO: sort the data according to the order of columns in 's'
   ::sort(_table.begin(), _table.end(), s);
}

void
DBTable::printCol(size_t c) const
{
   // TODO...Done: to print out a column.
   // - Data are seperated by a space. No trailing space at the end.
   // - Null cells are printed as '.'
   for(unsigned int j=0;j<_table.size();j++){
    if(_table[j][c]==INT_MAX){ cout <<'.'; }
    else cout << _table[j][c];
    if (j!=_table.size()-1)
        cout << ' ';
   }
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

