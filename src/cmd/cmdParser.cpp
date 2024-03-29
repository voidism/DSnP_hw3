/****************************************************************************
  FileName     [ cmdParser.cpp ]
  PackageName  [ cmd ]
  Synopsis     [ Define command parsing member functions for class CmdParser ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <cassert>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include "util.h"
#include "cmdParser.h"
#include <dirent.h>
#include <algorithm>

using namespace std;

//----------------------------------------------------------------------
//    External funcitons
//----------------------------------------------------------------------
void mybeep();


//----------------------------------------------------------------------
//    Member Function for class cmdParser
//----------------------------------------------------------------------
// return false if file cannot be opened
// Please refer to the comments in "DofileCmd::exec", cmdCommon.cpp
bool
CmdParser::openDofile(const string& dof)
{
   // TODO...
   _dofile = new ifstream(dof.c_str());
   if(_dofileStack.size() > 1024){
    for(size_t i = 0; i < 1024; ++i) _dofileStack.pop();
    cerr << "Error: cannot open file " << dof.c_str() << "!!" << endl;
    return false;
 }
   if (!*_dofile){
      delete _dofile;
      if (!_dofileStack.empty()) _dofile=_dofileStack.top();
      return false;
   }
   else{
      _dofileStack.push(_dofile);
      _dofile=_dofileStack.top();
      return true;
   }
}

// Must make sure _dofile != 0
void
CmdParser::closeDofile()
{
   assert(_dofile != 0);
   // TODO...
   _dofile->close();
   _dofile = 0;
   _dofileStack.pop();
   if (!_dofileStack.empty()) _dofile=_dofileStack.top();
}

// Return false if registration fails
bool
CmdParser::regCmd(const string& cmd, unsigned nCmp, CmdExec* e)
{
   // Make sure cmd hasn't been registered and won't cause ambiguity
   string str = cmd;
   unsigned s = str.size();
   if (s < nCmp) return false;
   while (true) {
      if (getCmd(str)) return false;
      if (s == nCmp) break;
      str.resize(--s);
   }

   // Change the first nCmp characters to upper case to facilitate
   //    case-insensitive comparison later.
   // The strings stored in _cmdMap are all upper case
   //
   assert(str.size() == nCmp);  // str is now mandCmd
   string& mandCmd = str;
   for (unsigned i = 0; i < nCmp; ++i)
      mandCmd[i] = toupper(mandCmd[i]);
   string optCmd = cmd.substr(nCmp);
   assert(e != 0);
   e->setOptCmd(optCmd);

   // insert (mandCmd, e) to _cmdMap; return false if insertion fails.
   return (_cmdMap.insert(CmdRegPair(mandCmd, e))).second;
}

// Return false on "quit" or if excetion happens
CmdExecStatus
CmdParser::execOneCmd()
{
   bool newCmd = false;
   if (_dofile != 0)
      newCmd = readCmd(*_dofile);
   else
      newCmd = readCmd(cin);

   // execute the command
   if (newCmd) {
      string option;
      CmdExec* e = parseCmd(option);
      if (e != 0)
         return e->exec(option);
   }
   return CMD_EXEC_NOP;
}

// For each CmdExec* in _cmdMap, call its "help()" to print out the help msg.
// Print an endl at the end.
void
CmdParser::printHelps() const
{
   // TODO...Done
   for (CmdMap::const_iterator i = _cmdMap.begin(); i != _cmdMap.end(); i++)
   {
     i->second->help();
   }
   cout << endl;
}

void
CmdParser::printHistory(int nPrint) const
{
   assert(_tempCmdStored == false);
   if (_history.empty()) {
      cout << "Empty command history!!" << endl;
      return;
   }
   int s = _history.size();
   if ((nPrint < 0) || (nPrint > s))
      nPrint = s;
   for (int i = s - nPrint; i < s; ++i)
      cout << "   " << i << ": " << _history[i] << endl;
}


//
// Parse the command from _history.back();
// Let string str = _history.back();
//
// 1. Read the command string (may contain multiple words) from the leading
//    part of str (i.e. the first word) and retrive the corresponding
//    CmdExec* from _cmdMap
//    ==> If command not found, print to cerr the following message:
//        Illegal command!! "(string cmdName)"
//    ==> return it at the end.
// 2. Call getCmd(cmd) to retrieve command from _cmdMap.
//    "cmd" is the first word of "str".
// 3. Get the command options from the trailing part of str (i.e. second
//    words and beyond) and store them in "option"
//
/* string getsec(string cmd){
  for (unsigned i=0; i < cmd.size();i++){
    if(cmd[i]==' '){
      return cmd.substr(i + 1, cmd.size() - (i + 1));
    }
  }
  string tmp = " ";
  return tmp;
} */
CmdExec*
CmdParser::parseCmd(string& option)
{
   assert(_tempCmdStored == false);
   assert(!_history.empty());
   string str = _history.back();

   // TODO...
   assert(str[0] != 0 && str[0] != ' ');
   string maincmd;
   size_t seccmd = myStrGetTok(str, maincmd);
   CmdExec *e = getCmd(maincmd);
   if (e == NULL){
    cerr << "Error: Illegal option!! (" << maincmd << ")" << endl;
    //e->errorOption(CMD_OPT_ILLEGAL, maincmd);
    return NULL;
    }
    else if (seccmd!=string::npos){
      option = str.substr(seccmd+1);
      //cout << "seccmd is " << option << endl;
      return e;
   }
   /* else{
     option = "-";
     return e;
   } */
   return e;
}

// This function is called by pressing 'Tab'.
// It is to list the partially matched commands.
// "str" is the partial string before current cursor position. It can be 
// a null string, or begin with ' '. The beginning ' ' will be ignored.
//
// Several possibilities after pressing 'Tab'
// (Let $ be the cursor position)
// 1. LIST ALL COMMANDS
//    --- 1.1 ---
//    [Before] Null cmd
//    cmd> $
//    --- 1.2 ---
//    [Before] Cmd with ' ' only
//    cmd>     $
//    [After Tab]
//    ==> List all the commands, each command is printed out by:
//           cout << setw(12) << left << cmd;
//    ==> Print a new line for every 5 commands
//    ==> After printing, re-print the prompt and place the cursor back to
//        original location (including ' ')
//
// 2. LIST ALL PARTIALLY MATCHED COMMANDS
//    --- 2.1 ---
//    [Before] partially matched (multiple matches)
//    cmd> h$                   // partially matched
//    [After Tab]
//    HELp        HIStory       // List all the parially matched commands
//    cmd> h$                   // and then re-print the partial command
//    --- 2.2 ---
//    [Before] partially matched (multiple matches)
//    cmd> h$llo                // partially matched with trailing characters
//    [After Tab]
//    HELp        HIStory       // List all the parially matched commands
//    cmd> h$llo                // and then re-print the partial command
//
// 3. LIST THE SINGLY MATCHED COMMAND
//    ==> In either of the following cases, print out cmd + ' '
//    ==> and reset _tabPressCount to 0
//    --- 3.1 ---
//    [Before] partially matched (single match)
//    cmd> he$
//    [After Tab]
//    cmd> heLp $               // auto completed with a space inserted
//    --- 3.2 ---
//    [Before] partially matched with trailing characters (single match)
//    cmd> he$ahah
//    [After Tab]
//    cmd> heLp $ahaha
//    ==> Automatically complete on the same line
//    ==> The auto-expanded part follow the strings stored in cmd map and
//        cmd->_optCmd. Insert a space after "heLp"
//    --- 3.3 ---
//    [Before] fully matched (cursor right behind cmd)
//    cmd> hElP$sdf
//    [After Tab]
//    cmd> hElP $sdf            // a space character is inserted
//
// 4. NO MATCH IN FITST WORD
//    --- 4.1 ---
//    [Before] No match
//    cmd> hek$
//    [After Tab]
//    ==> Beep and stay in the same location
//
// 5. FIRST WORD ALREADY MATCHED ON FIRST TAB PRESSING
//    --- 5.1 ---
//    [Before] Already matched on first tab pressing
//    cmd> help asd$gh
//    [After] Print out the usage for the already matched command
//    Usage: HELp [(string cmd)]
//    cmd> help asd$gh
//
// 6. FIRST WORD ALREADY MATCHED ON SECOND AND LATER TAB PRESSING
//    ==> Note: command usage has been printed under first tab press
//    ==> Check the word the cursor is at; get the prefix before the cursor
//    ==> So, this is to list the file names under current directory that
//        match the prefix
//    ==> List all the matched file names alphabetically by:
//           cout << setw(16) << left << fileName;
//    ==> Print a new line for every 5 commands
//    ==> After printing, re-print the prompt and place the cursor back to
//        original location
//    --- 6.1 ---
//    [Before] if prefix is empty, print all the file names
//    cmd> help $sdfgh
//    [After]
//    .               ..              Homework_3.docx Homework_3.pdf  Makefile
//    MustExist.txt   MustRemove.txt  bin             dofiles         include
//    lib             mydb            ref             src             testdb
//    cmd> help $sdfgh
//    --- 6.2 ---
//    [Before] with a prefix and with mutiple matched files
//    cmd> help M$Donald
//    [After]
//    Makefile        MustExist.txt   MustRemove.txt
//    cmd> help M$Donald
//    --- 6.3 ---
//    [Before] with a prefix and with mutiple matched files,
//             and these matched files have a common prefix
//    cmd> help Mu$k
//    [After]
//    ==> auto insert the common prefix and make a beep sound
//    ==> DO NOT print the matched files
//    cmd> help Must$k
//    --- 6.4 ---
//    [Before] with a prefix and with a singly matched file
//    cmd> help MustE$aa
//    [After] insert the remaining of the matched file name followed by a ' '
//    cmd> help MustExist.txt $aa
//    --- 6.5 ---
//    [Before] with a prefix and NO matched file
//    cmd> help Ye$kk
//    [After] beep and stay in the same location
//    cmd> help Ye$kk
//
//    [Note] The counting of tab press is reset after "newline" is entered.
//
// 7. FIRST WORD NO MATCH
//    --- 7.1 ---
//    [Before] Cursor NOT on the first word and NOT matched command
//    cmd> he haha$kk
//    [After Tab]
//    ==> Beep and stay in the same location
bool getdir(vector<string> &files){
  DIR *dp;
  string dir = ".";
  struct dirent *dirp;
  if((dp = opendir(dir.c_str())) == NULL){
      cout << "Error opening " << dir << endl;
      return false;
  }
  while((dirp = readdir(dp)) != NULL){
      files.push_back(string(dirp->d_name));
  }
  closedir(dp);
  return true;
}

string longestCommonPrefix(vector<string> &strs,string &pre) {
  string comPrefix ;//= pre;
  if(strs.empty()) return comPrefix;
  for(unsigned int i=0; i<strs[0].size(); i++) {
      for(unsigned int j=1; j<strs.size(); j++) {
          if(i>=strs[j].size() || strs[j][i]!=strs[0][i])
              return comPrefix;
      }
      comPrefix.push_back(strs[0][i]);
  }
  return comPrefix;//.substr(pre.size());
}

void
CmdParser::listCmd(const string& str)
{
   // TODO...
   //_tabPressCount +=1;
   size_t start;
   size_t end = str.size();
   bool nullstr = true;
   
   for ( std::string::const_iterator it=str.begin(); it!=str.end(); ++it){
      if(*it != ' '){
         nullstr = false;
         start = str.find(*it);
         end = start;
         for ( std::string::const_iterator it2=it; it2!=str.end(); ++it2){
          if(*it2 == ' '){
             break;
          }
          else end ++;
        }
         break;
      }
   }
   /* for ( std::string::const_reverse_iterator it=str.rbegin(); it!=str.rend(); ++it){
    if(*it != ' '){
       break;
    }
    else{
      end -=1;
    }
 } */
   if(nullstr){
   // 1. LIST ALL COMMANDS (ALL NULL string)
   int count5 = 0;
   cout << endl;
   for (CmdMap::iterator i = _cmdMap.begin(); i != _cmdMap.end(); i++){
      string cmdentry = (*i).first + (*i).second->getOptCmd();
      cout << setw(12) << left << cmdentry;
      count5+=1;
      if (count5 % 5 == 0) cout << endl;
    }
    
    _tabPressCount=0;
    reprintCmd();
   }
   // case 1 end
   else{
     string cmd = str.substr(start);
     //CmdExec* e = getCmd(cmd);
     //if (e==0){
       vector<string> subspec;
      for (CmdMap::iterator i = _cmdMap.begin(); i != _cmdMap.end(); i++)
      {
        if (cmd.size()<=((*i).first + (*i).second->getOptCmd()).size()){
        if (myStrNCmp(((*i).first + (*i).second->getOptCmd()),cmd,cmd.size())==0)//cmd.substr(0, (*i).first.size()) == (*i).first)
        {
           subspec.push_back((*i).first + (*i).second->getOptCmd());
        }
      }

      }
   // 2. LIST ALL PARTIALLY MATCHED COMMANDS
      if (subspec.size()>1){
        cout << endl;
        int count52=0;
        for (vector<string>::iterator i = subspec.begin(); i != subspec.end(); i++){
          cout << setw(12) << left << *i;
          count52+=1;
          if (count52 % 5 == 0) cout << endl;
      }

      _tabPressCount=0;
      reprintCmd();
      }
   // case 2 end

   // 3. LIST THE SINGLY MATCHED COMMAND
   //    ==> In either of the following cases, print out cmd + ' '
   //    ==> and reset _tabPressCount to 0
   // auto completed with a space inserted
      else if (subspec.size()==1){
          string clipped = subspec[0].substr(cmd.size());
          for ( std::string::const_iterator it=clipped.begin(); it!=clipped.end(); ++it){
            //*_readBufPtr = (char)*it;
            //moveBufPtr(_readBufPtr+1);
            insertChar(*it);
          
         }
         //*_readBufPtr = ' ';
         //moveBufPtr(_readBufPtr+1);
         //if (end == str.size()){
        //}
        insertChar(' ');

        _tabPressCount=0;
        
        //reprintCmd();
     }

   // 5. FIRST WORD ALREADY MATCHED ON FIRST TAB PRESSING

   // 6. FIRST WORD ALREADY MATCHED ON SECOND AND LATER TAB PRESSING
         


   
      else if (subspec.empty()){
        string main = str.substr(start, end-start);
        string second;

        CmdExec* exe = 0;
        if(end<str.size()){
          second = str.substr(end+1);
          exe = getCmd(main);
        } 
        //cout << endl << main << endl << second;
        //if (second=="") mybeep();
        // 4. NO MATCH IN FITST WORD // don't do any thing and beep
        if(exe==0) mybeep();
        else if(exe!=0){
          if (_tabPressCount==1){
            cout << "\n";
            exe->usage(cout);
            reprintCmd();
          }
          if (_tabPressCount>=2){
            vector<string> fnames;
            string common = "";
            if (!getdir(fnames)) cout << "fail\n";
            std::sort(fnames.begin(), fnames.end());
            vector <string> matched;
            for (vector<string>::iterator i = fnames.begin(); i != fnames.end(); i++){
              if (i->compare(0, second.size(), second)==0)
                 matched.push_back(*i);
             }
            common = longestCommonPrefix(matched,second);
            string clipped = "";
            if(second.size()<common.size())
              clipped = common.substr(second.size());
            //else
            //  clipped = "";
            if (clipped!=""&&matched.size()>1){//have words to comp.,and multiple probablity
              mybeep();
              for ( std::string::const_iterator it=clipped.begin(); it!=clipped.end(); ++it){
                //*_readBufPtr = (char)*it;
                //moveBufPtr(_readBufPtr+1);
                insertChar(*it);
             }
            }
            else if (matched.size()==1){//have words to comp.,but one probablity
                string clipped = matched[0].substr(second.size());
                for ( std::string::const_iterator it=clipped.begin(); it!=clipped.end(); ++it){
                  insertChar(*it);
               }
               insertChar(' ');
            }
            else if(matched.size()>1){//no words to comp.,and multiple probablity
              cout << "\n";
              int count53 = 0;
              for (vector<string>::iterator i = matched.begin(); i != matched.end(); i++){
                cout << setw(16) << left << *i;
                count53+=1;
                if (count53 % 5 == 0) cout << endl;
              }
              reprintCmd();
            }
            else {//if (second == ""){//any other conditions
              cout << "\n";
              int count53 = 0;
              for (vector<string>::iterator i = fnames.begin(); i != fnames.end(); i++){
                cout << setw(16) << left << *i;
                count53+=1;
                if (count53 % 5 == 0) cout << endl;
              }
              reprintCmd();
            }
            
          }
        }
        else mybeep();
      }
     //}the end of if e == 0
     /* else if (e){
       //if (_tabPressCount == 0){
        string clipped = e->
        for ( std::string::const_iterator it=clipped.begin(); it!=clipped.end(); ++it){
          *_readBufPtr = (char)*it;
          _readBufPtr ++;
        
       }
       *_readBufPtr = ' ';
       _readBufPtr ++;
       //cout << '\r';
       cout << "fuck\n";
       reprintCmd();
       //}
     } */
    }

   // 7. FIRST WORD NO MATCH
   //Don't do any thing and let it go 
   
}

// cmd is a copy of the original input
//
// return the corresponding CmdExec* if "cmd" matches any command in _cmdMap
// return 0 if not found.
//
// Please note:
// ------------
// 1. The mandatory part of the command string (stored in _cmdMap) must match
// 2. The optional part can be partially omitted.
// 3. All string comparison are "case-insensitive".
//
CmdExec*
CmdParser::getCmd(string cmd)
{
   CmdExec* e = 0;
   // TODO...
   //bool flag = 0;
   for (CmdMap::iterator i = _cmdMap.begin(); i != _cmdMap.end(); i++)
   {
     if (cmd.size()>=(*i).first.size()){
     if (myStrNCmp(((*i).first + (*i).second->getOptCmd()),cmd,(*i).first.size())==0)//cmd.substr(0, (*i).first.size()) == (*i).first)
     {
       //cout << "getCmd:"<<(*i).first << endl;
       //flag = 1;
       e = (*i).second;
       //check option
       //if(cmd.size()>(*i).first.size()){
       //string remind = cmd.substr((*i).first.size() + 1, cmd.size() - ((*i).first.size() + 1));
       //e->setOptCmd(remind);
       //}
     }
   }
  }

   //if (flag==0){
   //  cout << "no such cmd named" << cmd << endl;
   //}

   return e;
}


//----------------------------------------------------------------------
//    Member Function for class CmdExec
//----------------------------------------------------------------------
// Return false if error options found
// "optional" = true if the option is optional XD
// "optional": default = true
//
bool
CmdExec::lexSingleOption
(const string& option, string& token, bool optional) const
{
   size_t n = myStrGetTok(option, token);
   if (!optional) {
      if (token.size() == 0) {
         errorOption(CMD_OPT_MISSING, "");
         return false;
      }
   }
   if (n != string::npos) {
      errorOption(CMD_OPT_EXTRA, option.substr(n));
      return false;
   }
   return true;
}

// if nOpts is specified (!= 0), the number of tokens must be exactly = nOpts
// Otherwise, return false.
//
bool
CmdExec::lexOptions
(const string& option, vector<string>& tokens, size_t nOpts) const
{
   string token;
   size_t n = myStrGetTok(option, token);
   while (token.size()) {
      tokens.push_back(token);
      n = myStrGetTok(option, token, n);
   }
   if (nOpts != 0) {
      if (tokens.size() < nOpts) {
         errorOption(CMD_OPT_MISSING, "");
         return false;
      }
      if (tokens.size() > nOpts) {
         errorOption(CMD_OPT_EXTRA, tokens[nOpts]);
         return false;
      }
   }
   return true;
}

CmdExecStatus
CmdExec::errorOption(CmdOptionError err, const string& opt) const
{
   switch (err) {
      case CMD_OPT_MISSING:
         cerr << "Error: Missing option";
         if (opt.size()) cerr << " after (" << opt << ")";
         cerr << "!!" << endl;
      break;
      case CMD_OPT_EXTRA:
         cerr << "Error: Extra option!! (" << opt << ")" << endl;
      break;
      case CMD_OPT_ILLEGAL:
         cerr << "Error: Illegal option!! (" << opt << ")" << endl;
      break;
      case CMD_OPT_FOPEN_FAIL:
         cerr << "Error: cannot open file \"" << opt << "\"!!" << endl;
      break;
      default:
         cerr << "Error: Unknown option error type!! (" << err << ")" << endl;
      exit(-1);
   }
   return CMD_EXEC_ERROR;
}
