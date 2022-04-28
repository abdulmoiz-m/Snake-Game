#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>

/**
 * struct: drawableObj
 * Purpose: create snake peices and trophie objects with a posiiton
 * Author: Moiz
**/
typedef struct drawableObj{
   int y,x;
   chtype ch;
} dObj;

/**
 * enum: Direction
 * Purpose: refer to snakes direction and assign values to each direction for detecting reversal of direction
 * Author: Corwin
**/
enum Direction { 
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
int xMax, yMax, snakeSize = 3, refreshDelay = 250, randNumber, trophy_time;
time_t trophyCreationTime; 
dObj prevTrophy; //to keep track of prev trophy


/**
 * Function: main()
 * Purpose: initializes the game and contains the main game loop that updates the state of game and ends the game
 * Author: Thomas & Moiz
**/
int main () {
    initscr(); //initialize ncurses library
    refresh();
    curs_set(false); // Don't display a cursor
    noecho(); // Don't echo any keypresses
    keypad(stdscr, true);
    signal(SIGINT, exitGame); //catch the interrupt signal

    initializeGame(); //initialize the game

    while (!gameOver) {
        updateState(); // update game state
        checkInput(); //check input and set direction
        refresh(); //update display
    }

    //end game
    usleep(700000);
    clear();
    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    attron(A_BOLD);
    char *scoreMsg;
    asprintf(&scoreMsg, "Score: %d", snakeSize);

    if (winGame) {
        attron(COLOR_PAIR(2));
        displayMessage("You Won!");
        sleep(1);
        displayMessage(scoreMsg);
        attroff(COLOR_PAIR(2));
    }
    else {
        attron(COLOR_PAIR(1));
        displayMessage("Game Over");
        sleep(1);
        displayMessage(scoreMsg);
        attroff(COLOR_PAIR(1));
    }
    
    usleep(1500000);
    exitGame();
    attroff(A_BOLD);
}

/**
 * Function: initializeGame()
 * Purpose: initializes the game by setting up the snake pit, the snake and the first trophy
 * Author: Thomas
**/
void initializeGame() {
    board(); //initialize the snake pit
    refreshDelay -= (COLS < 250) ? (COLS/1.3) : 150; //decrease refresh dealy according to screen size
    srand(time(NULL));
    gameOver = false;
    winGame = false;
    trophyPresent = false;

    //initializing a snake with three characters going in random direction
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
 * Author: Corwin
**/
void board() {
    getmaxyx(stdscr, yMax, xMax); //get dimentions of terminal
    box(stdscr, 0, 0); //box representing the border
    refresh();
    timeout(refreshDelay);
}

/**
 * Function: displayCharAt()
 * Purpose: displays a character on the board at specified position
 * Author: Moiz
**/
void displayCharAt(int yPos, int xPos, chtype ch) {
    mvaddch(yPos, xPos, ch);
}

/**
 * Function: displayObj()
 * Purpose: uses displayCharAt() to display a displayable object (dObj)
 * Author: Moiz
**/
void displayObj(dObj obj) {
    displayCharAt(obj.y, obj.x, obj.ch);
}

/**
 * Function: checkInput()
 * Purpose: checks if key pressed is an arrow key and sets new direction
 * Author: Corwin & Tom
**/
void checkInput() {
    chtype input = getch();

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
 * Purpose: creates a trophy object of type dObj with a random integer and sets random display time
 * Author: Corwin
**/
dObj trophy(int y, int x) {
    randNumber = (rand()%9)+1;
    trophy_time = (rand()%9)+1;
    trophyCreationTime = time(NULL);
    dObj trophy = {y, x, (randNumber + '0')};
    return trophy;
}

int increaseLengthBy = 0;
/**
 * Function: updateState()
 * Purpose: updates the state of the game, i.e, moves snake, creates new trophies, increases snake length and detects collisions
 * Author: Thomas, Moiz & Corwin
**/
void updateState() {
    //getting next snake head
    dObj nextSnakePeice = nextHead();
    int nextY = nextSnakePeice.y, nextX = nextSnakePeice.x;

    if (getCharAt(nextY, nextX) == ' ') { //if snake moves across empty space
        displayObj(empty(snakeTail().y, snakeTail().x));
        removeSnakePiece();
    }
    else if (getCharAt(nextY, nextX) >= 49 && getCharAt(nextY, nextX) <= 57) { //if snake eats a trophy
        //49 to 57 represent integrers 1 - 9 in acsii table
        snakeSize += randNumber;
        increaseLengthBy += randNumber;
        trophyPresent = false;
        // for(int i=1; i<randNumber;i++)
        //     addSnakePiece(nextSnakePeice);
    }
    else {
        gameOver = true;
        displayObj(empty(snakeTail().y, snakeTail().x));
        removeSnakePiece();
    }
    // increases length of the snake
    if (increaseLengthBy > 1) {
        displayObj(nextSnakePeice);
        addSnakePiece(nextSnakePeice);
        increaseLengthBy--;
        if (refreshDelay >= 60) refreshDelay -= 6; //increase snake speed proportionl to size
    }
    displayObj(nextSnakePeice);
    addSnakePiece(nextSnakePeice);

    //Check the elapsed time from trophy creation against trophy lifespan
    if((time(NULL) - trophyCreationTime) >= trophy_time){
        if (getCharAt(prevTrophy.y, prevTrophy.x) != '@')
            displayObj(empty(prevTrophy.y, prevTrophy.x));
      trophyPresent = false;
    }

    //if trophy gets eaten by the snake create new one
    if (!trophyPresent) {
        int y,x;
        getEmptyCoords(&y,&x);
        displayObj(prevTrophy = trophy(y, x));
        trophyPresent = true;
    }

    //check if snakeSize reaches half the perimeter of the board
    if (snakeSize >= BOARD_HALF_PERIMETER) {
        winGame = true;
        gameOver = true;
    }
}


/**
 * Code Block: Queue implementation
 * Purpose: implements a queue data structure with the functions,
 *          peek() that returns the object at the front w/o dequeing,
 *          dequeue() that returns the object at the front and dequeues,
 *          enqueue() that queues an object at the back, and
 *          peekBack() that returns the object at the back w/o dequeing.
 * Author: Moiz & Thomas
**/
// --------------------------------------------------------------------------
// Queue implemented as a doubly linked list
struct s_node {
    dObj *object;
    struct s_node *prev;
    struct s_node *next;
} *front=NULL, *back=NULL;
typedef struct s_node node;

// Returns the object at the front w/o dequeing
dObj* peek() {
    return front == NULL ? NULL : front->object;
}

// Returns the object at the front and dequeues
dObj* dequeue() {
    node *oldfront = front;
    front = front->next;
    return oldfront->object;
}

// Queues a object at the back
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

// Returns the object at the back w/o dequeing.
dObj* peekBack( ) {
    return back == NULL ? NULL : back->object;
}
// --------------------------------------------------------------------------
// End of Queue components

/**
 * Function: addSnakePiece()
 * Purpose: adds a snake piece to the queue
 * Author: Corwin
**/
void addSnakePiece(dObj piece) {
    enqueue(piece);
}

/**
 * Function: removeSnakePiece()
 * Purpose: removes a snake piece from the queue
 * Author: Corwin
**/
void removeSnakePiece() {
    dequeue();
}

/**
 * Function: snakeTail()
 * Purpose: gets the tail of the snake
 * Author: Thomas
**/
dObj snakeTail() {
    return *peek();
}

/**
 * Function: snakeHead()
 * Purpose: gets the head of the snake
 * Author: Thomas
**/
dObj snakeHead() {
    return *peekBack();
}

/**
 * Function: setDirection()
 * Purpose: changes direction and detects if snake runs into itself
 * Author: Corwin
**/
void setDirection(enum Direction newDirection) {
// change in direction is illegal if sum is 1 or 5.
    int num = currentDirection + newDirection;
    if(num == 1 || num == 5) {
        gameOver = true;
        displayMessage("Wrong Direction! You ran into yourself.");
        sleep(2);
        return;
    }
    currentDirection = newDirection;
}

/**
 * Function: nextHead()
 * Purpose: to refer to the current head of the snake and get the next head based on the current direction set
 * Author: Moiz
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
 * Purpose: creates a dObj (display object) that is used to erase a character at specified position 
 * Author: Thomas
**/
dObj empty(int y, int x) { //used to empty out a spot on the board
    dObj blank = {y, x, ' '};
    return blank;
}

/**
 * Function: getCharAt()
 * Purpose: gets the character present at specified posiiton on board
 * Author: Moiz
**/
chtype getCharAt(int y, int x) {
    return mvinch(y, x);
}

/**
 * Function: getEmptyCoords()
 * Purpose: gets a set of random empty coords within border for trophy
 * Author: Moiz
**/
void getEmptyCoords(int *y, int *x) { //gets a set of random empty coords for trophy
    while (getCharAt(*y = rand() % (BOARD_ROWS-1), *x = rand() % (BOARD_COLUMNS-1)) != ' ');
}

/**
 * Function: displayMessage()
 * Purpose: blanks the row, then writes whatever message was passed
 * Author: Thomas
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
 * Author: Corwin
**/
void exitGame() {
    displayMessage("Exiting");
    usleep(1300000);
    endwin();
    exit(0);
}
