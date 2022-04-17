# Snake-Game
CS355 Snake Game Project

Write a C program snake that implements the classic snake game.
The snake pit:
•	The snake pit is the area where the snake can move.
•	The snake pit must utilize all available space of the current terminal window.
•	There must be a visible border delineating the snake pit.
The snake:
•	The initial length of the snake is three characters.
•	Initial direction of the snake's movement is chosen randomly.
•	The user can press one of the four arrow keys to change the direction of the snake's movement.
•	The snake's speed is proportional to its length.
The trophies:
•	Trophies are represented by a digit randomly chosen from 1 to 9.
•	There's always exactly one trophy in the snake pit at any given moment.
•	When the snake eats the trophy, its length is increased by the corresponding number of characters.
•	A trophy expires after a random interval from 1 to 9 seconds.
•	A new trophy is shown at a random location on the screen after the previous one has either expired or is eaten by the snake.
The gameplay:
•	The snake dies and the game ends if:
It runs into the border; or
It runs into itself; or
The user attempts to reverse the snake's direction.
•	The user wins the game if the snake's length grows to the length equal to half the perimeter of the border.

