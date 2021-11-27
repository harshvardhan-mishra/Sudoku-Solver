#include <iostream>
#include <cmath>
#include <string>
#include <ctime>
#define MAX_K 1000
#define SIZE 9
struct Node {
Node *left;
Node *right;
Node *up;
Node *down;
Node *head;
int size;
int rowID[3];
};
const int SIZE_SQUARED = SIZE*SIZE;
const int SIZE_SQRT = sqrt((double)SIZE);
const int ROW_NB = SIZE*SIZE*SIZE;
const int COL_NB = 4 * SIZE*SIZE;
struct Node Head;
struct Node* HeadNode = &Head;
struct Node* solution[MAX_K];
struct Node* orig_values[MAX_K];
bool matrix[ROW_NB][COL_NB] = { { 0 } };
bool isSolved = false;
void MapSolutionToGrid(int Sudoku[][SIZE]);
void PrintGrid(int Sudoku[][SIZE]);
clock_t timer, timer2;
void coverColumn(Node* col) {
col->left->right = col->right;
col->right->left = col->left;
for (Node* node = col->down; node != col; node = node->down) {
for (Node* temp = node->right; temp != node; temp = temp->right) {
temp->down->up = temp->up;
temp->up->down = temp->down;
temp->head->size--;
}
}
}
void uncoverColumn(Node* col) {
for (Node* node = col->up; node != col; node = node->up) {
for (Node* temp = node->left; temp != node; temp = temp->left)
{
temp->head->size++;
temp->down->up = temp;
temp->up->down = temp;
}
}
col->left->right = col;
col->right->left = col;
}
void search(int k) {
if (HeadNode->right == HeadNode) {
timer2 = clock() - timer;
int Grid[SIZE][SIZE] = { {0} };
MapSolutionToGrid(Grid);
PrintGrid(Grid);
std::cout << "Time Elapsed: " << (float)timer2 / CLOCKS_PER_SEC << 
" seconds.\n\n";
std::cin.get(); //Pause console
timer = clock();
isSolved = true;
return;
}
Node* Col = HeadNode->right;
for (Node* temp = Col->right; temp != HeadNode; temp = temp->right)
if (temp->size < Col->size)
Col = temp;
coverColumn(Col);
for (Node* temp = Col->down; temp != Col; temp = temp->down) {
solution[k] = temp;
for (Node* node = temp->right; node != temp; node = node->right) {
coverColumn(node->head);
}
search(k + 1);
temp = solution[k];
solution[k] = NULL;
Col = temp->head;
for (Node* node = temp->left; node != temp; node = node->left) {
uncoverColumn(node->head);
}
}
uncoverColumn(Col);
}
void BuildSparseMatrix(bool matrix[ROW_NB][COL_NB]) {
int j = 0, counter = 0;
for (int i = 0; i < ROW_NB; i++) { //iterate over all rows
matrix[i][j] = 1;
counter++;
if (counter >= SIZE) {
j++;
counter = 0;
}
}
int x = 0;
counter = 1;
for (j = SIZE_SQUARED; j < 2 * SIZE_SQUARED; j++) {
for (int i = x; i < counter*SIZE_SQUARED; i += SIZE)
matrix[i][j] = 1;
if ((j + 1) % SIZE == 0) {
x = counter*SIZE_SQUARED;
counter++;
}
else
x++;
}
j = 2 * SIZE_SQUARED;
for (int i = 0; i < ROW_NB; i++)
{
matrix[i][j] = 1;
j++;
if (j >= 3 * SIZE_SQUARED)
j = 2 * SIZE_SQUARED;
}
x = 0;
for (j = 3 * SIZE_SQUARED; j < COL_NB; j++) {
for (int l = 0; l < SIZE_SQRT; l++) {
for (int k = 0; k<SIZE_SQRT; k++)
matrix[x + l*SIZE + k*SIZE_SQUARED][j] = 1;
}
int temp = j + 1 - 3 * SIZE_SQUARED;
if (temp % (int)(SIZE_SQRT * SIZE) == 0)
x += (SIZE_SQRT - 1)*SIZE_SQUARED + (SIZE_SQRT - 1)*SIZE + 1;
else if (temp % SIZE == 0)
x += SIZE*(SIZE_SQRT - 1) + 1;
else
x++;
}
}
Node* header = new Node;
header->left = header;
header->right = header;
header->down = header;
header->up = header;
header->size = -1;
header->head = header;
Node* temp = header;
for (int i = 0; i < COL_NB; i++) {
Node* newNode = new Node;
newNode->size = 0;
newNode->up = newNode;
newNode->down = newNode;
newNode->head = newNode;
newNode->right = header;
newNode->left = temp;
temp->right = newNode;
temp = newNode;
}
int ID[3] = { 0,1,1 };
for (int i = 0; i < ROW_NB; i++) {
Node* top = header->right;
Node* prev = NULL;
if (i != 0 && i%SIZE_SQUARED == 0) {
ID[0] -= SIZE - 1;
ID[1]++;
ID[2] -= SIZE - 1;
}
else if (i!= 0 && i%SIZE == 0) {
ID[0] -= SIZE - 1;
ID[2]++;
}
else {
ID[0]++;
}
for (int j = 0; j < COL_NB; j++, top = top->right) {
if (matrix[i][j]) {
Node* newNode = new Node;
newNode->rowID[0] = ID[0];
newNode->rowID[1] = ID[1];
newNode->rowID[2] = ID[2];
if (prev == NULL) {
prev = newNode;
prev->right = newNode;
}
newNode->left = prev;
newNode->right = prev->right;
newNode->right->left = newNode;
prev->right = newNode;
newNode->head = top;
newNode->down = top;
newNode->up = top->up;
top->up->down = newNode;
top->size++;
top->up = newNode;
if (top->down == top)
top->down = newNode;
prev = newNode;
}
}
}
HeadNode = header;
}
void TransformListToCurrentGrid(int Puzzle[][SIZE]) {
int index = 0;
for(int i = 0 ; i<SIZE; i++ )
for(int j = 0 ; j<SIZE; j++)
if (Puzzle[i][j] > 0) {
Node* Col = NULL;
Node* temp = NULL;
for (Col = HeadNode->right; Col != HeadNode; Col = Col-
>right) {
for (temp = Col->down; temp != Col; temp = temp-
>down)
if (temp->rowID[0] == Puzzle[i][j] && 
(temp->rowID[1] - 1) == i && (temp->rowID[2] - 1) == j)
goto ExitLoops;
}
ExitLoops: coverColumn(Col);
orig_values[index] = temp;
index++;
for (Node* node = temp->right; node != temp; node = 
node->right) {
coverColumn(node->head);
}
}
}
void MapSolutionToGrid(int Sudoku[][SIZE]) {
for (int i = 0; solution[i] != NULL; i++) {
Sudoku[solution[i]->rowID[1]-1][solution[i]->rowID[2]-1] = 
solution[i]->rowID[0];
}
for (int i = 0; orig_values[i] != NULL; i++) {
Sudoku[orig_values[i]->rowID[1] - 1][orig_values[i]->rowID[2] - 1] 
= orig_values[i]->rowID[0];
}
}
//---------------------------------PRINTS A SUDOKU GRID OF ANY SIZE------------
---------------------------------//
void PrintGrid(int Sudoku[][SIZE]){
std::string ext_border = "+", int_border = "|";
int counter = 1;
int additional = 0;
if (SIZE > 9)
additional = SIZE;
for (int i = 0; i < ((SIZE +SIZE_SQRT - 1) * 2 +additional+ 1); i++) {
ext_border += '-';
if (i > 0 && i % ((SIZE_SQRT*2+SIZE_SQRT*(SIZE>9)+1)*counter + 
counter-1) == 0) {
int_border += '+';
counter++;
}
else
int_border += '-';
}
ext_border += '+';
int_border += "|";
std::cout << ext_border << std::endl;
for (int i = 0; i<SIZE; i++){
std::cout << "| ";
for (int j = 0; j<SIZE; j++){
if (Sudoku[i][j] == 0)
std::cout << ". ";
else
std::cout << Sudoku[i][j] << " ";
if (additional > 0 && Sudoku[i][j]<10)
std::cout << " ";
if ((j+1)%SIZE_SQRT == 0)
std::cout << "| ";
}
std::cout << std::endl;
if ((i + 1) % SIZE_SQRT == 0 && (i+1)<SIZE)
std::cout << int_border << std::endl;
}
std::cout << ext_border << std::endl << std::endl;
}
//----------------------------------------------------------------------------
----//
void SolveSudoku(int Sudoku[][SIZE]) {
timer = clock();
BuildSparseMatrix(matrix);
BuildLinkedList(matrix);
TransformListToCurrentGrid(Sudoku);
search(0);
if (!isSolved)
std::cout << "No Solution!" << std::endl;
isSolved = false;
}
int main(){
//Sudoku Hard to Brute Force
int Puzzle[9][9] = { { 0,0,0, 0,0,0, 0,0,0 },
{ 0,0,0, 0,0,3, 0,8,5 },
{ 0,0,1, 0,2,0, 0,0,0 },
{ 0,0,0, 5,0,7, 0,0,0 },
{ 0,0,4, 0,0,0, 1,0,0 },
{ 0,9,0, 0,0,0, 0,0,0 },
{ 5,0,0, 0,0,0, 0,7,3 },
{ 0,0,2, 0,1,0, 0,0,0 },
{ 0,0,0, 0,4,0, 0,0,9 }
};
int EmptyPuzzle[SIZE][SIZE] = { {0} };
SolveSudoku(Puzzle);
std::cin.get();
return 0;
}