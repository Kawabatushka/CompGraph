#include <SFML/Graphics.hpp>
#include <memory>
#include "SFML/Graphics/RectangleShape.hpp"
#include "SFML/Graphics/RenderWindow.hpp"
#include "SFML/Graphics/Text.hpp"
static int SIZE = 3;
enum class Player { None, Cross, Circle };

class RestartWindow
{
private:
    sf::Text *text;
    sf::Font font;
    sf::RectangleShape windowBg;
    bool isVisible = 1;
public:
    RestartWindow()
    {

        if (!font.loadFromFile("arial.ttf"))
        {
            // Error...
        }
        text = new sf::Text();
        text->setString("PLAY");
        text->setFont(font);
        text->setPosition(280, 328);
        text->setCharacterSize(48);
        text->setFillColor(sf::Color::Blue);
        windowBg = sf::RectangleShape(sf::Vector2f(750, 300));
        windowBg.setFillColor(sf::Color::Black);
        windowBg.setPosition(0,225);
    }
    void draw(sf::RenderWindow& window)
    {
        if(isVisible)
        {
            window.draw(windowBg);
            window.draw(*text);
        }
    }
    void handleEvent(sf::Event& event)
    {
        if(isVisible){
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                int mouseX = event.mouseButton.x;
                int mouseY = event.mouseButton.y;

                if (mouseX >= windowBg.getPosition().x && mouseX < windowBg.getPosition().x + windowBg.getSize().x && mouseY >= windowBg.getPosition().y && mouseY < windowBg.getPosition().y + windowBg.getSize().y) {
                    isVisible = 0;
                }
            }
        }
    }
    bool getVisible()
    {
        return isVisible;
    }
    void setVisible(bool visibility)
    {
        isVisible = visibility;
    }
    sf::Text* getText()
    {
        return text;
    }
};

class TicTacToe
{
private:
    std::vector<std::vector<Player>> board;
    Player player;
    RestartWindow* restartWindow;
public:
    TicTacToe(){
        board.resize(SIZE, std::vector<Player>(SIZE, Player::None));
        restartWindow = new RestartWindow();
    }
    void draw(sf::RenderWindow& window)
    {
        window.clear();
        for(int i = 0; i < SIZE; i++)
        {
            for(int j = 0; j < SIZE; j++)
            {
                sf::RectangleShape cell(sf::Vector2f(250, 250));
                cell.setPosition(j * 250, i * 250);
                cell.setOutlineThickness(2);
                cell.setOutlineColor(sf::Color::Black);
                window.draw(cell);
                if (board[j][i] == Player::Cross) {
                    sf::VertexArray cross(sf::Lines, 4);
                    cross[0].position = sf::Vector2f(j * 250 + 10, i * 250 + 10);
                    cross[1].position = sf::Vector2f((j + 1) * 250 - 10, (i + 1) * 250 - 10);
                    cross[2].position = sf::Vector2f((j + 1) * 250 - 10, i * 250 + 10);
                    cross[3].position = sf::Vector2f(j * 250 + 10, (i + 1) * 250 - 10);
                    cross[0].color = sf::Color::Black;
                    cross[1].color = sf::Color::Black;
                    cross[2].color = sf::Color::Black;
                    cross[3].color = sf::Color::Black;
                    window.draw(cross);
                } else if (board[j][i] == Player::Circle) {
                    sf::CircleShape circle(250 / 2 - 10);
                    circle.setPosition(j * 250 + 10, i * 250 + 10);
                    circle.setOutlineThickness(2);
                    circle.setOutlineColor(sf::Color::Black);
                    circle.setFillColor(sf::Color::Transparent);
                    window.draw(circle);
                }
            }
        }
        restartWindow->draw(window);

    }
    void handleEvent(sf::Event& event)
    {
        if(restartWindow->getVisible() == false){
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                int mouseX = event.mouseButton.x / 250;
                int mouseY = event.mouseButton.y / 250;

                if (mouseX >= 0 && mouseX < SIZE && mouseY >= 0 && mouseY < SIZE && board[mouseX][mouseY] == Player::None) {

                    board[mouseX][mouseY] = player;
                    if(checkWin(mouseX, mouseY) != Player::None)
                    {
                        restart();
                        restartWindow->setVisible(true);
                        restartWindow->getText()->setString((player == Player::Cross) ? "CROSS WINS" : "CIRCLE WINS");
                        player = Player::Circle;
                    }
                    player = (player == Player::Cross) ? Player::Circle : Player::Cross;

                }
            }
        }
         restartWindow->handleEvent(event);
    }
    void restart()
    {
        for(int i = 0; i < SIZE; i++)
        {
            for(int j = 0; j < SIZE; j++)
            {
                board[i][j] = Player::None;

            }
        }

    }
    Player checkWin(int row, int col) {
        for (int j = 0; j < SIZE; ++j) {
            if (board[row][j] != player) {
                break;
            }
            if (j == SIZE - 1) {
                return player;
            }
        }
        for (int i = 0; i < SIZE; ++i) {
            if (board[i][col] != player) {
                break;
            }
            if (i == SIZE - 1) {
                return player;
            }
        }
        if (row == col) {
            for (int i = 0; i < SIZE; ++i) {
                if (board[i][i] != player) {
                    break;
                }
                if (i == SIZE - 1) {
                    return player;
                }
            }
        }
        if (row + col == SIZE - 1) {
            for (int i = 0; i < SIZE; ++i) {
                if (board[i][SIZE - 1 - i] != player) {
                    break;
                }
                if (i == SIZE - 1) {
                    return player;
                }
            }
        }
        return Player::None;
    }
};


int main() {

    sf::RenderWindow window(sf::VideoMode(SIZE * 250, SIZE * 250), "Tic Tac Toe", sf::Style::Titlebar | sf::Style::Close);
    TicTacToe* game = new TicTacToe();
    RestartWindow* restart = new RestartWindow;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            game->handleEvent(event);
        }
        game->draw(window);
        window.display();
    }

    return 0;
}
