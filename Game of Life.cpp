#include <SFML/Graphics.hpp>
#include <vector>

// -------------------- Classe Cell --------------------
class Cell {
private:
    bool alive;
    bool obstacle;
    sf::RectangleShape shape;

public:
    Cell(int x, int y, int size) : alive(false), obstacle(false) {
        shape.setSize(sf::Vector2f(size, size));
        shape.setFillColor(sf::Color::White);
        shape.setOutlineColor(sf::Color::Red);
        shape.setOutlineThickness(1.f);
        shape.setPosition(x * size + 2, y * size + 2);
    }

    bool isAlive() const { return alive; }
    bool isObstacle() const { return obstacle; }

    void setAlive(bool a) {
        if (!obstacle) {
            alive = a;
            shape.setFillColor(alive ? sf::Color::Black : sf::Color::White);
        }
    }

    void toggleObstacle() {
        obstacle = !obstacle;
        shape.setFillColor(obstacle ? sf::Color(100,100,100) : sf::Color::White);
    }

    void draw(sf::RenderWindow &window) { window.draw(shape); }
};

// -------------------- Classe RuleEngine --------------------
class RuleEngine {
public:
    bool applyRules(bool currentState, int neighbours, bool obstacle) {
        if (obstacle) return currentState; // obstacles ne changent pas
        if (currentState && neighbours < 2) return false;
        if (currentState && (neighbours == 2 || neighbours == 3)) return true;
        if (currentState && neighbours > 3) return false;
        if (!currentState && neighbours == 3) return true;
        return currentState;
    }
};

// -------------------- Classe Grid --------------------
class Grid {
public:
    static const int CELL_SIZE = 12;
    static const int WIDTH = 50;
    static const int HEIGHT = 30;

private:
    std::vector<std::vector<Cell>> cells;
    RuleEngine rules;

public:
    Grid() {
        for (int x = 0; x < WIDTH; x++) {
            std::vector<Cell> col;
            for (int y = 0; y < HEIGHT; y++) {
                col.emplace_back(x, y, CELL_SIZE);
            }
            cells.push_back(col);
        }
    }

    int countNeighbours(int x, int y) {
        int count = 0;
        int left = (x - 1 + WIDTH) % WIDTH;
        int right = (x + 1) % WIDTH;
        int up = (y - 1 + HEIGHT) % HEIGHT;
        int down = (y + 1) % HEIGHT;

        auto check = [&](int cx, int cy) {
            if (!cells[cx][cy].isObstacle() && cells[cx][cy].isAlive()) count++;
        };

        check(left, up); check(x, up); check(right, up);
        check(left, y);               check(right, y);
        check(left, down); check(x, down); check(right, down);

        return count;
    }

    void update() {
        std::vector<std::vector<bool>> next(WIDTH, std::vector<bool>(HEIGHT));
        for (int x = 0; x < WIDTH; x++) {
            for (int y = 0; y < HEIGHT; y++) {
                next[x][y] = rules.applyRules(cells[x][y].isAlive(),
                                              countNeighbours(x,y),
                                              cells[x][y].isObstacle());
            }
        }
        for (int x = 0; x < WIDTH; x++)
            for (int y = 0; y < HEIGHT; y++)
                cells[x][y].setAlive(next[x][y]);
    }

    void clear() {
        for (int x = 0; x < WIDTH; x++) {
            for (int y = 0; y < HEIGHT; y++) {
                cells[x][y].setAlive(false);

                // remettre l'obstacle à false manuellement
                if (cells[x][y].isObstacle()) {
                    cells[x][y].toggleObstacle(); 
                }
            }
        }
    }

    void draw(sf::RenderWindow &window) {
        for (int x = 0; x < WIDTH; x++)
            for (int y = 0; y < HEIGHT; y++)
                cells[x][y].draw(window);
    }

    Cell& getCell(int x, int y) { return cells[x][y]; }
};

const int Grid::CELL_SIZE;
const int Grid::WIDTH;
const int Grid::HEIGHT;

// -------------------- Classe PatternManager --------------------
class PatternManager {
public:
    void placeGlider(Grid &grid, int x, int y) {
        grid.getCell(x+1,y).setAlive(true);
        grid.getCell(x+2,y+1).setAlive(true);
        grid.getCell(x,y+2).setAlive(true);
        grid.getCell(x+1,y+2).setAlive(true);
        grid.getCell(x+2,y+2).setAlive(true);
    }

    void place_block(Grid &grid, int x, int y){
        grid.getCell(x, y).setAlive(true);
        grid.getCell(x+1, y).setAlive(true);
        grid.getCell(x, y-1).setAlive(true);
        grid.getCell(x+1, y-1).setAlive(true);
    }

    void placeVaisseau(Grid &grid, int x, int y){
        grid.getCell(x, y-1).setAlive(true);
        grid.getCell(x+1, y).setAlive(true);
        grid.getCell(x+2, y).setAlive(true);
        grid.getCell(x+3, y).setAlive(true);
        grid.getCell(x+4, y).setAlive(true);
        grid.getCell(x+5, y).setAlive(true);
        grid.getCell(x+5, y-1).setAlive(true);
        grid.getCell(x+5, y-2).setAlive(true);
        grid.getCell(x+4, y-3).setAlive(true);
    }
};

// -------------------- Classe InputHandler --------------------
class InputHandler {
public:
    void processEvents(sf::RenderWindow &window, Grid &grid, PatternManager &patterns, bool &running) {
        sf::Event event;
        sf::Vector2i mouse = sf::Mouse::getPosition(window);
        int cellX = mouse.x / Grid::CELL_SIZE;
        int cellY = mouse.y / Grid::CELL_SIZE;

        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) window.close();
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) window.close();

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
                grid.getCell(cellX, cellY).setAlive(true);
            if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
                grid.getCell(cellX, cellY).setAlive(false);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::O))
                grid.getCell(cellX, cellY).toggleObstacle();

            if (event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Space)
                running = !running;

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::G))
                patterns.placeGlider(grid, cellX, cellY);

            
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::B)) {
                patterns.place_block(grid, cellX, cellY);
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
                running = false;
                grid.clear();
            }

            if (sf::Keyboard::isKeyPressed(sf::Keyboard::V)) {
                patterns.placeVaisseau(grid, cellX, cellY);
            }
        }
    }
};

// -------------------- Classe Renderer --------------------
class Renderer {
public:
    void render(sf::RenderWindow &window, Grid &grid) {
        window.clear();
        grid.draw(window);
        window.display();
    }
};

// -------------------- Classe GameOfLifeApp --------------------
class GameOfLifeApp {
private:
    Grid grid;
    PatternManager patterns;
    InputHandler input;
    Renderer renderer;
    sf::RenderWindow window;
    bool running = false;

public:
    GameOfLifeApp() : window(sf::VideoMode(Grid::WIDTH * Grid::CELL_SIZE + 4,
                                           Grid::HEIGHT * Grid::CELL_SIZE + 4),
                                "Game Of Life") {
        window.setFramerateLimit(60);
    }

    void run() {
        while (window.isOpen()) {
            input.processEvents(window, grid, patterns, running);
            if (running) grid.update();
            renderer.render(window, grid);
        }
    }
};

// -------------------- main --------------------
int main() {
    GameOfLifeApp app;
    app.run();
    return 0;
}
