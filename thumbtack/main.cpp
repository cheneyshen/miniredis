//
//  main.cpp
//  thumbtack
//
//  Created by spring  on 2016/11/16.
//  Copyright © 2016年 spring . All rights reserved.
//
#include <map>
#include <set>
#include <list>
#include <cmath>
#include <ctime>
#include <deque>
#include <queue>
#include <stack>
#include <string>
#include <bitset>
#include <cstdio>
#include <limits>
#include <vector>
#include <climits>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <numeric>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <unordered_map>
#include <istream>

using namespace std;
class Solution {
private:
    //common comands sequence stack
    stack<string> stacker1;
    //reverse stack1 for commit sequence
    stack<string> stacker2;
    //master hash for name and all the values
    unordered_map<string, vector<int>> mapper1;
    //slave hash for value and all the names, for numequalto
    unordered_map<int, set<string>> mapper2;
public:
    //print out the command back
    void printCmd(vector<string>& cmd) {
        for(int i=0; i<cmd.size(); i++) {
            cout<<cmd[i]<<" ";
        }
        cout<<endl;
    }
    
    //push set, unset, begin, commit and rollback into stack
    void storeCmd(vector<string>& cmd) {
        string result;
        for(int i=0; i<cmd.size(); i++) {
            result+=cmd[i] + " ";
        }
        stacker1.push(result);
    }
    
    //ececute new command
    int executeCmd(vector<string>& cmd) {
        //string command;
        int leng=(int)cmd.size();
        //transform(cmd[0].begin(), cmd[0].end(), command, ::toupper);
        if(cmd[0]=="SET" && leng==3) {
            printCmd(cmd);
            //in stack
            storeCmd(cmd);
            setValue(cmd[1], stoi(cmd[2]));
        }
        else if(cmd[0]=="GET" && leng==2) {
            printCmd(cmd);
            getValue(cmd[1]);
        }
        else if(cmd[0]=="UNSET" && leng==2) {
            printCmd(cmd);
            //in stack
            storeCmd(cmd);
            unsetValue(cmd[1]);
            return 0;
        }
        else if(cmd[0]=="NUMEQUALTO" && leng==2) {
            printCmd(cmd);
            numEqualTo(stoi(cmd[1]));
            return 0;
        }
        else if(cmd[0]=="END" && leng==1) {
            printCmd(cmd);
            return 1;
        }
        else if(cmd[0]=="BEGIN" && leng==1)
        {
            printCmd(cmd);
            //in stack
            storeCmd(cmd);
        }
        else if(cmd[0]=="COMMIT" && leng==1) {
            printCmd(cmd);
            //in stack
            storeCmd(cmd);
            commitCmd();
            return 0;
        }
        else if(cmd[0]=="ROLLBACK" && leng==1) {
            printCmd(cmd);
            //in stack
            storeCmd(cmd);
            rollback();
            return 0;
        }
        return 0;
    }
    
    //commit transactions
    int commitCmd() {
        string curr;
        string tok;
        char delimiter=' ';
        vector<string> params;
        //pop up this commit command
        curr=stacker1.top();
        stacker1.pop();
        if(stacker1.empty()) {
            //no commands now
            cout<<"> NO TRANSACTION"<<endl;
            return 0;
        }
        while(!stacker1.empty()) {
            if(stacker1.top().substr(0, 6)=="COMMIT" || stacker1.top().substr(0, 8)=="ROLLBACK") {
                //meet the previous commit / rollback
                cout<<"> NO TRANSACTION"<<endl;
                break;
            }
            else {
                //reverse command sequence
                curr=stacker1.top();
                stacker1.pop();
                stacker2.push(curr);
            }
        }
        while(!stacker2.empty()) {
            if(stacker2.top().substr(0, 5)!="BEGIN") {
                //only has unset and set these 2 commands
                curr=stacker2.top();
                stacker2.pop();
                /*
                 if(stacker1.empty()) {
                 cout<<"> NO TRANSACTION"<<endl;
                 break;
                 }
                 */
                stringstream ss(curr); // Turn the cmd into a stream.
                while(getline(ss, tok, delimiter)) {
                    params.push_back(tok);
                }
                if(params[0]=="SET") {
                    setValue(params[1], stoi(params[2]));

                }
                else if(params[0]=="UNSET") {
                    unsetValue(params[1]);
                }
                params.clear();
            }
            else {
                //commit can match multiple begins
                curr=stacker2.top();
                stacker2.pop();
            }

        }
        return 1;
    }
    int rollback() {
        string curr;
        string tok;
        char delimiter=' ';
        vector<string> params;
        //pop up rollback command
        curr=stacker1.top();
        stacker1.pop();
        if(stacker1.empty()) {
            cout<<"> NO TRANSACTION"<<endl;
            return 0;
        }
        while(!stacker1.empty()) {
            if(stacker1.top().substr(0, 6)=="COMMIT" || stacker1.top().substr(0, 8)=="ROLLBACK" ) {
                cout<<"> NO TRANSACTION"<<endl;
                break;
            }
            else if(stacker1.top().substr(0, 5)!="BEGIN") {
                //only has unset and set these 2 commands
                curr=stacker1.top();
                stacker1.pop();
                stringstream ss(curr); // Turn the cmd into a stream.
                while(getline(ss, tok, delimiter)) {
                    params.push_back(tok);
                }
                if(params[0]=="SET") {
                    rollbackSet(params);
                }
                else if(params[0]=="UNSET") {
                    rollbackUnset(params);
                }
                params.clear();
            }
            else {
                //only match the first BEGIN and break out
                curr=stacker1.top();
                stacker1.pop();
                break;
                
            }
        }
        return 0;
    }
    //roll back current SET command
    int rollbackSet(vector<string>& cmd) {
        int leng=(int)cmd.size();
        if(cmd[0]=="SET" && leng==3) {
            unordered_map<string, vector<int>>::iterator it=mapper1.find(cmd[1]);
            if(it!=mapper1.end()) {
                //if it is still open, then do 3 actions
                if((it->second)[0]==1) {
                    //remove current name from latest value list
                    removeName(it->second.back(), cmd[1]);
                    //pop up latest value
                    it->second.pop_back();
                    //first value is flag, if values.size()>1
                    if(it->second.size()>1) {
                        //add name to previous value list
                        setList(it->second.back(), cmd[1]);
                    }
                }
                else {
                    return -1;
                }
            }
            return 0;
        }
        return -1;
    }
    
    //roll back current UNSET command
    int rollbackUnset(vector<string>& cmd) {
        int leng=(int)cmd.size();
        if(cmd[0]=="UNSET" && leng==2) {
            unordered_map<string, vector<int>>::iterator it=mapper1.find(cmd[1]);
            if(it!=mapper1.end()) {
                //after unset it is closed
                if((it->second)[0]==0) {
                    //reopen
                    (it->second)[0]=1;
                    //first value is flag, if values.size()>1
                    if(it->second.size()>1) {
                        //add name to lastest value list
                        setList(it->second.back(), cmd[1]);
                    }
                }
                else {
                    return -1;
                }
            }
            return 0;

        }
        return -1;
    }
    
    //set name lastest value
    int setValue(const string& name, const int& value) {
        unordered_map<string, vector<int>>::iterator it=mapper1.find(name);
        if(it!=mapper1.end()) {
            if((it->second)[0]==1) {
                //remove name from existing value
                removeName((it->second)[1], name);
                //push latest value
                it->second.push_back(value);
                //add name to lastest name list
                setList(value, name);
            }
            else {
                return -1;
            }
        }
        else {
            //create new hash node
            vector<int> values;
            values.push_back(1);    //open flag
            values.push_back(value);
            mapper1.insert({name, values});
            //also create name list have same value
            setList(value, name);
        }
        return 0;
    }
    
    //set name list for current value
    int setList(const int& value, const string& name) {
        unordered_map<int, set<string>>::iterator it=mapper2.find(value);
        if(it!=mapper2.end()) {
            it->second.insert(name);
        }
        else {
            //create new hash node
            set<string> iset;
            iset.insert(name);
            mapper2.insert({value, iset});
        }
        return 0;
    }
    
    //get command
    void getValue(const string& name) {
        unordered_map<string, vector<int>>::iterator it=mapper1.find(name);
        if(it!=mapper1.end()) {
            if(it->second.size()==1) {
                //only has flag, no other values, just output NULL
                cout<<"> "<<"NULL";
            }
            else if((it->second)[0]==1) {
                cout<<"> "<<it->second.back();
            }
            else {
                //already closed
                cout<<"> "<<"NULL";
            }
        }
        else
            cout<<"> "<<"NULL";
        cout<<endl;
    }
    
    //unset command
    int unsetValue(const string& name) {
        unordered_map<string, vector<int>>::iterator it=mapper1.find(name);
        if(it!=mapper1.end()) {
            if((it->second)[0]==1) {
                (it->second)[0]=0;  //closed
                //delete name from latest value
                removeName(it->second.back(), name);
                return 0;
            }
            else
                return 0;
        }
        else
            return -1;
    }
    
    //delete name from the value list
    int removeName(const int& value, const string& name) {
        unordered_map<int, set<string>>::iterator it=mapper2.find(value);
        if(it!=mapper2.end()) {
            it->second.erase(name);
            return 0;
        }
        else {
            return -1;
        }
    }
    
    //list.size() time complexity is constant
    void numEqualTo(const int& value) {
        unordered_map<int, set<string>>::iterator it=mapper2.find(value);
        if(it!=mapper2.end()) {
            cout<<"> "<<(int)(it->second).size();
        }
        else {
            cout<<"> "<<0;
        }
        cout<<endl;
    }
};

int main() {
    /* Enter your code here. Read input from STDIN. Print output to STDOUT */
    vector<string> params;
    int res;
    Solution instance = Solution();
    
    char delimiter = ' ';
    while(1) {
        string cmd;
        string tok;
        getline(cin, cmd);
        //cout<<cmd<<endl;
        stringstream ss(cmd); // Turn the cmd into a stream.
        while(getline(ss, tok, delimiter)) {
            params.push_back(tok);
        }
        //cout<<cmd<<endl;
        /*
         size_t pos = 0;
         string token;
         while ((pos = cmd.find(delimiter)) != string::npos) {
         token = cmd.substr(0, pos);
         params.push_back(token);
         //cout<<token<<endl;
         cmd.erase(0, pos + delimiter.length());
         }
         */
        //for(int i=0; i<params.size(); i++)
        //   cout<<params[i]<<" ";
        res=instance.executeCmd(params);
        if(res==1) {
            break;
        }
        params.clear();
    }
    return 0;
}