#include <cmath>
#include <iostream>

#include "imgui-SFML.h"
#include "imgui.h"
#include <SFML/Graphics.hpp>

#include <functional>
#include <vector>
#include <thread>
#include <omp.h>
float determinant3x3(const std::vector<std::vector<float>>& matrix) {
    if (matrix.size() != 3 || matrix[0].size() != 3 || matrix[1].size() != 3 || matrix[2].size() != 3) {
        throw std::runtime_error("Matrix dimensionality is not 3x3");
    }

    return
        matrix[0][0] * (matrix[1][1] * matrix[2][2] - matrix[1][2] * matrix[2][1]) -
        matrix[0][1] * (matrix[1][0] * matrix[2][2] - matrix[1][2] * matrix[2][0]) +
        matrix[0][2] * (matrix[1][0] * matrix[2][1] - matrix[1][1] * matrix[2][0]);;
}

sf::Color interpolateColors(const sf::Color& color1, const sf::Color& color2, float t) {
    float r = color1.r + (color2.r - color1.r) * t;
    float g = color1.g + (color2.g - color1.g) * t;
    float b = color1.b + (color2.b - color1.b) * t;
    float a = color1.a + (color2.a - color1.a) * t;

    return sf::Color(static_cast<sf::Uint8>(r), static_cast<sf::Uint8>(g), static_cast<sf::Uint8>(b), static_cast<sf::Uint8>(a));
}

class RFuncSprite : public sf::Sprite
{
public:
    void HandleEvent(sf::Event& event, sf::RenderWindow &window)
    {
        if(event.type == sf::Event::MouseButtonPressed)
        {
            int normalType = 2*(sf::Mouse::getPosition(window).y / (window.getSize().y/2)) + (sf::Mouse::getPosition(window).x / (window.getSize().x/2)) + 1;
            if((sf::Mouse::getPosition(window).x < window.getSize().x) && (sf::Mouse::getPosition(window).y < window.getSize().y))
            {
                auto a = sf::Mouse::getPosition(window).y % window.getSize().y;
                gradientDescent(sf::Mouse::getPosition(window).x % (window.getSize().x/2), sf::Mouse::getPosition(window).y % (window.getSize().y/2), normalType, window.getSize().y/2);
            }
        }
    }

    void Create(const sf::Vector2u& size, const int selectedNormalIndex)
    {

        vnx.resize(size.x);
        vny.resize(size.x);
        vnz.resize(size.x);
        vnw.resize(size.x);

        for (int i = 0; i < size.x; i++) {
            vnx[i].resize(size.x);
            vny[i].resize(size.x);
            vnz[i].resize(size.x);
            vnw[i].resize(size.x);
        }



        _image.create(size.x, size.y, sf::Color::Cyan);
        imageNx.create(size.x, size.y, sf::Color::Cyan);
        imageNy.create(size.x, size.y, sf::Color::Cyan);
        imageNz.create(size.x, size.y, sf::Color::Cyan);
        imageNw.create(size.x, size.y, sf::Color::Cyan);
        _texture.loadFromImage(_image);
        _textureNx.loadFromImage(imageNx);
        _textureNy.loadFromImage(imageNy);
        _textureNz.loadFromImage(imageNz);
        _textureNw.loadFromImage(imageNw);
        setTexture(_texture);
        NWSprite.setTexture(_textureNw);
        NZSprite.setTexture(_textureNz);
        NYSprite.setTexture(_textureNy);
        NXSprite.setTexture(_textureNx);
        NYSprite.setPosition(size.x, 0);
        NZSprite.setPosition(0, size.y);
        NWSprite.setPosition(size.x, size.y);
        NXSprite.setPosition(0, 0);
        _firstColor = sf::Color::Black;
        _secondColor = sf::Color::White;

        _selectedNormalIndex = selectedNormalIndex;
    }

    void DrawRFunc(const std::function<float(const sf::Vector2f&)>& rfunc, const sf::FloatRect& subSpace)
    {
        sf::Vector2f spaceStep = {
            subSpace.width / static_cast<float>(_image.getSize().x),
            subSpace.height / static_cast<float>(_image.getSize().y)
        };

   #pragma omp parallel for
        for (int x = 0; x < _image.getSize().x - 1; ++x)
        {
            for (int y = 0; y < _image.getSize().y - 1; ++y)
            {
                sf::Vector2f spacePoint1 = {
                    subSpace.left + static_cast<float>(x) * spaceStep.x,
                    subSpace.top + static_cast<float>(y) * spaceStep.y
                };

                const float z1 = rfunc(spacePoint1);

                sf::Vector2f spacePoint2 = {
                    subSpace.left + static_cast<float>(x + 1) * spaceStep.x,
                    subSpace.top + static_cast<float>(y) * spaceStep.y
                };

                const float z2 = rfunc(spacePoint2);

                sf::Vector2f spacePoint3 = {
                    subSpace.left + static_cast<float>(x) * spaceStep.x,
                    subSpace.top + static_cast<float>(y + 1) * spaceStep.y
                };

                const float z3 = rfunc(spacePoint3);

                const float A = determinant3x3({
                    {spacePoint1.y, z1, 1},
                    {spacePoint2.y, z2, 1},
                    {spacePoint3.y, z3, 1},
                    });

                const float B = determinant3x3({
                    {spacePoint1.x, z1, 1},
                    {spacePoint2.x, z2, 1},
                    {spacePoint3.x, z3, 1},
                    });

                const float C = determinant3x3({
                    {spacePoint1.x, spacePoint1.y, 1},
                    {spacePoint2.x, spacePoint2.y, 1},
                    {spacePoint3.x, spacePoint3.y, 1},
                    });

                const float D = determinant3x3({
                    {spacePoint1.x, spacePoint1.y, z1},
                    {spacePoint2.x, spacePoint2.y, z2},
                    {spacePoint3.x, spacePoint3.y, z3},
                    });

                const float lenPv = std::sqrt(A * A + B * B + C * C + D * D);

                float nx = A / lenPv;
                float ny = B / lenPv;
                float nz = C / lenPv;
                float nw = D / lenPv;
                vnx[x][y]=nx;
                vny[x][y]=ny;
                vnz[x][y]=nz;
                vnw[x][y]=nw;
                auto pixelColor = interpolateColors(_firstColor, _secondColor, (1.f + nx) / 2);
                if(z1>0){
                    _image.setPixel(x, y, sf::Color::Red);
                }

                pixelColor = interpolateColors(_firstColor, _secondColor, (1.f + nx) / 2);

                imageNx.setPixel(x, y, pixelColor);

                pixelColor = interpolateColors(_firstColor, _secondColor, (1.f + ny) / 2);

                imageNy.setPixel(x, y, pixelColor);

                pixelColor = interpolateColors(_firstColor, _secondColor, (1.f + nz) / 2);

                imageNz.setPixel(x, y, pixelColor);

                pixelColor = interpolateColors(_firstColor, _secondColor, (1.f + nw) / 2);

                imageNw.setPixel(x, y, pixelColor);
            }
        }

        _texture.update(_image);
        _textureNx.update(imageNx);
        _textureNy.update(imageNy);
        _textureNz.update(imageNz);
        _textureNw.update(imageNw);
    }


    void SaveImageToFile()
    {
        imageNx.saveToFile("nx.png");
        imageNy.saveToFile("ny.png");
        imageNz.saveToFile("nz.png");
        imageNw.saveToFile("nw.png");

    }

    void draw(sf::RenderWindow& window)
    {
        NXSprite.setTexture(_textureNx);
        NYSprite.setTexture(_textureNy);
        NZSprite.setTexture(_textureNz);
        NWSprite.setTexture(_textureNw);
        window.draw(NWSprite);
        window.draw(NZSprite);
        window.draw(NYSprite);
        window.draw(NXSprite);
        window.draw(ZSprite);
        for(const auto &g: gradLines)
        {
            window.draw(*g);
        }
    }

    void gradientDescent(unsigned int x, unsigned int y, int normalType, int _size)
    {

        std::vector<sf::Vector2u> dots;
        sf::Image*  gradLineImage = new sf::Image();
        gradLineImage->create(2*_size, 2*_size, sf::Color::Transparent);
        sf::Texture*  gradLineTexture = new sf::Texture();
        gradLineTexture->loadFromImage(*gradLineImage);
        sf::Sprite* gradLineSprite = new sf::Sprite();



        for (int iteration = 0; iteration < 1000; ++iteration) {
            double gradientX, gradientY;
            sf::Vector2u dot = {x,y};

            computeGradient(x, y, gradientX, gradientY, normalType, _size);




            if(std::abs(learning_rate * gradientX) < 1 && std::abs(learning_rate * gradientY)<1){
                x -= learning_rate * gradientX*5;
                y -= learning_rate * gradientY*5;
            }
            else
            {
                x -= learning_rate * gradientX;
                y -= learning_rate * gradientY;
            }
            gradLineImage->setPixel(x+((normalType+1)%2)*(_size),y+((normalType-1)/2)*(_size),sf::Color::Red);


            if (std::find(dots.begin(), dots.end(), dot) != dots.end()) {
                gradLineTexture->loadFromImage(*gradLineImage);
                gradLineSprite->setTexture(*gradLineTexture);
                gradLines.push_back(gradLineSprite);
                break;
            }
            dots.push_back(dot);

        }



    }
    void computeGradient(int x, int y, double& gradientX, double& gradientY, int normalType, int _size)
    {
        double a,b,c,d;
        if (x > 0 && x < _size - 1 && y > 0 && y < _size - 1) {
            switch(normalType)
            {
            case 1:
                a = vnx[x + 1][y];
                b = vnx[x - 1][y];
                c = vnx[x][y + 1];
                d = vnx[x][y - 1];
                break;
            case 2:
                a = vny[x + 1][y];
                b = vny[x - 1][y];
                c = vny[x][y + 1];
                d = vny[x][y - 1];
                break;
            case 3:
                a = vnz[x + 1][y];
                b = vnz[x - 1][y];
                c = vnz[x][y + 1];
                d = vnz[x][y - 1];
                break;
            case 4:
                a = vnw[x + 1][y];
                b = vnw[x - 1][y];
                c = vnw[x][y + 1];
                d = vnw[x][y - 1];
                break;
            }
            gradientX = (a - b) / 2.0;
            gradientY = (c - d) / 2.0;
            } else {
                gradientX = 0.0;
                gradientY = 0.0;
            }
    }
    std::vector<sf::Sprite*> sprites;
    void gradClear()
    {
        gradLines.clear();
    }
private:
    sf::Color _firstColor;
    sf::Color _secondColor;
    sf::Sprite NXSprite;
    sf::Sprite NYSprite;
    sf::Sprite NZSprite;
    sf::Sprite NWSprite;
    sf::Sprite ZSprite;
    sf::Texture _texture;
    sf::Texture _textureNx;
    sf::Texture _textureNy;
    sf::Texture _textureNz;
    sf::Texture _textureNw;
    sf::Image imageNx;
    sf::Image imageNy;
    sf::Image imageNz;
    sf::Image imageNw;
    double learning_rate = 100;
    sf::Image _image;
    sf::Image gradient;
    sf::Texture gradTexture;
    sf::Sprite gradSprite;
    int _selectedNormalIndex;
    std::vector<std::vector<double>> vnx;
    std::vector<std::vector<double>> vny;
    std::vector<std::vector<double>> vnz;
    std::vector<std::vector<double>> vnw;
    std::vector<sf::Sprite*> gradLines;
};

float RAnd(float w1, float w2) {
    return w1 + w2 - std::sqrt(w1*w1+w2*w2);
}

float ROr(float w1, float w2) {
    return w1+w2+std::sqrt(w1*w1+w2*w2);
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(400, 400), "Lab 3");
    window.setFramerateLimit(60);
    if (!ImGui::SFML::Init(window))
    {
        std::cout << "ImGui initialization failed\n";
        return -1;
    }

    auto spriteSize = sf::Vector2u{ window.getSize().x / 2, window.getSize().y / 2 };

    RFuncSprite rFuncSprite1;
    rFuncSprite1.Create(spriteSize, 0);



    std::function<float(const sf::Vector2f&)> rFunctions[5];

    rFunctions[0] = [](const sf::Vector2f& point) -> float {
        return 9-(point.x-1) * (point.x-1) - point.y * point.y;
        };
    rFunctions[1] = [](const sf::Vector2f& point) -> float {
        return 1-(point.x) * (point.x) - (point.y+1) * (point.y+1);
        };
    rFunctions[2] = [](const sf::Vector2f& point) -> float {
        return 1-(point.x+1) * (point.x+1) - point.y * point.y;
        };
    rFunctions[3] = [](const sf::Vector2f& point) -> float {
        return 1 - (point.x) * (point.x) - (point.y-1) * (point.y-1) ;
        };
    rFunctions[4] = [](const sf::Vector2f& point) -> float {
        return 1 - point.x * point.x - point.y * point.y ;
        };

    std::function<float(const sf::Vector2f&)> complexFunction = [&rFunctions](const sf::Vector2f& point) -> float {
        return  rFunctions[4](point);
        };


    sf::FloatRect subSpace(-10.f, -10.f, 20.f, 20.f);

    static ImVec4 firstColor(0, 0, 0, 1);
    static ImVec4 secondColor(1, 1, 1, 1);

    rFuncSprite1.DrawRFunc(complexFunction, subSpace);



    sf::Clock deltaClock;

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            ImGui::SFML::ProcessEvent(event);

            rFuncSprite1.HandleEvent(event, window);

            if (event.type == sf::Event::Closed)
            {
                window.close();
            }
        }

        ImGui::SFML::Update(window, deltaClock.restart());

        ImGui::Begin("Controls");

        if (ImGui::Button("Save Image"))
        {
            rFuncSprite1.SaveImageToFile();
        }

        if(ImGui::Button("Clear"))
        {
            rFuncSprite1.gradClear();
        }

        ImGui::End();

        window.clear();

        rFuncSprite1.draw(window);

        ImGui::SFML::Render(window);

        window.display();
    }

    ImGui::SFML::Shutdown();

    return 0;
}
