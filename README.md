## Build
Basic AVL mode:
```bash
g++ -std=c++17 main.cpp Playlist.cpp -o app
```

Threaded mode (bonus):
```bash
g++ -std=c++17 -DUSE_THREADED_AVL main.cpp Playlist.cpp -o app
```