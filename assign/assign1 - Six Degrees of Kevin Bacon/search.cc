#include <iostream>
#include <list>
#include <map>
#include <set>
#include <vector>
#include "imdb.h"

using namespace std;

static const int kWrongArgumentCount = 1;
static const int kDatabaseNotFound = 2;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <actor1> <actor2>" << endl;
        return kWrongArgumentCount;
    }

    imdb db(kIMDBDataDirectory);
    if (!db.good()) {
        cerr << "Data directory not found!  Aborting..." << endl;
        return kDatabaseNotFound;
    }

    string source = string(argv[1]);
    string dest = string(argv[2]);
    list<pair<string, short>> queue = list<pair<string, short>>();
    map<string, pair<film, string>> ancestor = map<string, pair<film, string>>();
    set<string> seenActors = set<string>();
    set<film> seenFilms = set<film>();


    queue.push_back(pair<string, short>(source, 0));
    bool found = false;

    while (!queue.empty()) {
        pair<string, short> frontPair = queue.front();
        string front = frontPair.first;
        short chainLength = frontPair.second;

        if (chainLength >= 6) break;

        queue.pop_front();
        seenActors.insert(front);

        vector<film> films = vector<film>();
        db.getCredits(front, films);

        for (film f: films) {
            if (seenFilms.find(f) != seenFilms.end()) continue;
            seenFilms.insert(f);
            vector<string> cast = vector<string>();
            db.getCast(f, cast);
            for (string p: cast) {
                if (seenActors.find(p) != seenActors.end()) continue;
                seenActors.insert(p);
                ancestor[p] = pair<film, string>(f, front);
                queue.push_back(pair<string, short>(p, chainLength + 1));
                if (p == dest) {
                    found = true;
                    break;
                }
            }
            if (found) break;
        }
        if (found) break;
    }

    if (!found) {
        cout << "No connection found between " << source << " and " << dest << endl;
        return 0;
    }


    list<pair<film, string>> path = list<pair<film, string>>();
    string currentActor = dest;
    while (currentActor != source) {
        path.push_front(pair<film, string>(ancestor[currentActor].first, currentActor));
        currentActor = ancestor[currentActor].second;
    }

    currentActor = source;
    for (pair<film, string> p: path) {
        cout << currentActor << " was in \"" << p.first.title << "\" (" << 1900 + p.first.year
             << ") with " << p.second << endl;
        currentActor = p.second;
    }

    return 0;
}
