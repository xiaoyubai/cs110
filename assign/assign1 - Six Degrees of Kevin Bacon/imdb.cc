#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <algorithm>
#include <fcntl.h>
#include <unistd.h>
#include "imdb.h"
using namespace std;

const char *const imdb::kActorFileName = "actordata";
const char *const imdb::kMovieFileName = "moviedata";
imdb::imdb(const string& directory) {
    const string actorFileName = directory + "/" + kActorFileName;
    const string movieFileName = directory + "/" + kMovieFileName;
    actorFile = acquireFileMap(actorFileName, actorInfo);
    movieFile = acquireFileMap(movieFileName, movieInfo);
}

bool imdb::good() const {
    return !( (actorInfo.fd == -1) || (movieInfo.fd == -1) );
}

imdb::~imdb() {
    releaseFileMap(actorInfo);
    releaseFileMap(movieInfo);
}

bool imdb::getCredits(const string& player, vector<film>& films) const {
    const int numPlayers = *(int *) actorFile;
    int *firstOffset = (int *) actorFile + 1;
    int *lastOffset = (int *) actorFile + numPlayers;
    const int *playerOffset = lower_bound(firstOffset, lastOffset, player, [&](const int a, const string& b) -> bool {
        return getPlayer(a).compare(b) < 0;
    });
    if (getPlayer(*playerOffset) != player) return false;

    // Skip name
    char *player_ptr = (char *) actorFile + *playerOffset;
    long nameSpaceUsed = (player.length() % 2 == 0) ? player.length() + 2 : player.length() + 1;
    player_ptr += nameSpaceUsed;

    // Skip number of movies
    short numMovies = *(short *) player_ptr;
    player_ptr += ((nameSpaceUsed + 2) % 4 == 0) ? 2 : 4;

    // Store all movies into vector
    for (short i=0; i < numMovies; i++) {
        films.push_back(getFilm(*(int *) player_ptr));
        player_ptr += 4;
    }

    return true;
}

const string imdb::getPlayer(int offset) const {
    return string((char *) actorFile + offset);
}

const film imdb::getFilm(int offset) const {
    char *film_ptr = (char *) movieFile + offset;
    const string filmName = string(film_ptr);
    film_ptr += filmName.length() + 1;
    char year = *film_ptr;
    return film { filmName, year };
}

bool imdb::getCast(const film& movie, vector<string>& players) const {
    const int numFilms = *(int *) movieFile;
    int *firstOffset = (int *) movieFile + 1;
    int *lastOffset = (int *) movieFile + numFilms;
    const int *movieOffset = lower_bound(firstOffset, lastOffset, movie, [&](const int a, const film& b) -> bool {
        return getFilm(a) < b;
    });
    if (!(getFilm(*movieOffset) == movie)) return false;

    // Skip name
    char *movie_ptr = (char *) movieFile + *movieOffset;
    long titleSpaceUsed = movie.title.length() + 1;
    movie_ptr += titleSpaceUsed;

    // Skip year
    long yearSpaceUsed = ((titleSpaceUsed + 1) % 2 == 0) ? 1 : 2;
    movie_ptr += yearSpaceUsed;

    // Skip number of actors
    short numActors = *(short *) movie_ptr;
    movie_ptr += ((titleSpaceUsed + yearSpaceUsed + 2) % 4 == 0) ? 2 : 4;

    // Store all players into vector
    for (short i=0; i<numActors; i++) {
        players.push_back(getPlayer(*(int *) movie_ptr));
        movie_ptr += 4;
    }

    return true;
}

const void *imdb::acquireFileMap(const string& fileName, struct fileInfo& info) {
    struct stat stats;
    stat(fileName.c_str(), &stats);
    info.fileSize = stats.st_size;
    info.fd = open(fileName.c_str(), O_RDONLY);
    return info.fileMap = mmap(0, info.fileSize, PROT_READ, MAP_SHARED, info.fd, 0);
}

void imdb::releaseFileMap(struct fileInfo& info) {
    if (info.fileMap != NULL) munmap((char *) info.fileMap, info.fileSize);
    if (info.fd != -1) close(info.fd);
}
