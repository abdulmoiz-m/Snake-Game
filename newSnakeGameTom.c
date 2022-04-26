#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

//used to create snake peices and trophie objects
typedef struct drawableObj{
   int y,x;
   chtype ch;
} dObj;

enum Direction {
    // assigning num. to directions to detect reverse direction
    // change in direction is illegal if sum is 1 or 5, game over.
    up = 0,
    down = 1,
    left = 2,
    right = 3
} currentDirection;


void board(void);
void displayObj(dObj);
void checkInput(void);
void updateState(void);
void updateDisplay(void);
void addSnakePiece(dObj);
void removeSnakePiece(void);
dObj snakeTail(void);
void initializeGame(void);
void setDirection(enum Direction);
dObj nextHead(void);
void getEmptyCoords(int*, int*);
dObj trophy(int, int);
dObj empty(int, int);
void displayMessage(char*);
void exitGame();
chtype getCharAt(int,int);

#define BOARD_ROWS (LINES - 1)
#define BOARD_COLUMNS (COLS - 2)
#define BOARD_HALF_PERIMETER (LINES + COLS - 3)

bool gameOver, trophyPresent, winGame;
int xMax, yMax, snakeSize = 3, refreshDelay = 400, randNumber, trophy_time;
time_t trophyCreationTime;
dObj prevTrophy; // keep track of prev trophy

/**
 * Function: main()
 * Purpose:
 * Author:
**/
int main () {
    initscr(); //initialize ncurses library
    //clear();
    refresh();
    curs_set(false); // Don't display a cursor
    noecho(); // Don't echo any keypresses
    keypad(stdscr, true);
    signal(SIGINT, exitGame); //catch the interrupt signal

    initializeGame(); //initialize the game

    while (!gameOver) {
        checkInput(); //only checks input and sets direction
        updateState(); // updates snake peices and handles all collisons and win game
        updateDisplay(); // just refreshes the screen
    }

    //end cleanly
    if (winGame)
        displayMessage("You Won!");
    else
        displayMessage("Game Over");
    usleep(1300000);
    exitGame();
}

/**
 * Function: initializeGame()
 * Purpose:
 * Author:
**/
void initializeGame() {
    board(); //initialize the snake pit
    srand(time(NULL));
    gameOver = false;
    winGame = false;
    trophyPresent = false;

    //initializing a snake with three characters
    currentDirection = rand()%4; //sets a random direction 0 - 3;
    dObj nextSnakePeice = {BOARD_ROWS/2, (BOARD_COLUMNS/2)-2, '@'};
    displayObj(nextSnakePeice);
    addSnakePiece(nextSnakePeice);

    nextSnakePeice = nextHead();
    displayObj(nextSnakePeice);
    addSnakePiece(nextSnakePeice);

    nextSnakePeice = nextHead();
    displayObj(nextSnakePeice);
    addSnakePiece(nextSnakePeice);

    //create the initial trophy
    if (!trophyPresent) {
        int y,x;
        getEmptyCoords(&y,&x);
        displayObj(prevTrophy = trophy(y, x));
        trophyPresent = true;
    }
}

/**
 * Function: board()
 * Purpose: displays the snake pit
 * Author:
**/
void board() {
    getmaxyx(stdscr, yMax, xMax); //get dimentions of terminal

    //creating a new window to display the board
    stdscr = newwin(BOARD_ROWS, BOARD_COLUMNS, (yMax/2)-(BOARD_ROWS/2), (xMax/2)-(BOARD_COLUMNS/2));
    box(stdscr, 0, 0); //box representing the border
    wrefresh(stdscr);
    wtimeout(stdscr, refreshDelay); //how fast screen is refreshed(500 ms)
}

/**
 * Function: displayCharAt()
 * Purpose: displays a character on the board at specified position
 * Author:
**/
void displayCharAt(int yPos, int xPos, chtype ch) {
    mvwaddch(stdscr, yPos, xPos, ch);
}

/**
 * Function: displayObj()
 * Purpose: displays a displayable object
 * Author:
**/
void displayObj(dObj obj) {
    displayCharAt(obj.y, obj.x, obj.ch);
}

/**
 * Function: checkInput()
 * Purpose: checks if a key is being held and parses that into a variable
 * Author: Corwin & Tom
**/
void checkInput() {
    chtype input = wgetch(stdscr);

    switch (input) {
        case KEY_UP:
        case 'w':
            setDirection(up);
            break;
        case KEY_DOWN:
        case 's':
            setDirection(down);
            break;
        case KEY_RIGHT:
        case 'd':
            setDirection(right);
            break;
        case KEY_LEFT:
        case 'a':
            setDirection(left);
            break;
        default:
            break;
    }
}

/**
 * Function: trophy()
 * Purpose:
 * Author:
**/
dObj trophy(int y, int x) { //used to create trophy object
    randNumber = (rand()%9)+1;
    trophy_time = (rand()%9)+1;
    trophyCreationTime = time(NULL);
    dObj trophy = {y, x, (randNumber + '0')};
    return trophy;
}

/**
 * Function: updateState()
 * Purpose:
 * Author:
**/
void updateState() {
    //updating the snake
    dObj nextSnakePeice = nextHead();
    int nextY = nextSnakePeice.y, nextX = nextSnakePeice.x;
    //if (nextSnakePeice.x != prevTrophy.x || nextSnakePeice.y != prevTrophy.y) {
    if (getCharAt(nextSnakePeice.y, nextSnakePeice.x) == ' ') {
        displayObj(empty(snakeTail().y, snakeTail().x));
        removeSnakePiece();
    }
    //49 is 1, 57 is 9 in acsii table
    else if (getCharAt(nextY, nextX) >= 49 && getCharAt(nextY, nextX) <= 57) {
    //else if (getCharAt(nextY, nextX) == 'T') {
        snakeSize= snakeSize + randNumber;
        trophyPresent = false;
        for(int i=1; i<randNumber;i++)
            addSnakePiece(nextSnakePeice);
    }
    else {
        gameOver = true;
        displayObj(empty(snakeTail().y, snakeTail().x));
        removeSnakePiece();
    }
    displayObj(nextSnakePeice);
    addSnakePiece(nextSnakePeice);

    //Check the elapsed time from trophy creation against trophy lifespan
    if((time(NULL) - trophyCreationTime) >= trophy_time){
      displayObj(empty(prevTrophy.y, prevTrophy.x));
      trophyPresent = false;
    }

    //if trophy gets eaten by the snake
    if (!trophyPresent) {
        int y,x;
        getEmptyCoords(&y,&x);
        displayObj(prevTrophy = trophy(y, x));
        trophyPresent = true;
    }

    refreshDelay -= snakeSize*10; //increase snake speed proportionl to size

    if (snakeSize >= BOARD_HALF_PERIMETER) {
        winGame = true;
        gameOver = true;
    }
}

/**
 * Function: updateDisplay()
 * Purpose: refreshes the window
 * Author:
**/
void updateDisplay() {
    wrefresh(stdscr);
}

// --------------------------------------------------------------------------
// Queue stuff
// Queue implemented as a doubly linked list
struct s_node {
    dObj *object;
    struct s_node *prev;
    struct s_node *next;
} *front=NULL, *back=NULL;
typedef struct s_node node;

/**
 * Function: peek()
 * Purpose: Returns the object at the front w/o dequeing
 * Author:
**/
dObj* peek( ) {
    return front == NULL ? NULL : front->object;
}

/**
 * Function: dequeue()
 * Purpose: Returns the object at the front and dequeues
 * Author:
**/
dObj* dequeue( ) {
    node *oldfront = front;
    front = front->next;
    return oldfront->object;
}

/**
 * Function: enqueue()
 * Purpose: Queues a object at the back
 * Author:
**/
void enqueue( dObj object )
{
   dObj *newobject   = (dObj*)  malloc( sizeof( object ) );
   node *newnode = (node*) malloc( sizeof( node ) );

   newobject->x = object.x;
   newobject->y = object.y;
   newobject->ch = object.ch;
   newnode->object = newobject;

   if( front == NULL && back == NULL )
       front = back = newnode;
   else {
       back->next = newnode;
       newnode->prev = back;
       back = newnode;
   }
}

/**
 * Function: peekBack()
 * Purpose: gets the node from the back of the list without dequeing
 * Author:
**/
dObj* peekBack( ) {
    return back == NULL ? NULL : back->object;
}
// --------------------------------------------------------------------------
// End Queue stuff

/**
 * Function: addSnakePiece()
 * Purpose: adds a snake piece to the queue
 * Author:
**/
void addSnakePiece(dObj piece) {
    enqueue(piece);
}

/**
 * Function: removeSnakePiece()
 * Purpose: removes a snake piece from the queue
 * Author:
**/
void removeSnakePiece() {
    dequeue();
}

/**
 * Function: snakeTail()
 * Purpose: gets the tail of the snake
 * Author:
**/
dObj snakeTail() {
    return *peek();
}

/**
 * Function: snakeHead()
 * Purpose: gets the head of the snake
 * Author:
**/
dObj snakeHead() {
    return *peekBack();
}

/**
 * Function: getDirection()
 * Purpose: gets the direction the snake is travelling
 * Author:
**/
enum Direction getDirection() {
    return currentDirection;
}

/**
 * Function: setDirection()
 * Purpose: detects if we are reversing the direction and ends game
 * Author:
**/
void setDirection(enum Direction newDirection) {
    int num = currentDirection + newDirection;
    if(num == 1 || num == 5) {
        gameOver = true;
        return;
    }
    currentDirection = newDirection;
}

/**
 * Function: nextHead()
 * Purpose: 
 * Author:
**/
dObj nextHead() {
    int currRow = snakeHead().y;
    int currCol = snakeHead().x;

    switch (currentDirection) {
        case down:
            currRow++;
            break;
        case up:
            currRow--;
            break;
        case left:
            currCol--;
            break;
        case right:
            currCol++;
            break;
    }
    dObj newSnakeHead = {currRow, currCol, '@'};
    return newSnakeHead;
}

/**
 * Function: empty()
 * Purpose: used to empty out a spot on the board
 * Author:
**/
dObj empty(int y, int x) {
    dObj blank = {y, x, ' '};
    return blank;
}

/**
 * Function: getCharAt()
 * Purpose: gets characters present at specified posiiton on board
 * Author:
**/
chtype getCharAt(int y, int x) {
    return mvwinch(stdscr, y, x);
}

/**
 * Function: getEmptyCoords()
 * Purpose: gets a set of random empty coords for trophy
 * Author:
**/
void getEmptyCoords(int *y, int *x) {
    while (getCharAt(*y = rand() % (BOARD_ROWS-1), *x = rand() % (BOARD_COLUMNS-1)) != ' ');
}

/**
 * Function: displayMessage()
 * Purpose: blanks the row, then writes whatever message was passed
 * Author:
**/
void displayMessage(char* str) { //displays a generic message on board
    move(BOARD_ROWS/2, 5);//goto the line
    hline(' ', BOARD_COLUMNS-5);//blank the line
    move(BOARD_ROWS/2, (BOARD_COLUMNS - strlen(str)) / 2);//goto the middle of the line -1/2 the string length
    printw("%s", str);
    refresh();
}

/**
 * Function: exitGame()
 * Purpose: exits the game cleanly
 * Author: Corwin Van Deusen
**/
void exitGame() {
    displayMessage("Exiting");
    usleep(1300000);
    endwin();
    exit(0);
}
