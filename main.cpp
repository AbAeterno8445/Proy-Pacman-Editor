#include <SFML/Graphics.hpp>
#include <math.h>
#include <vector>
#include <fstream>

using namespace std;

class Bloque {
	vector<sf::Texture>* texture;
	sf::Sprite sprite;

	sf::RenderWindow* window;

	int draw_x;
	int draw_y;

	int sprite_id;

public:
	//Constructor
	Bloque(sf::RenderWindow* window_param, vector<sf::Texture>* texture_param) {
		window = window_param;

		texture = texture_param;
		sprite.setTexture((*texture)[0]);

		sprite_id = 0;

		draw_x = 0;
		draw_y = 0;
	}

	int get_drawx() { return draw_x; }
	int get_drawy() { return draw_y; }
	int get_spriteid() { return sprite_id; }

	void set_spriteid(int id) {
		sprite_id = id;

		sprite.setTextureRect(sf::IntRect(sf::IntRect(32 * (sprite_id % 10), 32 * floor(sprite_id / 10), 32, 32)));
	}

	void set_textureid(int id) {
		sprite.setTexture((*texture)[id]);
	}

	void set_drawposition(int dx, int dy) {
		draw_x = dx;
		draw_y = dy;

		sprite.setPosition(draw_x, draw_y);
	}

	void dibujar() {
		window->draw(sprite);
	}

	bool clicked(int x, int y) {
		if (x > draw_x - 16 && x < draw_x + 16 &&
			y > draw_y - 16 && y < draw_y + 16) {
				return true;
		}

		return false;
	}
};

void cargar_nivel(string filename, vector<int>& matriz_nivel, int& tam_x, int& tam_y) {
	string ruta = "mapas/" + filename;

	fstream mapfile;
	mapfile.open(ruta);

	if (mapfile.is_open()) {
		string sub_line;
		string tam_x_str, tam_y_str;
		getline(mapfile, tam_x_str);
		getline(mapfile, tam_y_str);

		tam_x = atoi(tam_x_str.c_str());
		tam_y = atoi(tam_y_str.c_str());

		while(getline(mapfile, sub_line, ',')) {
			matriz_nivel.push_back(atoi(sub_line.c_str()));
		}

		mapfile.close();
	}
}

void guardar_nivel(string filename, vector<int>& matriz_nivel, int tam_x, int tam_y) {
    fstream mapfile;

    string ruta = "mapas/" + filename;
    mapfile.open(ruta, ios::out|ios::trunc);

    if (mapfile.is_open()) {
        mapfile << tam_x << endl;
        mapfile << tam_y << endl;

        for (int i = 0; i < tam_x; i++) {
            for (int j = 0; j < tam_y; j++) {
                mapfile << matriz_nivel[j + i * tam_y] << ",";
            }
            mapfile << endl;
        }

        mapfile.close();
    }
}

int main()
{
	// Ventana
    sf::RenderWindow window(sf::VideoMode(1280, 720), "PACMAN Editor");
    window.setFramerateLimit(60);

    // Texturas
    vector<sf::Texture> textures;

    // Textura paredes
    sf::Texture walls_texture;
    walls_texture.loadFromFile("assets/walls.png");
    textures.push_back(walls_texture);

    // Textura personajes
    sf::Texture charset_texture;
    charset_texture.loadFromFile("assets/charset.png");
    textures.push_back(charset_texture);

    // Textura fantasmas
    sf::Texture ghosts_texture;
    ghosts_texture.loadFromFile("assets/charset_fantasmas.png");
    textures.push_back(ghosts_texture);

    // Nivel
    string nombre_nivel = "mapa1.txt";

    vector<int> matriz_nivel; // Matriz

    // Offset dibujado del nivel
    int niv_draw_xoff = 352;
    int niv_draw_yoff = 0;

    // Tamaño nivel
    int nivel_tam_x = 0;
    int nivel_tam_y = 0;

    cargar_nivel(nombre_nivel, matriz_nivel, nivel_tam_x, nivel_tam_y);

    // Bloques (paleta)
    vector<Bloque> bloques_paleta;
    int bloque_selec = 0;

    // Bloques (nivel)
    vector<Bloque> bloques_nivel;

    for (int i = 0; i < nivel_tam_x; i++) {
		for (int j = 0; j < nivel_tam_y; j++) {
			Bloque bloque_temp(&window, &textures);

			switch(matriz_nivel[i + j * nivel_tam_x]) {
			case 69: // Pacman
				bloque_temp.set_textureid(1);
				bloque_temp.set_spriteid(11);
				break;

			default:
				bloque_temp.set_spriteid(matriz_nivel[i + j * nivel_tam_x]);
				break;
			}

			bloque_temp.set_drawposition(i * 32 + niv_draw_xoff, j * 32 + niv_draw_yoff);

			bloques_nivel.push_back(bloque_temp);
		}
    }

    // Rectangulo de seleccion
    sf::RectangleShape selec_rect;
    selec_rect.setSize(sf::Vector2f(32, 32));
    selec_rect.setFillColor(sf::Color::Transparent);
    selec_rect.setOutlineColor(sf::Color::Yellow);
    selec_rect.setOutlineThickness(1);

    for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 10; j++) {

			if (i == 1) {
				if (j >= 8) continue;
			}

			Bloque bloque_temp(&window, &textures);

			bloque_temp.set_drawposition(j * 32, i * 32);
			bloque_temp.set_spriteid(j + i * 10);

			bloques_paleta.push_back(bloque_temp);
		}
    }

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();

            if (event.type == sf::Event::KeyPressed) {
                switch(event.key.code) {
                case sf::Keyboard::S:
                    guardar_nivel(nombre_nivel, matriz_nivel, nivel_tam_x, nivel_tam_y);
                    break;

                default: break;
                }
            }

			if (event.type == sf::Event::MouseButtonPressed) {
				if (event.mouseButton.button == sf::Mouse::Left) {
					int mouse_x = event.mouseButton.x - 16;
					int mouse_y = event.mouseButton.y - 16;

					if (mouse_x < 356) {
                        for (unsigned int i = 0; i < bloques_paleta.size(); i++) {
                            if (bloques_paleta[i].clicked(mouse_x, mouse_y)) {
                                bloque_selec = i;

                                selec_rect.setPosition(bloques_paleta[i].get_drawx(), bloques_paleta[i].get_drawy());
                                break;
                            }
                        }
					} else {
                        for (unsigned int i = 0; i < bloques_nivel.size(); i++) {
                            if (bloques_nivel[i].clicked(mouse_x, mouse_y)) {
                                bloques_nivel[i].set_spriteid(bloques_paleta[bloque_selec].get_spriteid());

                                int xx = (bloques_nivel[i].get_drawx() % 32) / 32;
                                int yy = (bloques_nivel[i].get_drawy() % 32) / 32;
                                matriz_nivel[xx + yy * nivel_tam_x] = bloques_paleta[bloque_selec].get_spriteid();
                                break;
                            }
                        }
					}
				}
			}
        }

        window.clear();

		for (unsigned int i = 0; i < bloques_paleta.size(); i++) {
			bloques_paleta[i].dibujar();
		}

		for (unsigned int i = 0; i < bloques_nivel.size(); i++) {
			bloques_nivel[i].dibujar();
		}

		window.draw(selec_rect);

        window.display();
    }

    return 0;
}
