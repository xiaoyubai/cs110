#include <iostream>
#include <list>
#include <unordered_map>
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

    string source(argv[1]), dest(argv[2]);
    list<pair<string, short>> queue;
    unordered_map<string, pair<film, string>> ancestor;
    set<string> seenActors;
    set<film> seenFilms;

    queue.push_back({source, 0});
    bool found = false;

    while (!queue.empty()) {
        pair<string, short> frontPair = queue.front();
        const string& front = frontPair.first;
        const short chainLength = frontPair.second;

        if (chainLength >= 6) break;

        queue.pop_front();
        seenActors.insert(front);

        vector<film> films;
        db.getCredits(front, films);

        for (const film& f: films) {
            if (seenFilms.find(f) != seenFilms.end()) continue;
            seenFilms.insert(f);
            vector<string> cast;
            db.getCast(f, cast);
            for (const string& p: cast) {
                if (seenActors.find(p) != seenActors.end()) continue;
                seenActors.insert(p);
                ancestor[p] = {f, front};
                queue.push_back({p, chainLength + 1});
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
        cout << "No connection found between "
             << source << " and " << dest << "." << endl;
        return 0;
    }

    list<pair<film, string>> path;
    string currentActor = dest;
    while (currentActor != source) {
        path.push_front({ancestor[currentActor].first, currentActor});
        currentActor = ancestor[currentActor].second;
    }

    currentActor = source;
    for (const pair<film, string>& p: path) {
        cout << currentActor
             << " was in \"" << p.first.title << "\" ("
             << 1900 + p.first.year
             << ") with " << p.second
             << "." << endl;
        currentActor = p.second;
    }

    return 0;
}
