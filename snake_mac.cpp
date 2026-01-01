#include <iostream>
#include <unistd.h>     // usleep
#include <termios.h>    // terminal control
#include <fcntl.h>
#include <cstdlib>
#include <ctime>

using namespace std;

const int WIDTH = 40;
const int HEIGHT = 20;
const int MAX_LENGTH = 1000;

const char DIR_UP = 'U';
const char DIR_DOWN = 'D';
const char DIR_LEFT = 'L';
const char DIR_RIGHT = 'R';

struct Point {
    int x, y;
};

class Snake {
    int length;
    char direction;

public:
    Point body[MAX_LENGTH];

    Snake() {
        length = 1;
        body[0] = {WIDTH / 2, HEIGHT / 2};
        direction = DIR_RIGHT;
    }

    int getLength() const {
        return length;
    }

    Point getHead() const {
        return body[0];
    }

    void changeDirection(char d) {
        if ((d == DIR_UP && direction != DIR_DOWN) ||
            (d == DIR_DOWN && direction != DIR_UP) ||
            (d == DIR_LEFT && direction != DIR_RIGHT) ||
            (d == DIR_RIGHT && direction != DIR_LEFT)) {
            direction = d;
        }
    }

    bool move() {
        for (int i = length - 1; i > 0; i--)
            body[i] = body[i - 1];

        switch (direction) {
            case DIR_UP: body[0].y--; break;
            case DIR_DOWN: body[0].y++; break;
            case DIR_LEFT: body[0].x--; break;
            case DIR_RIGHT: body[0].x++; break;
        }

        if (body[0].x <= 0 || body[0].x >= WIDTH - 1 ||
            body[0].y <= 0 || body[0].y >= HEIGHT - 1)
            return false;

        for (int i = 1; i < length; i++) {
            if (body[0].x == body[i].x && body[0].y == body[i].y)
                return false;
        }

        return true;
    }

    void grow() {
        body[length] = body[length - 1];
        length++;
    }

    bool onBody(int x, int y) {
        for (int i = 0; i < length; i++)
            if (body[i].x == x && body[i].y == y)
                return true;
        return false;
    }
};

class Game {
    Snake snake;
    Point food;
    int score;

public:
    Game() {
        score = 0;
        spawnFood();
    }

    void spawnFood() {
        do {
            food.x = rand() % (WIDTH - 2) + 1;
            food.y = rand() % (HEIGHT - 2) + 1;
        } while (snake.onBody(food.x, food.y));
    }

    void draw() {
        cout << "\033[H";     // Move cursor to top-left
        for (int i = 0; i < WIDTH + 2; i++) cout << "#";
        cout << endl;

        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {
                if (x == 0 || x == WIDTH - 1 || y == 0 || y == HEIGHT - 1)
                    cout << "#";
                else if (x == food.x && y == food.y)
                    cout << "o";
                else {
                    bool printed = false;
                    for (int i = 0; i < snake.getLength(); i++) {
                        if (snake.body[i].x == x && snake.body[i].y == y) {
                            cout << "O";
                            printed = true;
                            break;
                        }
                    }
                    if (!printed) cout << " ";
                }
            }
            cout << endl;
        }
        cout << "Score: " << score << endl;
    }

    bool update() {
        if (!snake.move())
            return false;

        if (snake.getHead().x == food.x &&
            snake.getHead().y == food.y) {
            score++;
            snake.grow();
            spawnFood();
        }
        return true;
    }

    void input() {
        char c;
        if (read(STDIN_FILENO, &c, 1) > 0) {
            if (c == 'w') snake.changeDirection(DIR_UP);
            else if (c == 's') snake.changeDirection(DIR_DOWN);
            else if (c == 'a') snake.changeDirection(DIR_LEFT);
            else if (c == 'd') snake.changeDirection(DIR_RIGHT);
        }
    }

    int getScore() const {
        return score;
    }
};

// Non-blocking keyboard input
void setupTerminal(bool enable) {
    static struct termios oldt;
    struct termios newt;

    if (enable) {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
        fcntl(STDIN_FILENO, F_SETFL, 0);
    }
}

int main() {
    srand(time(0));
    system("clear");
    cout << "\033[?25l";   // Hide cursor

    setupTerminal(true);
    Game game;

    while (true) {
        game.input();
        if (!game.update())
            break;
        game.draw();
        usleep(120000);
    }

    setupTerminal(false);
    cout << "\033[?25h";   // Show cursor
    cout << "\nGAME OVER\nFinal Score: " << game.getScore() << endl;

    return 0;
}