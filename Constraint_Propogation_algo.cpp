#include <iostream>
#include <assert.h>
#include <unistd.h>
#include <stdexcept>
constexpr int n = 9;
constexpr int nn = 3;
typedef char grid_t[n][n]; 
typedef unsigned short notes_t[n][n]; 
int verbose = 0;
long guesses = 0;
class sudoku {
 grid_t grid;
 notes_t notes;
public:
 sudoku();
 template<typename Iter> explicit sudoku(Iter first, Iter last);
 sudoku(std::initializer_list<int> const &l) : sudoku(l.begin(), l.end()) {}
 template<typename Container> explicit sudoku(Container const &c) : 
sudoku(c.begin(), c.end()) {}
 bool solve();
 void set_cell(int i, int j, int v);
 bool search(int i, int j);
 bool verify() const;
 void print_grid(char rowsep = '\n') const;
 void print_notes() const;
};
sudoku::sudoku() : grid {0} {
 for (int i = 0; i < n; ++i) {
 for (int j = 0; j < n; ++j) {
 notes[i][j] = 0x1ff;
 }
 }
}
template<typename Iter>
sudoku::sudoku(Iter first, Iter last) : sudoku() {
 int i = 0;
 for ( ; first != last; ++i, ++first) {
 if (i >= n * n) { throw std::length_error("sudoku constructor: 
initializer too long"); }
 auto v = *first;
 if (v < 0 || v > 9) { throw std::out_of_range("sudoku constructor: 
element out of range"); }
 set_cell(i / n, i % n, v);
 }
 if (i < n * n) { throw std::length_error("sudoku constructor: initializer 
too short"); }
}
class findbit {
 char setbit[1 << n];
public:
 findbit() : setbit {0} {
 for (int i = 0; i < n; ++i) {
 setbit[1 << i] = i + 1;
 }
 }
 // Returns the 1-based bit that is set if there is a single bit, 0
 // otherwise.
 int operator()(int i) const {
 assert(i < sizeof setbit);
 return setbit[i];
 }
};
findbit findbit;
int
unknown_count(const grid_t &grid) {
 int unknowns = 0;
 for (int i = 0; i < n * n; i++) {
 if (grid[i / n][i % n] == 0) {
 ++unknowns;
 }
 }
 return unknowns;
}
void
sudoku::print_grid(char rowsep) const {
 for (int i = 0; i < n * n; ++i) {
 std::cout
 << static_cast<int>(grid[i / n][i % n])
 << (i == n * n - 1 ? '\n' : i % n == n-1 ? rowsep : ' ');
 }
}
void
sudoku::print_notes() const {
 for (int i = 0; i < n; ++i) {
 for (int j = 0; j < n; ++j) {
 assert(notes[i][j] < 0x200);
 for (int k = 0; k < 9; k++) {
 std::cout << static_cast<char>((notes[i][j] & (1 << k)) ? '1' + 
k : ' ');
 }
 std::cout << (j == n-1 ? "\n" : j % nn == nn-1 ? " | " : " : ");
 }
 if (i % nn == nn - 1 && i != n - 1) {
std::cout<<"----------+-----------+-----------+-----------+-----------+-------
----+-----------+-----------+----------\n";
 }
 }
 std::cout << '\n';
}
inline int valmask(int v) { return 1 << (v - 1); }
void
sudoku::set_cell(int i, int j, int v) {
 if (v == 0) { return; }
 // Update the cell value.
 grid[i][j] = v;
 notes[i][j] = 0;
 // a and b are top-left corner of square containing i, j
 int a = (i / nn) * nn;
 int b = (j / nn) * nn;
 // Update the notes for the cells row, column, and square.
 int vmask = ~ valmask(v) & 0x1ff;
 for (int k = 0; k < n; ++k) {
 // std::printf(" vmask: %x %x -> %x\n", vmask, notes[i][k], notes[i][k] 
& vmask);
 notes[i][k] &= vmask; // propagate to row.
 notes[k][j] &= vmask; // propagate to column.
 notes[a + (k % nn)][b + (k / nn)] &= vmask; // propagate to square.
 }
}
bool
sudoku::search(int i, int j) {
 // Find a blank cell starting from grid[i][j].
 for ( ; i < n; ++i) {
 for ( ; j < n; ++j) {
 if (grid[i][j] == 0) {
 goto found_blank;
 }
 }
 j = 0;
 }
 // If we get here the sudoku is solved.
 return true;
found_blank:
 int bits = notes[i][j];
 // if bits is 0 then we have found a blank cell but there are
 // no allowed candidates for it.
 if (bits == 0) {
 return false;
 }
 // For each of the allowed candidates for this cell, set its value to the
 // candidate and then attempt to solve the rest of the puzzle.
 for (int k = 0; k < n; ++k) {
 if (bits & (1 << k)) {
 sudoku search_state = *this;
 int v = k + 1;
 search_state.set_cell(i, j, v);
 if (search_state.search(i, j)) {
 *this = search_state;
 return true;
 }
 ++guesses;
 }
 }
 return false;
}
bool
sudoku::solve() {
 if (verbose) {
 std::printf("unknowns before constraint propagation: %d\n", 
unknown_count(grid));
 }
 // Make one or more passes attempting to update the grid with values we
 // are certain about based on initial notes. For puzzles that are not too
 // difficult this will find some values and reduce the amount of searching
 // we have to do.
 int found_update = 1;
 while (found_update) {
 found_update = 0;
 // print_notes();
 for (int i = 0; i < n; ++i) {
 for (int j = 0; j < n; ++j) {
 if (grid[i][j] == 0) {
 int v = findbit(notes[i][j]);
 if (v > 0) {
 set_cell(i, j, v);
 found_update = 1;
 }
 }
 }
 }
 }
 // print_grid();
 if (verbose) {
 std::printf("unknowns after constraint propagation: %d\n", 
unknown_count(grid));
 }
 // Now do search to find missing values.
 bool found = search(0, 0);
 return found;
}
bool
sudoku::verify() const {
 for (int k = 0; k < n; ++k) {
 // a and b are top-left corner of square k.
 int a = (k / nn) * nn;
 int b = (k % nn) * nn;
 int r = 0;
 int c = 0;
 int s = 0;
 for (int l = 0; l < n; ++l) {
 r |= 1 << grid[k][l]; // validate row.
 c |= 1 << grid[l][k]; // validate column.
 s |= 1 << grid[a + (l % nn)][b + (l / nn)]; // Validate square.
 }
 if (r != 0x3fe || c != 0x3fe || s != 0x3fe) {
 std::printf("%d r:%x c:%x s:%x\n", k, r, c, s);
 return false;
 }
 }
 return true;
}
void
skip_comment() {
 if ((std::cin >> std::ws).peek() == '#') {
 while (std::cin.get() != '\n') { ; }
 }
}
int main(int argc, char **argv) {
 char rowsep = '\n';
 
 skip_comment();
 int numtests = 0;
 std::cin >> numtests;
 for (int t = 0; t != numtests; ++t) {
 guesses = 0;
 sudoku sudoku;
 for (int i = 0; i < n * n; ++i) {
 skip_comment();
 int v;
 std::cin >> v;
 if (v < 0 || v > 9) {
 std::printf("Bad grid[%d][%d] value %d\n", i / n, i % n, v);
 exit(1);
 }
 sudoku.set_cell(i / n, i % n, v);
 }
 std::cout << "\n\n\n\n\n\n";
 // sudoku.print_grid();
 sudoku.solve();
 if (verbose) { std::printf("guesses: %ld\n", guesses); }
 if (! sudoku.verify()) { std::cout << "no valid solution found\n"; }
 sudoku.print_grid(rowsep);
 }
 return 0;
}
