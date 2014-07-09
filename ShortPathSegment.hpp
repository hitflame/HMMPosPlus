#ifndef  SHORTPATHSEGMENT_H_
#define  SHORTPATHSEGMENT_H_

#include <map>
#include <stdint.h>
#include <fstream>
#include <string>
#include <algorithm>
#include <queue>
#include <list>
#include <vector>

#include "TransCode.hpp"

using namespace CppJieba;
using namespace TransCode;

typedef struct trie_node_s trie_node_t;

#define MAX_UINT32 0xffffffff

typedef map<uint16_t, trie_node_t*> NodeMap;

struct trie_node_s
{
    int infoIndex;
    NodeMap nextMap;
    trie_node_t *search_clue;

    trie_node_s() : infoIndex(-1), search_clue(NULL) {}

};


struct DAG_node_t
{
    int prev;
    uint32_t TF;
    bool visited;
    list<uint8_t> nextPos;

    friend bool operator< (DAG_node_t n1, DAG_node_t n2) {
        return n1.TF > n2.TF;
    }

    DAG_node_t() : prev(-1) , TF(MAX_UINT32), visited(false) {}
};

typedef vector<DAG_node_t> vDAG;

class ShortPathSegment
{

    private :
        uint32_t size;
        vector<uint32_t> weightVect;
        vector<uint8_t> lengthVect;
        map< pair<int, int> , uint32_t > PathWeight;
        trie_node_t *root;
        string delimiter;

    private :
        bool buildClue();
        bool Dijkstra(vDAG &_Dag);
        uint32_t getPathWeight(const int &index,const int &step);
        bool destroyTrie(trie_node_t *node);
        bool decodeOutString(const vDAG &_dag, const Unicode &_unicode, vector<string> &out);
        bool genDAG(const Unicode &_unicode,vDAG &_dag);

    public :
        ShortPathSegment();
        ~ShortPathSegment();
        bool insert(Unicode &_unicode);
        bool loadDict(const char *filePath);
        bool matchTextFile(const char *textFilePath, vector<string> &outString, const char *deli);
        uint32_t getSize() const { return size; };
        bool matchAll(const string &src, vector<string> &outVect);
        bool setDelimiter(const char *deli)  { delimiter = deli; } ;
        void OutString2File(vector<string> &out, const char *filePath);

};

bool ShortPathSegment::destroyTrie(trie_node_t *node)
{
    for (auto next : node->nextMap) {
        if ( next.second != nullptr ) {
            destroyTrie(next.second);
        }
    }
    delete node;
    node = NULL;
    return true;
}

ShortPathSegment::~ShortPathSegment()
{
    destroyTrie(root);
}

inline uint32_t ShortPathSegment::getPathWeight(const int &index,const int &step)
{
    auto iter = PathWeight.find(make_pair(index, step));
    if (iter != PathWeight.end()) {
        return iter->second;
    }
    return 0;
}

bool ShortPathSegment::Dijkstra(vDAG &_Dag)
{
    priority_queue<vector<DAG_node_t>::iterator> _minHeap;
    auto loc = _Dag.begin();
    auto next = loc;
    uint32_t curTF,newTF;
    _minHeap.push(loc);
    loc->visited = true;
    do {
        loc = _minHeap.top();
        _minHeap.pop();
        if (loc >= _Dag.end()) continue;
        if (loc->nextPos.empty()) {
            next = loc + 1;
            if (! next->visited) {
                _minHeap.push(next);
                next->visited = true;
            }
            continue;
        }
        curTF = loc->TF;
        for (auto j : (loc->nextPos)) {
            newTF = curTF + getPathWeight(int(loc-_Dag.begin()), j);
            next = loc + j;
            if (newTF < next->TF) {
                next->TF =newTF;
                next->prev = int(loc - _Dag.begin());
            }
            if ( ! next->visited ) {
                _minHeap.push(next);
                next->visited = true;
            }
        }

    } while ( ! _minHeap.empty() );
    return true;
}



bool ShortPathSegment::genDAG(const Unicode &_unicode,vDAG &_dag)
{
    trie_node_t *node = root;
    uint16_t index;
    int head, tail, pos;
    auto iter = _unicode.begin();
    auto begin = iter;
    for( ; iter != _unicode.end(); iter++ ) {
        index = *iter;
        for ( ; ; ) {
            if ( node == NULL ) {
                break;
            } else if ( node->nextMap.find(index) == node->nextMap.end() ) {
                node = node->search_clue;
            } else {
                node = node->nextMap[index];
                break;
            }
        }
        tail = iter - begin + 1;
        if ( node == NULL ) {
            node = root;
            _dag[tail - 1].nextPos.push_back(1);
        } else	{
            if ( node->infoIndex != -1 ) {
                pos = node->infoIndex;
                head = tail - lengthVect[pos];
                PathWeight.insert(make_pair(make_pair(head, lengthVect[pos]), weightVect[pos]));
                _dag[head].nextPos.push_back(lengthVect[pos]);
            }
        }
    }
    _dag[0].TF = 0;
    for (int i = 0;i < _dag.size(); i++) {
        _dag[i].prev = i -1;
    }

    return true;
}

void ShortPathSegment::OutString2File(vector<string> &out, const char *filePath)
{
    if (out.size() < 1) return;
    ofstream out_fs(filePath);
    if (!out_fs) return;
    for (auto elem : out) {
        out_fs << elem << endl;
    }
    out_fs.close();
}

bool ShortPathSegment::decodeOutString(const vDAG &_dag, const Unicode &_unicode, vector<string> &out)
{
    int i, j = _dag.size()-1;
    auto begin = _unicode.begin();
    string text,  temp;
    while (j > 0) {
        i = _dag[j].prev;
        encode(begin+i, begin+j, temp);
        j = i;
        text = temp + delimiter + text;
    }
    out.push_back( text  );
    return true;
}

ShortPathSegment::ShortPathSegment()
{
    root = new trie_node_t;
    size = 0;
}


bool ShortPathSegment::loadDict(const char *filePath)
{
    ifstream ifs(filePath);
    if ( ! ifs ) {
        return false;
    }
    string line;
    uint32_t w;
    Unicode _unicode;
    while ( !ifs.eof()) {
        ifs >> line >> w;
        decode(line, _unicode);
        weightVect.push_back(w);
        lengthVect.push_back(_unicode.size());
        insert(_unicode);
        ifs.ignore(64, '\n');
    }
    ifs.close();
    if (!buildClue()) {
        return false;
    }
    return true;
}

bool ShortPathSegment::insert(Unicode &_unicode)
{
    trie_node_t *node = root;
    for (auto index : _unicode) {
        if ( node->nextMap.find(index) == node->nextMap.end() ) {
            try {
                node->nextMap[index] = new trie_node_t;
            }
            catch(const bad_alloc& ) {
                return false;
            }
        }
        node = node->nextMap[index];
    }
    node->infoIndex = size++;
    return true;
}


bool ShortPathSegment::matchAll(const string &src, vector<string> &outVect)
{
    trie_node_t *node = root;
    uint16_t index;
    Unicode _unicode;
    decode(src, _unicode);
    auto iter = _unicode.begin();
    auto begin = iter, last = iter;
    auto end = _unicode.end();
    string temp;
    for( ; iter != end; iter++ ) {
        index = *iter;
        for ( ; ; ) {
            if ( node == NULL ) {
                break;
            } else if ( node->nextMap.find(index) == node->nextMap.end() ) {
                node = node->search_clue;
            } else {
                node = node->nextMap[index];
                break;
            }
        }
        if ( node == NULL ) {
            node = root;
        } else	{
            if ( node->infoIndex != -1 ) {
                temp.clear();
                last = iter - lengthVect[node->infoIndex] + 1;
                encode(last, iter+1 , temp);
                outVect.push_back(temp);
            }
        }
    }
    return true;
}

bool ShortPathSegment::buildClue()
{
    queue<trie_node_t*> Queue;
    auto iter = root->nextMap.begin();
    int i, head = 0, tail = 0;
    trie_node_t *clue = root, *out, *child;
    Queue.push(root);
    while ( ! Queue.empty() ) {
        out = Queue.front();
        Queue.pop();
        if (out->nextMap.empty()) {
            continue;
        }
        for (iter = out->nextMap.begin(); iter != out->nextMap.end(); iter++) {
            child = iter->second;
            if (child != NULL) {
                Queue.push(child);
                if (out == root) {
                    child->search_clue = root;
                    continue;
                }
                clue = out->search_clue;
                i = iter->first;
                while (clue) {
                    if (clue->nextMap.find(i) != clue->nextMap.end()) {
                        child->search_clue = clue->nextMap[i];
                        break;
                    }
                    clue = clue->search_clue;
                }
                if (clue == NULL) {
                    child->search_clue = root;
                }
            }
        }
    }
    return true;
}

bool ShortPathSegment::matchTextFile(const char *textFilePath,   vector<string> &outString, const char *deli)
{
    ifstream ifs(textFilePath);
    if ( ! ifs ) {
        return false;
    }
    setDelimiter(deli);
    /*
     * 将一行文本视为路径进行分词
     */
    string line;
    while (getline(ifs, line, '\n')) {
        Unicode _unicode;
        decode(line, _unicode);     // convernt words to Unicode
        vDAG _dag( _unicode.size()+1 );
        genDAG(_unicode, _dag);     // generate words' DAG
        Dijkstra(_dag);             // compute shortest path
        // decode Unicode to segment words and push to vector
        decodeOutString( _dag,  _unicode, outString);
    }
    return true;
}


#endif
