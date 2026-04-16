// ============================================================
// test_all.cpp  –  Comprehensive test suite
// Compile WITHOUT threading:
//   g++ -std=c++17 -DTESTING -Wall -o test_normal test_all.cpp Playlist.cpp
// Compile WITH threading:
//   g++ -std=c++17 -DTESTING -DUSE_THREADED_AVL -Wall -o test_threaded test_all.cpp Playlist.cpp
// ============================================================

#include "main.h"
#include "AVL.h"
#include "ThreadedAVL.h"
#include "Playlist.h"

#include <cassert>
#include <vector>
#include <algorithm>
#include <chrono>
#include <cmath>

// ────────────────────────────────────────────────────
// Tiny test-framework helpers
// ────────────────────────────────────────────────────
static int g_pass = 0, g_fail = 0;

#define ASSERT_TRUE(cond, msg)                                         \
    do {                                                               \
        if (cond) { ++g_pass; cout << "  [PASS] " << msg << "\n"; }   \
        else { ++g_fail; cout << "  [FAIL] " << msg << "\n"; }        \
    } while (0)

#define ASSERT_EQ(a, b, msg) ASSERT_TRUE((a)==(b), msg)

#define SECTION(name) cout << "\n[" << name << "]\n"

// ============================================================
//  AVL TREE TESTS  (template<int,string>)
// ============================================================

// ────────────────────────────────────────────────────
// TC-AVL-01 : Empty tree properties
// ────────────────────────────────────────────────────
void test_avl_empty() {
    SECTION("TC-AVL-01  Empty tree");
    AVL<int,string> t;
    ASSERT_TRUE(t.empty(),        "empty() on fresh tree");
    ASSERT_EQ(t.size(), 0,        "size() == 0");
    ASSERT_EQ(t.height(), 0,      "height() == 0");
    ASSERT_EQ(t.find(0), nullptr, "find on empty returns nullptr");
    ASSERT_TRUE(!t.contains(42),  "contains on empty is false");
    ASSERT_TRUE(t.ascendingList().empty(),  "ascendingList is empty");
    ASSERT_TRUE(t.descendingList().empty(), "descendingList is empty");
    ASSERT_TRUE(!t.erase(5),      "erase non-existent returns false");
}

// ────────────────────────────────────────────────────
// TC-AVL-02 : Single node (boundary: 1 element)
// ────────────────────────────────────────────────────
void test_avl_single_node() {
    SECTION("TC-AVL-02  Single node");
    AVL<int,string> t;
    ASSERT_TRUE(t.insert(10,"X"), "insert returns true");
    ASSERT_EQ(t.size(), 1,        "size == 1");
    ASSERT_EQ(t.height(), 1,      "height == 1 after single insert");
    ASSERT_TRUE(t.contains(10),   "contains inserted key");
    ASSERT_EQ(*t.find(10), string("X"), "find returns correct value");
    ASSERT_TRUE(!t.empty(),       "not empty");

    auto asc = t.ascendingList();
    ASSERT_EQ(asc.size(), 1ul,    "ascendingList has 1 element");
    ASSERT_EQ(asc.front(), 10,    "first element is the inserted key");
}

// ────────────────────────────────────────────────────
// TC-AVL-03 : Duplicate insert
// ────────────────────────────────────────────────────
void test_avl_duplicate() {
    SECTION("TC-AVL-03  Duplicate insert");
    AVL<int,string> t;
    t.insert(5,"A");
    ASSERT_TRUE(!t.insert(5,"B"),  "duplicate insert returns false");
    ASSERT_EQ(t.size(), 1,         "size still 1 after duplicate");
    ASSERT_EQ(*t.find(5), string("A"), "value unchanged after duplicate");
}

// ────────────────────────────────────────────────────
// TC-AVL-04 : Right-Right rotation (LL-case inverted)
// ────────────────────────────────────────────────────
void test_avl_rr_rotation() {
    SECTION("TC-AVL-04  RR rotation");
    AVL<int,string> t;
    t.insert(10,"A");
    t.insert(20,"B");
    t.insert(30,"C");   // triggers left rotation
    ASSERT_EQ(t.size(), 3,      "size == 3");
    ASSERT_TRUE(t.height() <= 2, "height <= 2 after RR rotation");
    auto asc = t.ascendingList();
    vector<int> v(asc.begin(), asc.end());
    ASSERT_TRUE(v == vector<int>({10,20,30}), "ascending order correct");
}

// ────────────────────────────────────────────────────
// TC-AVL-05 : Left-Left rotation
// ────────────────────────────────────────────────────
void test_avl_ll_rotation() {
    SECTION("TC-AVL-05  LL rotation");
    AVL<int,string> t;
    t.insert(30,"A");
    t.insert(20,"B");
    t.insert(10,"C");   // triggers right rotation
    ASSERT_EQ(t.size(), 3,       "size == 3");
    ASSERT_TRUE(t.height() <= 2, "height <= 2 after LL rotation");
    auto asc = t.ascendingList();
    vector<int> v(asc.begin(), asc.end());
    ASSERT_TRUE(v == vector<int>({10,20,30}), "ascending order correct");
}

// ────────────────────────────────────────────────────
// TC-AVL-06 : Left-Right rotation
// ────────────────────────────────────────────────────
void test_avl_lr_rotation() {
    SECTION("TC-AVL-06  LR rotation");
    AVL<int,string> t;
    t.insert(30,"A");
    t.insert(10,"B");
    t.insert(20,"C");   // LR case
    ASSERT_TRUE(t.height() <= 2, "height <= 2 after LR rotation");
    auto asc = t.ascendingList();
    vector<int> v(asc.begin(), asc.end());
    ASSERT_TRUE(v == vector<int>({10,20,30}), "ascending order correct");
}

// ────────────────────────────────────────────────────
// TC-AVL-07 : Right-Left rotation
// ────────────────────────────────────────────────────
void test_avl_rl_rotation() {
    SECTION("TC-AVL-07  RL rotation");
    AVL<int,string> t;
    t.insert(10,"A");
    t.insert(30,"B");
    t.insert(20,"C");   // RL case
    ASSERT_TRUE(t.height() <= 2, "height <= 2 after RL rotation");
    auto asc = t.ascendingList();
    vector<int> v(asc.begin(), asc.end());
    ASSERT_TRUE(v == vector<int>({10,20,30}), "ascending order correct");
}

// ────────────────────────────────────────────────────
// TC-AVL-08 : Erase leaf node
// ────────────────────────────────────────────────────
void test_avl_erase_leaf() {
    SECTION("TC-AVL-08  Erase leaf");
    AVL<int,string> t;
    t.insert(20,"A"); t.insert(10,"B"); t.insert(30,"C");
    ASSERT_TRUE(t.erase(10),    "erase leaf returns true");
    ASSERT_EQ(t.size(), 2,      "size == 2 after erase");
    ASSERT_TRUE(!t.contains(10),"erased key not found");
    ASSERT_TRUE(t.contains(20), "root still present");
    ASSERT_TRUE(t.contains(30), "right child still present");
}

// ────────────────────────────────────────────────────
// TC-AVL-09 : Erase node with 1 child
// ────────────────────────────────────────────────────
void test_avl_erase_one_child() {
    SECTION("TC-AVL-09  Erase node with 1 child");
    AVL<int,string> t;
    t.insert(20,"A"); t.insert(10,"B"); t.insert(30,"C"); t.insert(5,"D");
    // 10 has only left child (5)
    ASSERT_TRUE(t.erase(10), "erase node with 1 child");
    ASSERT_EQ(t.size(), 3,   "size == 3");
    ASSERT_TRUE(!t.contains(10),"erased key gone");
    ASSERT_TRUE(t.contains(5),  "grandchild promoted");
    auto asc = t.ascendingList();
    vector<int> v(asc.begin(), asc.end());
    ASSERT_TRUE(v == vector<int>({5,20,30}), "ascending order after erase");
}

// ────────────────────────────────────────────────────
// TC-AVL-10 : Erase node with 2 children → replace by successor
// ────────────────────────────────────────────────────
void test_avl_erase_two_children() {
    SECTION("TC-AVL-10  Erase node with 2 children");
    AVL<int,string> t;
    for (int k : {50,30,70,20,40,60,80}) t.insert(k,"v");
    ASSERT_TRUE(t.erase(50), "erase root (2 children)");
    ASSERT_EQ(t.size(), 6,   "size == 6");
    ASSERT_TRUE(!t.contains(50), "erased key gone");
    auto asc = t.ascendingList();
    vector<int> v(asc.begin(), asc.end());
    vector<int> expected = {20,30,40,60,70,80};
    ASSERT_TRUE(v == expected, "ascending order correct after erase-2-children");
}

// ────────────────────────────────────────────────────
// TC-AVL-11 : Erase non-existent key
// ────────────────────────────────────────────────────
void test_avl_erase_nonexistent() {
    SECTION("TC-AVL-11  Erase non-existent");
    AVL<int,string> t;
    t.insert(1,"A");
    ASSERT_TRUE(!t.erase(999), "erase non-existent returns false");
    ASSERT_EQ(t.size(),1,      "size unchanged");
}

// ────────────────────────────────────────────────────
// TC-AVL-12 : kthNode boundary
// ────────────────────────────────────────────────────
void test_avl_kth_node() {
    SECTION("TC-AVL-12  kthNode");
    AVL<int,string> t;
    for (int k : {10,20,30,40,50}) t.insert(k,"v");
    // inorder: 10,20,30,40,50
    ASSERT_EQ(t.kthNode(0)->key, 10, "kth(0) == 10");
    ASSERT_EQ(t.kthNode(4)->key, 50, "kth(4) == 50");
    ASSERT_EQ(t.kthNode(2)->key, 30, "kth(2) == 30");
    ASSERT_EQ(t.kthNode(-1), nullptr, "kth(-1) returns nullptr");
    ASSERT_EQ(t.kthNode(5), nullptr,  "kth(N) returns nullptr");
}

// ────────────────────────────────────────────────────
// TC-AVL-13 : Clear then re-use
// ────────────────────────────────────────────────────
void test_avl_clear_reuse() {
    SECTION("TC-AVL-13  Clear and re-use");
    AVL<int,string> t;
    for (int i=1;i<=5;i++) t.insert(i,"v");
    t.clear();
    ASSERT_TRUE(t.empty(),  "empty after clear");
    ASSERT_EQ(t.size(), 0,  "size==0 after clear");
    ASSERT_EQ(t.height(),0, "height==0 after clear");
    t.insert(100,"X");
    ASSERT_EQ(t.size(), 1,  "can insert after clear");
    ASSERT_TRUE(t.contains(100), "new element found");
}

// ────────────────────────────────────────────────────
// TC-AVL-14 : Ascending / Descending list correctness
// ────────────────────────────────────────────────────
void test_avl_sorted_lists() {
    SECTION("TC-AVL-14  Sorted lists");
    AVL<int,string> t;
    vector<int> keys = {5,3,7,1,4,6,9,2,8};
    for (int k:keys) t.insert(k,"v");
    auto asc = t.ascendingList();
    vector<int> va(asc.begin(),asc.end());
    ASSERT_TRUE(is_sorted(va.begin(),va.end()), "ascendingList is sorted asc");

    auto dsc = t.descendingList();
    vector<int> vd(dsc.begin(),dsc.end());
    ASSERT_TRUE(is_sorted(vd.rbegin(),vd.rend()), "descendingList is sorted desc");
}

// ────────────────────────────────────────────────────
// TC-AVL-15 : Height constraint O(log n)
// ────────────────────────────────────────────────────
void test_avl_height_constraint() {
    SECTION("TC-AVL-15  Height constraint (log n)");
    AVL<int,string> t;
    int N = 1000;
    for (int i=1;i<=N;i++) t.insert(i,"v");   // worst-case sequential
    int h = t.height();
    int maxAllowed = (int)(1.5 * log2(N+1) + 2);
    ASSERT_TRUE(h <= maxAllowed,
        "height " + to_string(h) + " <= " + to_string(maxAllowed) + " (log-bound)");
}

// ────────────────────────────────────────────────────
// TC-AVL-16 : Alternating insert/erase, size consistency
// ────────────────────────────────────────────────────
void test_avl_interleaved_ops() {
    SECTION("TC-AVL-16  Interleaved insert/erase");
    AVL<int,string> t;
    for (int i=1;i<=10;i++) t.insert(i,"v");
    for (int i=2;i<=10;i+=2) t.erase(i);  // remove even
    ASSERT_EQ(t.size(),5, "size==5 after removing 5 evens");
    auto asc = t.ascendingList();
    vector<int> v(asc.begin(),asc.end());
    ASSERT_TRUE(v==vector<int>({1,3,5,7,9}), "only odd keys remain");
}

// ────────────────────────────────────────────────────
// TC-AVL-17 : Insert/erase all → empty
// ────────────────────────────────────────────────────
void test_avl_erase_all() {
    SECTION("TC-AVL-17  Erase all nodes");
    AVL<int,string> t;
    for (int i=1;i<=7;i++) t.insert(i,"v");
    for (int i=1;i<=7;i++) t.erase(i);
    ASSERT_TRUE(t.empty(),   "empty after erasing all");
    ASSERT_EQ(t.size(), 0,   "size==0");
    ASSERT_EQ(t.height(), 0, "height==0");
}

// ────────────────────────────────────────────────────
// TC-AVL-18 : Negative keys (boundary: negative domain)
// ────────────────────────────────────────────────────
void test_avl_negative_keys() {
    SECTION("TC-AVL-18  Negative keys");
    AVL<int,string> t;
    t.insert(-5,"A"); t.insert(-10,"B"); t.insert(-1,"C");
    auto asc = t.ascendingList();
    vector<int> v(asc.begin(),asc.end());
    ASSERT_TRUE(v==vector<int>({-10,-5,-1}), "negative keys sorted correctly");
}

// ────────────────────────────────────────────────────
// TC-AVL-19 : Large sequential insert performance
// ────────────────────────────────────────────────────
void test_avl_performance() {
    SECTION("TC-AVL-19  Performance: 10000 inserts");
    AVL<int,string> t;
    int N = 10000;
    auto start = chrono::high_resolution_clock::now();
    for (int i=1;i<=N;i++) t.insert(i,"v");
    auto end = chrono::high_resolution_clock::now();
    double ms = chrono::duration<double,milli>(end-start).count();
    ASSERT_EQ(t.size(), N, "size == N after bulk insert");
    ASSERT_TRUE(ms < 1000.0, "10000 inserts < 1000 ms (got "+to_string(ms)+"ms)");
    cout << "    [INFO] 10000 inserts took " << ms << " ms\n";
}

// ────────────────────────────────────────────────────
// TC-AVL-20 : toString on empty tree
// ────────────────────────────────────────────────────
void test_avl_to_string() {
    SECTION("TC-AVL-20  toString");
    AVL<int,string> t;
    ASSERT_EQ(t.toString(), string("(NULL)"), "empty tree toString == (NULL)");
    t.insert(5,"A");
    ASSERT_TRUE(!t.toString().empty(), "non-empty tree has non-empty toString");
}

// ============================================================
//  THREADED AVL TESTS
// ============================================================

// ────────────────────────────────────────────────────
// TC-TAVL-01 : Threading after insertions
// ────────────────────────────────────────────────────
void test_tavl_threading_insert() {
    SECTION("TC-TAVL-01  Threading after insertions");
    ThreadedAVL<int,string> t;
    vector<int> keys = {5,3,7,1,4,6,9};
    for (int k:keys) t.insert(k,"v");

    // Forward traversal via iterator
    auto it = t.beginIt();
    vector<int> fwd;
    for (; it != t.endIt(); ++it) fwd.push_back(it.key());
    vector<int> expected = {1,3,4,5,6,7,9};
    ASSERT_TRUE(fwd == expected, "forward threading correct after insert");

    // Backward traversal
    auto rit = t.rbeginIt();
    vector<int> bwd;
    for (; rit != t.endIt(); --rit) bwd.push_back(rit.key());
    vector<int> expRev = {9,7,6,5,4,3,1};
    ASSERT_TRUE(bwd == expRev, "backward threading correct after insert");
}

// ────────────────────────────────────────────────────
// TC-TAVL-02 : Single element threading
// ────────────────────────────────────────────────────
void test_tavl_single() {
    SECTION("TC-TAVL-02  Single element threading");
    ThreadedAVL<int,string> t;
    t.insert(42,"X");
    auto it = t.beginIt();
    ASSERT_TRUE(!it.isNull(),    "beginIt not null");
    ASSERT_EQ(it.key(), 42,      "beginIt.key == 42");
    ++it;
    ASSERT_TRUE(it == t.endIt(), "after ++, it == endIt");

    auto rit = t.rbeginIt();
    ASSERT_EQ(rit.key(), 42,     "rbeginIt.key == 42");
    --rit;
    ASSERT_TRUE(rit == t.endIt(),"after --, it == endIt");
}

// ────────────────────────────────────────────────────
// TC-TAVL-03 : Threading after erase (leaf)
// ────────────────────────────────────────────────────
void test_tavl_erase_leaf_threading() {
    SECTION("TC-TAVL-03  Threading after erase leaf");
    ThreadedAVL<int,string> t;
    for (int k:{10,5,15,3,7}) t.insert(k,"v");
    t.erase(3);
    auto it = t.beginIt();
    vector<int> fwd;
    for (; it != t.endIt(); ++it) fwd.push_back(it.key());
    ASSERT_TRUE(fwd == vector<int>({5,7,10,15}), "threading correct after leaf erase");
}

// ────────────────────────────────────────────────────
// TC-TAVL-04 : Threading after erase head (smallest)
// ────────────────────────────────────────────────────
void test_tavl_erase_head() {
    SECTION("TC-TAVL-04  Threading after erase head");
    ThreadedAVL<int,string> t;
    for (int k:{20,10,30}) t.insert(k,"v");
    t.erase(10); // head
    auto it = t.beginIt();
    ASSERT_TRUE(!it.isNull(),     "new head not null");
    ASSERT_EQ(it.key(), 20,       "new head == 20");
    vector<int> fwd;
    for (; it!=t.endIt(); ++it) fwd.push_back(it.key());
    ASSERT_TRUE(fwd==vector<int>({20,30}), "threading ok after erase head");
}

// ────────────────────────────────────────────────────
// TC-TAVL-05 : Threading after erase tail (largest)
// ────────────────────────────────────────────────────
void test_tavl_erase_tail() {
    SECTION("TC-TAVL-05  Threading after erase tail");
    ThreadedAVL<int,string> t;
    for (int k:{20,10,30}) t.insert(k,"v");
    t.erase(30); // tail
    auto rit = t.rbeginIt();
    ASSERT_TRUE(!rit.isNull(),    "new tail not null");
    ASSERT_EQ(rit.key(), 20,      "new tail == 20");
    vector<int> bwd;
    for (; rit!=t.endIt(); --rit) bwd.push_back(rit.key());
    ASSERT_TRUE(bwd==vector<int>({20,10}), "threading ok after erase tail");
}

// ────────────────────────────────────────────────────
// TC-TAVL-06 : Threading after erase node with 2 children
// ────────────────────────────────────────────────────
void test_tavl_erase_two_children_threading() {
    SECTION("TC-TAVL-06  Threading after erase 2-children node");
    ThreadedAVL<int,string> t;
    for (int k:{50,30,70,20,40,60,80}) t.insert(k,"v");
    t.erase(50);
    auto it = t.beginIt();
    vector<int> fwd;
    for (; it!=t.endIt(); ++it) fwd.push_back(it.key());
    vector<int> expected={20,30,40,60,70,80};
    ASSERT_TRUE(fwd==expected, "forward threading correct after 2-child erase");

    // Backward check
    auto rit = t.rbeginIt();
    vector<int> bwd;
    for (; rit!=t.endIt(); --rit) bwd.push_back(rit.key());
    vector<int> expRev={80,70,60,40,30,20};
    ASSERT_TRUE(bwd==expRev, "backward threading correct after 2-child erase");
}

// ────────────────────────────────────────────────────
// TC-TAVL-07 : kthNode using threading
// ────────────────────────────────────────────────────
void test_tavl_kth_node() {
    SECTION("TC-TAVL-07  kthNode via threading");
    ThreadedAVL<int,string> t;
    for (int k:{10,20,30,40,50}) t.insert(k,"v");
    ASSERT_EQ(t.kthNode(0)->key, 10,    "kth(0)==10");
    ASSERT_EQ(t.kthNode(4)->key, 50,    "kth(4)==50");
    ASSERT_EQ(t.kthNode(-1), nullptr,   "kth(-1)==nullptr");
    ASSERT_EQ(t.kthNode(5),  nullptr,   "kth(5)==nullptr");
}

// ────────────────────────────────────────────────────
// TC-TAVL-08 : ascendingList / descendingList via threading
// ────────────────────────────────────────────────────
void test_tavl_sorted_lists() {
    SECTION("TC-TAVL-08  Sorted lists via threading");
    ThreadedAVL<int,string> t;
    vector<int> keys={9,3,7,1,5};
    for (int k:keys) t.insert(k,"v");
    auto asc = t.ascendingList();
    vector<int> va(asc.begin(),asc.end());
    ASSERT_TRUE(is_sorted(va.begin(),va.end()), "ascendingList sorted asc");

    auto dsc = t.descendingList();
    vector<int> vd(dsc.begin(),dsc.end());
    ASSERT_TRUE(is_sorted(vd.rbegin(),vd.rend()), "descendingList sorted desc");
}

// ────────────────────────────────────────────────────
// TC-TAVL-09 : Clear resets head / tail
// ────────────────────────────────────────────────────
void test_tavl_clear() {
    SECTION("TC-TAVL-09  Clear resets threading");
    ThreadedAVL<int,string> t;
    for (int k:{1,2,3}) t.insert(k,"v");
    t.clear();
    ASSERT_TRUE(t.empty(),            "empty after clear");
    ASSERT_TRUE(t.beginIt().isNull(), "beginIt null after clear");
    ASSERT_TRUE(t.rbeginIt().isNull(),"rbeginIt null after clear");
    // re-insert after clear
    t.insert(99,"Z");
    ASSERT_EQ(t.beginIt().key(), 99, "new head after clear+insert");
}

// ────────────────────────────────────────────────────
// TC-TAVL-10 : findIt
// ────────────────────────────────────────────────────
void test_tavl_find_it() {
    SECTION("TC-TAVL-10  findIt");
    ThreadedAVL<int,string> t;
    for (int k:{10,20,30}) t.insert(k,"v");
    auto it = t.findIt(20);
    ASSERT_TRUE(!it.isNull(),      "findIt(20) not null");
    ASSERT_EQ(it.key(), 20,        "findIt(20) key correct");
    auto notFound = t.findIt(99);
    ASSERT_TRUE(notFound.isNull(), "findIt on missing key returns end");
}

// ────────────────────────────────────────────────────
// TC-TAVL-11 : Long chain erase (stress threading)
// ────────────────────────────────────────────────────
void test_tavl_stress_erase() {
    SECTION("TC-TAVL-11  Stress: insert 200, erase 100, check threading");
    ThreadedAVL<int,string> t;
    for (int i=0;i<200;i++) t.insert(i,"v");
    // erase every other node
    for (int i=0;i<200;i+=2) t.erase(i);
    ASSERT_EQ(t.size(), 100, "size==100 after stress erase");
    auto it = t.beginIt();
    vector<int> fwd;
    for (; it!=t.endIt(); ++it) fwd.push_back(it.key());
    ASSERT_EQ((int)fwd.size(), 100, "forward traversal yields 100 elements");
    ASSERT_TRUE(is_sorted(fwd.begin(),fwd.end()), "forward traversal sorted");

    auto rit = t.rbeginIt();
    vector<int> bwd;
    for (; rit!=t.endIt(); --rit) bwd.push_back(rit.key());
    ASSERT_EQ((int)bwd.size(), 100, "backward traversal yields 100 elements");
    ASSERT_TRUE(is_sorted(bwd.rbegin(), bwd.rend()), "backward traversal sorted desc");
}

// ============================================================
//  PLAYLIST TESTS
// ============================================================

// ────────────────────────────────────────────────────
// TC-PL-01 : Empty playlist
// ────────────────────────────────────────────────────
void test_pl_empty() {
    SECTION("TC-PL-01  Empty playlist");
    Playlist p("Empty");
    ASSERT_TRUE(p.empty(),     "empty() is true");
    ASSERT_EQ(p.getSize(), 0,  "getSize()==0");
    ASSERT_EQ(p.getSong(0), nullptr,    "getSong(0)==nullptr");
    ASSERT_EQ(p.playNext(), nullptr,    "playNext on empty==nullptr");
    ASSERT_EQ(p.playPrevious(), nullptr,"playPrevious on empty==nullptr");
    ASSERT_EQ(p.getTotalScore(), 0,     "getTotalScore on empty==0");
}

// ────────────────────────────────────────────────────
// TC-PL-02 : addSong / getSize / getSong (alphabetical order)
// ────────────────────────────────────────────────────
void test_pl_add_and_order() {
    SECTION("TC-PL-02  Add songs, alphabetical order");
    Playlist p("Mix");
    Song *s1 = new Song(1,"Zebra","A","X",100,3,"-");
    Song *s2 = new Song(2,"Apple","B","X",100,4,"-");
    Song *s3 = new Song(3,"Mango","C","X",100,5,"-");
    p.addSong(s1); p.addSong(s2); p.addSong(s3);
    ASSERT_EQ(p.getSize(), 3,       "size==3 after 3 adds");
    // alphabetical: Apple, Mango, Zebra
    ASSERT_EQ(p.getSong(0)->getName(), string("Apple"), "getSong(0)==Apple");
    ASSERT_EQ(p.getSong(1)->getName(), string("Mango"), "getSong(1)==Mango");
    ASSERT_EQ(p.getSong(2)->getName(), string("Zebra"), "getSong(2)==Zebra");
    ASSERT_EQ(p.getSong(3), nullptr, "getSong(3)==nullptr (out of bounds)");
    ASSERT_EQ(p.getSong(-1),nullptr, "getSong(-1)==nullptr");
}

// ────────────────────────────────────────────────────
// TC-PL-03 : Duplicate song (same title+id) not added
// ────────────────────────────────────────────────────
void test_pl_duplicate_song() {
    SECTION("TC-PL-03  Duplicate song rejected");
    Playlist p("Mix");
    Song *s1 = new Song(1,"A Song","X","Y",100,5,"-");
    Song *s2 = new Song(1,"A Song","X","Y",100,5,"-"); // same key
    p.addSong(s1);
    p.addSong(s2);
    ASSERT_EQ(p.getSize(), 1, "duplicate not added, size still 1");
    delete s2;
}

// ────────────────────────────────────────────────────
// TC-PL-04 : Same title, different ID (tie-break by id)
// ────────────────────────────────────────────────────
void test_pl_same_title_diff_id() {
    SECTION("TC-PL-04  Same title, different ID");
    Playlist p("Mix");
    Song *s1 = new Song(2,"Same","A","X",100,4,"-");
    Song *s2 = new Song(1,"Same","B","X",100,3,"-");
    p.addSong(s1); p.addSong(s2);
    ASSERT_EQ(p.getSize(), 2, "both songs added");
    // tie-break: id1 < id2, so song with id=1 comes first
    ASSERT_EQ(p.getSong(0)->getName(), string("Same"), "first is Same");
    ASSERT_EQ(p.getSong(1)->getName(), string("Same"), "second is Same");
}

// ────────────────────────────────────────────────────
// TC-PL-05 : removeSong boundary
// ────────────────────────────────────────────────────
void test_pl_remove_song() {
    SECTION("TC-PL-05  removeSong");
    Playlist p("Mix");
    p.addSong(new Song(1,"A","X","Y",100,3,"-"));
    p.addSong(new Song(2,"B","X","Y",100,4,"-"));
    p.addSong(new Song(3,"C","X","Y",100,5,"-"));
    // alphabetical order in AVL: A(idx=0), B(idx=1), C(idx=2)
    p.removeSong(1); // remove B
    ASSERT_EQ(p.getSize(), 2,       "size==2 after remove");
    ASSERT_EQ(p.getSong(0)->getName(), string("A"), "first is A");
    ASSERT_EQ(p.getSong(1)->getName(), string("C"), "second is C");

    // remove out-of-bounds: no crash, size unchanged
    p.removeSong(-1);
    p.removeSong(99);
    ASSERT_EQ(p.getSize(), 2, "size unchanged after invalid removes");
}

// ────────────────────────────────────────────────────
// TC-PL-06 : removeSong last element → empty
// ────────────────────────────────────────────────────
void test_pl_remove_all() {
    SECTION("TC-PL-06  Remove all → empty");
    Playlist p("Mix");
    p.addSong(new Song(1,"A","X","Y",100,3,"-"));
    p.removeSong(0);
    ASSERT_TRUE(p.empty(), "empty after removing only song");
    ASSERT_EQ(p.getSong(0), nullptr, "getSong returns nullptr");
}

// ────────────────────────────────────────────────────
// TC-PL-07 : playNext – circular wrap behavior
// Logic: ++currentIndex; currentIndex %= size;
//   - currentIndex starts at -1
//   - First call: -1+1=0 → song[0]
//   - After last song (size-1): (size-1)+1=size, %size=0 → wraps back to song[0]
//   - playNext NEVER returns nullptr on a non-empty list
// ────────────────────────────────────────────────────
void test_pl_playnext() {
    SECTION("TC-PL-07  playNext circular");
    Playlist p("Mix");
    p.addSong(new Song(1,"A","X","Y",100,3,"-"));
    p.addSong(new Song(2,"B","X","Y",100,4,"-"));
    p.addSong(new Song(3,"C","X","Y",100,5,"-"));
    // alphabetical in tree: A(0), B(1), C(2)

    Song *s = p.playNext();
    ASSERT_TRUE(s!=nullptr,            "1st playNext returns song");
    ASSERT_EQ(s->getName(),string("A"), "1st playNext == A");

    s = p.playNext();
    ASSERT_EQ(s->getName(),string("B"), "2nd playNext == B");

    s = p.playNext();
    ASSERT_EQ(s->getName(),string("C"), "3rd playNext == C");

    // 4th call: wraps around to A (circular)
    s = p.playNext();
    ASSERT_TRUE(s!=nullptr,            "4th playNext wraps - not nullptr");
    ASSERT_EQ(s->getName(),string("A"), "4th playNext wraps to A");
}

// ────────────────────────────────────────────────────
// TC-PL-08 : playPrevious – circular wrap behavior
// Logic: if(currentIndex==-1) currentIndex=0;
//        currentIndex = (currentIndex - 1 + size) % size;
//   - Before any play (currentIndex=-1): set 0, then (0-1+size)%size = size-1 → last song
//   - At song[0]: (0-1+size)%size = size-1 → wraps to last song
//   - playPrevious NEVER returns nullptr on a non-empty list
// ────────────────────────────────────────────────────
void test_pl_playprevious() {
    SECTION("TC-PL-08  playPrevious circular");
    Playlist p("Mix");
    p.addSong(new Song(1,"A","X","Y",100,3,"-"));
    p.addSong(new Song(2,"B","X","Y",100,4,"-"));
    p.addSong(new Song(3,"C","X","Y",100,5,"-"));
    // alphabetical: A(0), B(1), C(2)

    // Before any play: currentIndex=-1 → set 0 → (0-1+3)%3=2 → C
    Song *s = p.playPrevious();
    ASSERT_TRUE(s!=nullptr,            "playPrevious before any play wraps to last");
    ASSERT_EQ(s->getName(),string("C"), "playPrevious before any play == C");

    // currentIndex is now 2 (C); prev: (2-1+3)%3=1 → B
    s = p.playPrevious();
    ASSERT_EQ(s->getName(),string("B"), "playPrevious from C == B");

    // currentIndex=1 (B); prev: (1-1+3)%3=0 → A
    s = p.playPrevious();
    ASSERT_EQ(s->getName(),string("A"), "playPrevious from B == A");

    // currentIndex=0 (A); prev wraps: (0-1+3)%3=2 → C
    s = p.playPrevious();
    ASSERT_TRUE(s!=nullptr,            "playPrevious at first wraps - not nullptr");
    ASSERT_EQ(s->getName(),string("C"), "playPrevious from A wraps to C");
}

// ────────────────────────────────────────────────────
// TC-PL-09 : playRandom then playNext/playPrevious
// ────────────────────────────────────────────────────
void test_pl_play_random() {
    SECTION("TC-PL-09  playRandom");
    Playlist p("Mix");
    p.addSong(new Song(1,"A","X","Y",100,3,"-"));
    p.addSong(new Song(2,"B","X","Y",100,4,"-"));
    p.addSong(new Song(3,"C","X","Y",100,5,"-"));
    // alphabetical: A(0), B(1), C(2)

    p.playRandom(2); // jump to index 2 (C)
    ASSERT_EQ(p.getSong(2)->getName(), string("C"), "getSong(2) == C");

    // After playRandom(2), currentIndex=2 (last)
    // playNext: ++2=3, %3=0 → A (wraps)
    Song *nxt = p.playNext();
    ASSERT_TRUE(nxt!=nullptr,             "playNext after playRandom(last) wraps");
    ASSERT_EQ(nxt->getName(),string("A"), "playNext after playRandom(2) wraps to A");

    // Boundary: playRandom(-1) and playRandom(size) do nothing
    int szBefore = p.getSize();
    p.playRandom(-1);
    p.playRandom(99);
    ASSERT_EQ(p.getSize(), szBefore, "size unchanged after invalid playRandom");
    ASSERT_EQ(p.getSize(), 3, "size still 3");
}

// ────────────────────────────────────────────────────
// TC-PL-10 : playApproximate
// Logic: if(size==0) return currentIndex; (returns -1 if never played)
//        if(currentIndex==-1) currentIndex=0;
//        temp = (currentIndex + step%size + size) % size;
//        currentIndex = temp;
//        return temp;
// ────────────────────────────────────────────────────
void test_pl_play_approximate() {
    SECTION("TC-PL-10  playApproximate");
    Playlist p("Mix");
    for (int i=1;i<=5;i++)
        p.addSong(new Song(i, string(1,(char)('A'+i-1)), "X","Y",100,i,"-"));
    // songs (alphabetical): A(0),B(1),C(2),D(3),E(4), size=5

    // currentIndex=-1 → set 0; step=3 → 3%5=3; (0+3+5)%5=3 → index 3
    int idx = p.playApproximate(3);
    ASSERT_EQ(idx, 3, "playApproximate(3) from -1: set 0 then jump -> 3");

    // currentIndex=3; step=7 → 7%5=2; (3+2+5)%5=0 → index 0
    idx = p.playApproximate(7);
    ASSERT_EQ(idx, 0, "playApproximate(7) from 3: wrap-around -> 0");

    // currentIndex=0; step=0 → 0%5=0; (0+0+5)%5=0 → stays 0
    idx = p.playApproximate(0);
    ASSERT_EQ(idx, 0, "playApproximate(0) stays at current index 0");

    // currentIndex=0; step=5 → 5%5=0; (0+0+5)%5=0 → stays 0
    idx = p.playApproximate(5);
    ASSERT_EQ(idx, 0, "playApproximate(5) full circle back to 0");

    // currentIndex=0; step=1 → 1%5=1; (0+1+5)%5=1 → index 1
    idx = p.playApproximate(1);
    ASSERT_EQ(idx, 1, "playApproximate(1) from 0 -> 1");

    // currentIndex=1; step=4 → 4%5=4; (1+4+5)%5=0 → index 0
    idx = p.playApproximate(4);
    ASSERT_EQ(idx, 0, "playApproximate(4) from 1 -> 0");

    // currentIndex=0; step=10 → 10%5=0; (0+0+5)%5=0 → stays 0
    idx = p.playApproximate(10);
    ASSERT_EQ(idx, 0, "playApproximate(10) multiple of size wraps to same");

    // empty playlist → returns currentIndex which is -1 (size==0 path)
    Playlist empty("E");
    int r = empty.playApproximate(5);
    ASSERT_EQ(r, -1, "playApproximate on empty returns -1 (currentIndex)");
}

// ────────────────────────────────────────────────────
// TC-PL-11 : getTotalScore
// ────────────────────────────────────────────────────
void test_pl_total_score() {
    SECTION("TC-PL-11  getTotalScore");
    Playlist p("Mix");
    p.addSong(new Song(1,"A","X","Y",100,3,"-"));
    p.addSong(new Song(2,"B","X","Y",100,4,"-"));
    p.addSong(new Song(3,"C","X","Y",100,5,"-"));
    ASSERT_EQ(p.getTotalScore(), 12, "total score == 3+4+5 == 12");
}

// ────────────────────────────────────────────────────
// TC-PL-12 : compareTo
// ────────────────────────────────────────────────────
void test_pl_compare_to() {
    SECTION("TC-PL-12  compareTo");
    Playlist p1("P1"), p2("P2");
    // P1 songs (alphabetical in AVL): A(3), B(5), C(4)
    p1.addSong(new Song(1,"A","X","Y",100,3,"-"));
    p1.addSong(new Song(2,"B","X","Y",100,5,"-"));
    p1.addSong(new Song(3,"C","X","Y",100,4,"-"));
    // P2 songs: A(4), B(3), C(5)
    p2.addSong(new Song(4,"A","X","Y",100,4,"-"));
    p2.addSong(new Song(5,"B","X","Y",100,3,"-"));
    p2.addSong(new Song(6,"C","X","Y",100,5,"-"));

    // compareTo uses first numSong songs in ascending key order
    // P1 first 2: A(3)+B(5)=8  vs  P2 first 2: A(4)+B(3)=7 → P1 >= P2 → true
    ASSERT_TRUE(p1.compareTo(p2,2),  "P1 >= P2 for first 2");
    // P1 first 1: A(3) vs P2 first 1: A(4) → P1 < P2 → false
    ASSERT_TRUE(!p1.compareTo(p2,1), "P1 < P2 for first 1");
    // numSong=0: both 0, equal → P1 >= P2 → true
    ASSERT_TRUE(p1.compareTo(p2,0),  "compareTo(0) equal → true");
    // P1 all 3: 3+5+4=12 vs P2 all 3: 4+3+5=12 → equal → true
    ASSERT_TRUE(p1.compareTo(p2,3),  "compareTo(3) equal total → true");
}

// ────────────────────────────────────────────────────
// TC-PL-13 : removeSong adjusts currentIndex correctly
// ────────────────────────────────────────────────────
void test_pl_remove_adjusts_index() {
    SECTION("TC-PL-13  removeSong adjusts currentIndex");
    Playlist p("Mix");
    p.addSong(new Song(1,"A","X","Y",100,3,"-"));
    p.addSong(new Song(2,"B","X","Y",100,4,"-"));
    p.addSong(new Song(3,"C","X","Y",100,5,"-"));
    // A(0), B(1), C(2), size=3

    p.playNext(); // A, currentIndex=0
    p.playNext(); // B, currentIndex=1

    // Remove song BEFORE currentIndex (A at index 0)
    // currentIndex should become 0 (decremented by 1)
    p.removeSong(0);
    ASSERT_EQ(p.getSize(), 2, "size==2 after removing A");
    // currentIndex was 1-1=0, now getSong(0)==B
    Song *cur = p.getSong(p.getcurr());
    ASSERT_TRUE(cur != nullptr, "getSong(currentIndex) not null");
    ASSERT_EQ(cur->getName(), string("B"), "currentIndex now points to B after removing prior song");
}

// ────────────────────────────────────────────────────
// TC-PL-14 : removeSong at currentIndex (not last)
//   → should reset or advance playback
// ────────────────────────────────────────────────────
void test_pl_remove_at_current() {
    SECTION("TC-PL-14  removeSong at currentIndex (not last)");
    Playlist p("Mix");
    p.addSong(new Song(1,"A","X","Y",100,3,"-"));
    p.addSong(new Song(2,"B","X","Y",100,4,"-"));
    p.addSong(new Song(3,"C","X","Y",100,5,"-"));
    // A(0), B(1), C(2)

    p.playNext(); // A, currentIndex=0
    p.playNext(); // B, currentIndex=1

    // Remove B (cur index=1, not last): size==1 path → no. index==size-1? No.
    // code: index==currentIndex, not last → does NOT reset currentIndex
    // (only resets if size==1)
    p.removeSong(1); // remove B (currentIndex)
    ASSERT_EQ(p.getSize(), 2, "size==2 after removing B");
    // currentIndex is still 1 in this implementation (iterator advances)
    // getSong(currentIndex) should exist
    ASSERT_TRUE(p.getSong(p.getcurr()) != nullptr || p.getcurr() == -1,
        "currentIndex after removing current is valid or reset");
}

// ────────────────────────────────────────────────────
// TC-PL-15 : removeSong at last position (currentIndex == size-1)
// ────────────────────────────────────────────────────
void test_pl_remove_at_last() {
    SECTION("TC-PL-15  removeSong at last which is currentIndex");
    Playlist p("Mix");
    p.addSong(new Song(1,"A","X","Y",100,3,"-"));
    p.addSong(new Song(2,"B","X","Y",100,4,"-"));
    p.addSong(new Song(3,"C","X","Y",100,5,"-"));
    // A(0), B(1), C(2)

    p.playNext(); // A=0
    p.playNext(); // B=1
    p.playNext(); // C=2, currentIndex=2 == size-1

    // Remove C (index=2=currentIndex=size-1) → currentIndex becomes 0
    p.removeSong(2);
    ASSERT_EQ(p.getSize(), 2, "size==2 after removing C");
    ASSERT_EQ(p.getcurr(), 0, "currentIndex wraps to 0 when last current removed");
    // getSong(0) == A
    ASSERT_EQ(p.getSong(0)->getName(), string("A"), "getSong(0)==A after remove");
}

// ────────────────────────────────────────────────────
// TC-PL-16 : Remove only song → empty, index = -1
// ────────────────────────────────────────────────────
void test_pl_single_song_playback() {
    SECTION("TC-PL-16  Single song playback then remove");
    Playlist p("Solo");
    p.addSong(new Song(1,"Only","X","Y",100,5,"-"));

    Song *s = p.playNext();
    ASSERT_EQ(s->getName(), string("Only"), "playNext on single song == Only");

    // Next call wraps back: currentIndex was 0, +1=1, %1=0 → A again
    Song *s2 = p.playNext();
    ASSERT_TRUE(s2 != nullptr,             "playNext wraps on single-song list");
    ASSERT_EQ(s2->getName(), string("Only"),"playNext wraps to Only again");

    // Remove only song
    p.removeSong(0);
    ASSERT_TRUE(p.empty(), "empty after removing only song");
    ASSERT_EQ(p.playNext(), nullptr,     "playNext on empty after remove == nullptr");
    ASSERT_EQ(p.playPrevious(), nullptr, "playPrevious on empty after remove == nullptr");
}

// ────────────────────────────────────────────────────
// TC-PL-17 : addSong after playNext adjusts currentIndex if needed
// If inserted song comes before currentIndex alphabetically, index increments
// ────────────────────────────────────────────────────
void test_pl_add_adjusts_index() {
    SECTION("TC-PL-17  addSong adjusts currentIndex for prior insertions");
    Playlist p("Mix");
    p.addSong(new Song(2,"B","X","Y",100,4,"-"));
    p.addSong(new Song(3,"C","X","Y",100,5,"-"));
    // B(0), C(1)

    p.playNext(); // B, currentIndex=0
    p.playNext(); // C, currentIndex=1

    // Insert A before B (alphabetically) → currentIndex was 1, A < C → currentIndex should be 2
    p.addSong(new Song(1,"A","X","Y",100,3,"-"));
    ASSERT_EQ(p.getSize(), 3, "size==3 after addSong");
    // currentIndex should now refer to C still (shifted to 2)
    ASSERT_EQ(p.getcurr(), 2, "currentIndex shifted to 2 after inserting A before current");
    ASSERT_EQ(p.getSong(2)->getName(), string("C"), "getSong(2) still C");
}

// ────────────────────────────────────────────────────
// TC-PL-18 : Playlist performance (N songs)
// ────────────────────────────────────────────────────
void test_pl_performance() {
    SECTION("TC-PL-18  Performance: 500 songs addSong+getSong");
    Playlist p("Big");
    int N = 500;
    auto start = chrono::high_resolution_clock::now();
    for (int i=1;i<=N;i++){
        string title = "Song" + to_string(i);
        p.addSong(new Song(i, title, "Artist","Album",200,i%5+1,"-"));
    }
    auto mid = chrono::high_resolution_clock::now();
    // access all songs in order
    for (int i=0;i<N;i++) p.getSong(i);
    auto end = chrono::high_resolution_clock::now();
    double addMs  = chrono::duration<double,milli>(mid-start).count();
    double getMs  = chrono::duration<double,milli>(end-mid).count();
    ASSERT_EQ(p.getSize(), N, "size == N");
    ASSERT_TRUE(addMs < 5000.0, "500 adds < 5000ms (got "+to_string(addMs)+"ms)");
    cout << "    [INFO] 500 addSong: " << addMs << "ms | 500 getSong: " << getMs << "ms\n";
}

// ============================================================
//  MAIN
// ============================================================
int main()
{
#ifdef USE_THREADED_AVL
    cout << "======================================\n";
    cout << " MODE: USE_THREADED_AVL\n";
    cout << "======================================\n";
#else
    cout << "======================================\n";
    cout << " MODE: Normal AVL\n";
    cout << "======================================\n";
#endif

    // ---- AVL tests ----
    test_avl_empty();
    test_avl_single_node();
    test_avl_duplicate();
    test_avl_rr_rotation();
    test_avl_ll_rotation();
    test_avl_lr_rotation();
    test_avl_rl_rotation();
    test_avl_erase_leaf();
    test_avl_erase_one_child();
    test_avl_erase_two_children();
    test_avl_erase_nonexistent();
    test_avl_kth_node();
    test_avl_clear_reuse();
    test_avl_sorted_lists();
    test_avl_height_constraint();
    test_avl_interleaved_ops();
    test_avl_erase_all();
    test_avl_negative_keys();
    test_avl_performance();
    test_avl_to_string();

    // ---- ThreadedAVL tests ----
    test_tavl_threading_insert();
    test_tavl_single();
    test_tavl_erase_leaf_threading();
    test_tavl_erase_head();
    test_tavl_erase_tail();
    test_tavl_erase_two_children_threading();
    test_tavl_kth_node();
    test_tavl_sorted_lists();
    test_tavl_clear();
    test_tavl_find_it();
    test_tavl_stress_erase();

    // ---- Playlist tests ----
    test_pl_empty();
    test_pl_add_and_order();
    test_pl_duplicate_song();
    test_pl_same_title_diff_id();
    test_pl_remove_song();
    test_pl_remove_all();
    test_pl_playnext();
    test_pl_playprevious();
    test_pl_play_random();
    test_pl_play_approximate();
    test_pl_total_score();
    test_pl_compare_to();
    test_pl_remove_adjusts_index();
    test_pl_remove_at_current();
    test_pl_remove_at_last();
    test_pl_single_song_playback();
    test_pl_add_adjusts_index();
    test_pl_performance();

    cout << "\n======================================\n";
    cout << "  RESULTS:  " << g_pass << " passed, " << g_fail << " failed\n";
    cout << "======================================\n";
    return g_fail > 0 ? 1 : 0;
}
