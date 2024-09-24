#include <iostream>
#include <fstream> // 開檔
#include <stdio.h> // strtok
#include <string.h>
using namespace std;

void split(char*);
char tok;
char* token;

int main(int argc, char *argv[]) {
    // cout << "We have " << argc << " arguments" << endl;
    // for (int i = 0; i < argc; ++i) {
    //     cout << "[" << i << "] " << argv[i] << endl;
    // }
    cout << "-------------------------Input file ";
    cout << argv[1] << "-------------------------" << endl;

    tok = *argv[2];
    token = &tok;


    ifstream fp;
    fp.open(argv[1]);

    string str;
    char data[30];
    while(fp >> str){

        if(str == "reverse"){
            cout << str;
            fp >> str;
            cout << " " << str << endl;
            // cout << str.length() << endl;
            for(int i = str.length() - 1 ; i >= 0 ; i--){
                cout << str[i];
            }
            cout << endl;


        }
        else if(str == "split"){
            cout << str;
            fp >> data;
            cout << " " << data << endl;
            char* cmd = data;
            split(cmd);
        }

    }
    cout << "-------------------------End of input file " << argv[1] << "-------------------------" << endl;
    fp.close();

    cout << "*****************************User input*****************************" << endl;

    while(cin >> str){
        if(str == "reverse"){
            cin >> str;
            for(int i = str.length() - 1 ; i >= 0 ; i--){
                cout << str[i];
            }
            cout << endl;


        }
        else if(str == "split"){
            cin >> data;
            char* cmd = data;
            split(cmd);
        }
        else if(str == "exit"){
            break;
        }
    }
    

    return 0;
}

void split(char* cmd){
    char* pch;
    pch = strtok (cmd, token);
    int i = 0;
    while (pch != NULL && *pch != '\n')
    {
        cout << pch << " ";
        pch = strtok (NULL, token);
    }
    cout << endl;
    
}