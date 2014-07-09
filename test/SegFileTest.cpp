
#include "../ShortPathSegment.hpp"
#include <iostream>
#include <stdio.h>


using namespace std;




int main(int argc, char* argv[])
{
    if (argc < 3) {
        cout << "usage " << argv[0] << " dictfilePath textfilePath" << endl;
        return 0;
    }

    ShortPathSegment segMan;
    time_t time_s, time_e;
    time_s = clock();
    segMan.loadDict(argv[1]);
    time_e = clock();
    cout << " success in loadDict in " 
        << (double)(time_e - time_s)/CLOCKS_PER_SEC << "seconds" << endl;
    cout << " insert " << segMan.getSize() << " words" << endl;

    vector<string> outString;
    time_s = clock();
    segMan.matchTextFile(argv[2], outString, "|");
    time_e = clock();
    cout << " success in matchFile in " 
        << (double)(time_e - time_s)/CLOCKS_PER_SEC << "seconds" << endl;

    segMan.OutString2File(outString, "out.txt");
    system("less out.txt");
    return 0;
}
