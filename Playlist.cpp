#include "Playlist.h"

// =======================
// Song implementation (TODO)
// =======================

Song::Song(int id,
            string title,
            string artist,
            string album,
            int duration,
            int score,
            string url)
    : id(id),
    title(title),
    artist(artist),
    album(album),
    duration(duration),
    score(score),
    url(url),
    play_count(0)
{
}

std::string Song::toString() const {
    std::ostringstream oss;
    oss << "Song[id=" << id 
        << ", title=\"" << title << "\""
        << ", artist=\"" << artist << "\""
        << ", album=\"" << album << "\""
        << ", duration=" << duration
        << ", score=" << score
        << ", url=\"" << url << "\""
        << ", play_count=" << play_count
        << "]";
    return oss.str();
}
// TODO: Student can implement additional methods for Song here

// =======================
// Playlist implementation (TODO)
// =======================
Playlist::Playlist(string name)
    : name(name),
    size(0),
    currentIndex(-1)
#ifdef USE_THREADED_AVL
    , currentIt(songs.endIt()),
    hasCurrent(false)
#endif
{
    // TODO
}

SongKey Playlist::makeKey(Song* s) const {
    return SongKey(s->getTitle(), s->getId());
}

void Playlist::resetPlayback() {
    // TODO
    currentIndex = -1;
#ifdef USE_THREADED_AVL
    hasCurrent = false;
    currentIt = songs.endIt();
#endif
}

int Playlist::getSize() const {
    // TODO
    return size;
}

bool Playlist::empty() const {
    // TODO
    return size == 0;
}

void Playlist::clear() {
    // Delete all Song* objects then clear the tree
#ifdef USE_THREADED_AVL
    for (auto it = songs.beginIt(); !it.isNull(); ++it)
        delete it.value();
#else
    for (int i = 0; i < size; i++) {
        Song* song = getSong(i);
        if (song) delete song;
    }
#endif
    songs.clear();
    size = 0;
    resetPlayback();
}

void Playlist::addSong(Song* s) {
    SongKey key = makeKey(s);
    bool shouldIncrement = false;
#ifdef USE_THREADED_AVL
    if (currentIndex >= 0 && !currentIt.isNull()) {
        if (key < currentIt.key()) {
            shouldIncrement = true;
        }
    }
#else
    if (currentIndex >= 0) {
        Song* currentPlaying = getSong(currentIndex);
        if (currentPlaying != nullptr) {
            SongKey currentKey = makeKey(currentPlaying);
            if (key < currentKey) {
                shouldIncrement = true;
            }
        }
    }
#endif
    songs.insert(key, s);
    if (shouldIncrement) {
        currentIndex++;
    }
    
    size++;
}

void Playlist::removeSong(int index) {
    if (index < 0 || index >= size) return;
    Song* s = getSong(index);
    if (s == nullptr) return;

    SongKey key = makeKey(s);

    // --- Update playback state BEFORE erasing ---
    if (size == 1) {
        // Last song: full reset
        songs.erase(key);
        delete s;
        size--;
        resetPlayback();
        return;
    }

    if (index == currentIndex) {
        // Removing the currently-playing song
#ifdef USE_THREADED_AVL
        if (index == size - 1) {
            // It's the last song: wrap to beginning after erase
            songs.erase(key);
            delete s;
            size--;
            currentIndex = 0;
            currentIt = songs.beginIt();
            hasCurrent = true;
        } else if (currentIt.hasTwoChildren()) {
            songs.erase(key);
            delete s;
            size--;
            hasCurrent = true;
        } else {
            ++currentIt;
            songs.erase(key);
            delete s;
            size--;
            hasCurrent = true;
        }
#else
        if (index == size - 1) {
            songs.erase(key);
            delete s;
            size--;
            currentIndex = 0;
        } else {
            songs.erase(key);
            delete s;
            size--;
            // currentIndex stays (next song is now at this index)
        }
#endif
    } else if (index < currentIndex) {
        songs.erase(key);
        delete s;
        size--;
        currentIndex--;
#ifdef USE_THREADED_AVL
        // Rebuild iterator to guarantee it is not dangling.
        currentIt = songs.beginIt();
        for (int i = 0; i < currentIndex; i++) ++currentIt;
#endif
    } else {
        // Removed song is after the current: no index/iterator change needed.
        songs.erase(key);
        delete s;
        size--;
    }
}

Song* Playlist::getSong(int index) const {
    if (index < 0 || index >= size) return nullptr;

#ifdef USE_THREADED_AVL
    auto it = songs.beginIt();
    for (int i = 0; i < index; i++) ++it;
    return it.value();
#else
    std::list<SongKey> keys = songs.ascendingList();
    auto it = keys.begin();
    for (int i = 0; i < index; i++) {
        it++;
    }
    Song** songPtr = const_cast<AVL<SongKey, Song*>&>(songs).find(*it);
    
    if (songPtr != nullptr) {
        return *songPtr;
    }
    return nullptr;
#endif
}

// =======================
// Playing control (TODO)
// =======================

Song* Playlist::playNext() {
    // TODO
    if (size == 0) return nullptr;
    
    if (currentIndex == -1 || currentIndex == size - 1) {
        currentIndex = 0;
#ifdef USE_THREADED_AVL
        currentIt = songs.beginIt();
        hasCurrent = true;
        return currentIt.value();
#endif
        return getSong(0);
    }

    currentIndex++;
#ifdef USE_THREADED_AVL
    ++currentIt;
    return currentIt.value();
#else
    return getSong(currentIndex);
#endif
}

Song* Playlist::playPrevious() {
    // TODO
    if (size == 0) return nullptr;
    
    if (currentIndex <= 0 || currentIndex == -1) {
        currentIndex = size - 1;
#ifdef USE_THREADED_AVL
        currentIt = songs.rbeginIt();
        hasCurrent = true;
        return currentIt.value();
#endif
        return getSong(currentIndex);
    }
    
    currentIndex--;
#ifdef USE_THREADED_AVL
    --currentIt;
    return currentIt.value();
#else
    return getSong(currentIndex);
#endif
}

// =======================
// Score-related (TODO)
// =======================

int Playlist::getTotalScore() {
    // TODO
    int total = 0;
    #ifdef USE_THREADED_AVL
    if (size == 0) return 0;
    auto it = songs.beginIt();
    for (int i = 0; i < size; i++) {
        total += it.value()->getScore();
        ++it;
    }
#else
    for (int i = 0; i < size; i++) {
        Song* s = getSong(i);
        if (s != nullptr) {
            total += s->getScore();
        }
    }
#endif

    return total;
}

bool Playlist::compareTo(const Playlist& p, int numSong) {
    // TODO
    int myLimit = (numSong < this->size) ? numSong : this->size;
    int pLimit = (numSong < p.size) ? numSong : p.size;
    int myScore = 0;
    for (int i = 0; i < myLimit; i++) {
        Song* s = this->getSong(i);
        if (s != nullptr) {
            myScore += s->getScore();
        }
    }
    int pScore = 0;
    for (int i = 0; i < pLimit; i++) {
        Song* s = p.getSong(i);
        if (s != nullptr) {
            pScore += s->getScore();
        }
    }
    return myScore >= pScore;
}

// =======================
// Advanced playing modes (TODO)
// =======================

void Playlist::playRandom(int index) {
    // TODO
    if (index <0 || index >= size) return;
    currentIndex = index;
    #ifdef USE_THREADED_AVL
    currentIt = songs.beginIt();
    for (int i = 0; i < index; i++) ++currentIt;
    hasCurrent = true;
    #else
    #endif
}

int Playlist::playApproximate(int step) {
    if (size == 0) return -1;

    // Nếu playlist chưa phát (current_index = -1), coi như bắt đầu từ 0 theo như comment test case
    int start_idx = (currentIndex == -1) ? 0 : currentIndex;

    // Công thức tính wrap-around chuẩn cân được cả số âm và dương:
    currentIndex = ((start_idx + step) % size + size) % size;

    return currentIndex;
}