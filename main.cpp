#include <SFML/Graphics.hpp>
#include <math.h>
#include <vector>
#include <fstream>

using namespace std;

class Bloque {
	sf::Texture* texture;
	sf::Sprite sprite;

	sf::RenderWindow* window;

	int draw_x;
	int draw_y;

	int sprite_id;

public:
	//Constructor
	Bloque(sf::RenderWindow* window_param, sf::Texture* texture_param) {
		window = window_param;

		texture = texture_param;
		sprite.setTexture(*texture);

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
	string ruta = "mapas/" + filename + ".txt";

	fstream mapfile;
	mapfile.open(ruta);

	if (mapfile.is_open()) {
		matriz_nivel.clear();

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

    string ruta = "mapas/" + filename + ".txt";
    mapfile.open(ruta, ios::out|ios::trunc);

    if (mapfile.is_open()) {
        mapfile << tam_x << endl;
        mapfile << tam_y << endl;

        for (int i = 0; i < tam_y; i++) {
            for (int j = 0; j < tam_x; j++) {
                mapfile << matriz_nivel[j + i * tam_x] << ",";
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

    // Textura imagenes
    sf::Texture texture;
    texture.loadFromFile("assets/texture.png");

    // Nivel
    string nombre_nivel = "nivel_1";

    vector<int> matriz_nivel; // Matriz

    // Offset dibujado del nivel
    int niv_draw_xoff = 352;
    int niv_draw_yoff = 1;

    // Tamaño nivel
    int nivel_tam_x = 0;
    int nivel_tam_y = 0;

    cargar_nivel(nombre_nivel, matriz_nivel, nivel_tam_x, nivel_tam_y);

    // Bloques (paleta)
    vector<Bloque> bloques_paleta;
    int bloque_selec = 0;
    int bloque_selec_2 = 0;

    // Bloques (nivel)
    vector<Bloque> bloques_nivel;

    for (int i = 0; i < nivel_tam_x; i++) {
		for (int j = 0; j < nivel_tam_y; j++) {
			Bloque bloque_temp(&window, &texture);

			bloque_temp.set_spriteid(matriz_nivel[i + j * nivel_tam_x]);

			bloque_temp.set_drawposition(i * 32 + niv_draw_xoff, j * 32 + niv_draw_yoff);

			bloques_nivel.push_back(bloque_temp);
		}
    }

    // Rectangulo de seleccion primario
    sf::RectangleShape selec_rect;
    selec_rect.setSize(sf::Vector2f(32, 32));
    selec_rect.setFillColor(sf::Color::Transparent);
    selec_rect.setOutlineColor(sf::Color::Yellow);
    selec_rect.setOutlineThickness(1);

    // Rectangulo de seleccion secundario
    sf::RectangleShape selec_rect_2 = selec_rect;
    selec_rect_2.setOutlineColor(sf::Color::Cyan);

    // Rectangulo borde nivel
    sf::RectangleShape level_outline_rect;
    level_outline_rect.setSize(sf::Vector2f(nivel_tam_x * 32, nivel_tam_y * 32));
    level_outline_rect.setFillColor(sf::Color::Transparent);
    level_outline_rect.setOutlineColor(sf::Color::White);
    level_outline_rect.setOutlineThickness(1);
    level_outline_rect.setPosition(sf::Vector2f(niv_draw_xoff, niv_draw_yoff));

    // Inicializacion de la paleta de bloques
    for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 10; j++) {

			if (i == 1) {
				if (j >= 9) continue;
			}
			else if (i == 3) {
				if (j >= 5) continue;
			}

			Bloque bloque_temp(&window, &texture);

			bloque_temp.set_drawposition(j * 32, i * 32);
			bloque_temp.set_spriteid(j + i * 10);

			bloques_paleta.push_back(bloque_temp);
		}
    }

    // Info mouse
    int mouse_x, mouse_y;
    bool mouse_held = false;
    bool mouse_held_2 = false;

    // Texto guardado de nivel
    sf::Font font;
    font.loadFromFile("assets/fonts/monobit.ttf");

    sf::Text text_savelvl;
    text_savelvl.setFont(font);
    text_savelvl.setCharacterSize(48);
    text_savelvl.setFillColor(sf::Color::White);
    text_savelvl.setPosition(sf::Vector2f(12, window.getSize().y - 68));

    // Escribiendo
    bool inputting = false;
    bool input_start = false; // Necesario para evitar escribir letra extra en el nombre del nivel

    int input_state = 0; // 0 -> Guardado de nivel / 1 -> Cargado de nivel

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
        	switch(event.type) {

			// Evento -> Cerrar ventana
			case sf::Event::Closed:
                window.close();
				break;

			// Evento -> Apreto una tecla
            case sf::Event::KeyPressed:
                switch(event.key.code) {
				// S -> Guardar nivel
                case sf::Keyboard::S:
                	if (!inputting) {
						inputting = true;
						input_state = 0;
                	}
                    break;

				// L -> Cargar nivel
				case sf::Keyboard::L:
					if (!inputting) {
						inputting = true;
						input_state = 1;
					}
					break;

				// N -> Nuevo nivel
				case sf::Keyboard::N:
					if (!inputting) {
						for (int i = 0; i < nivel_tam_x; i++) {
							for (int j = 0; j < nivel_tam_y; j++) {
								matriz_nivel[i + j * nivel_tam_x] = 11;
								bloques_nivel[i + j * nivel_tam_x].set_spriteid(matriz_nivel[i + j * nivel_tam_x]);
							}
						}
					}
					break;

				// Enter -> Dejar de escribir
				case sf::Keyboard::Return:
					if (inputting) {
						inputting = false;
						input_start = false;

						if (input_state == 0) { // Guardar nivel
							guardar_nivel(nombre_nivel, matriz_nivel, nivel_tam_x, nivel_tam_y);
						}
						else if (input_state == 1) { // Cargar nivel
							cargar_nivel(nombre_nivel, matriz_nivel, nivel_tam_x, nivel_tam_y);

							level_outline_rect.setSize(sf::Vector2f(nivel_tam_x * 32, nivel_tam_y * 32));

							bloques_nivel.clear();
							for (int i = 0; i < nivel_tam_x; i++) {
								for (int j = 0; j < nivel_tam_y; j++) {
									Bloque bloque_temp(&window, &texture);

									bloque_temp.set_spriteid(matriz_nivel[i + j * nivel_tam_x]);

									bloque_temp.set_drawposition(i * 32 + niv_draw_xoff, j * 32 + niv_draw_yoff);

									bloques_nivel.push_back(bloque_temp);
								}
							}
						}
					}
					break;

                default: break;
                }
				break;

            // Evento -> Escribir texto
			case sf::Event::TextEntered:
				if (inputting) {
					if (input_start) {
						// Solo caracteres ASCII
						if (event.text.unicode < 128) {
							if (event.text.unicode == 8) { // Borrar
								if (nombre_nivel.size() > 0) nombre_nivel.resize(nombre_nivel.size() - 1);
							} else if (event.text.unicode != 13 && nombre_nivel.size() < 16) {
							nombre_nivel += static_cast<char>(event.text.unicode);
							}
						}
					} else {
						input_start = true;
					}
				}
				break;

            // Evento -> Movio el mouse
            case sf::Event::MouseMoved:
				if (window.hasFocus()) {
					mouse_x = event.mouseMove.x - 16;
					mouse_y = event.mouseMove.y - 16;
				}
				break;

            // Evento -> Click del mouse
            case sf::Event::MouseButtonPressed:
				if (event.mouseButton.button == sf::Mouse::Left) {
					mouse_held = true;
				}
				else if (event.mouseButton.button == sf::Mouse::Right) {
					mouse_held_2 = true;
				}
				break;

            // Evento -> Soltar click del mouse
            case sf::Event::MouseButtonReleased:
				if (event.mouseButton.button == sf::Mouse::Left) {
					mouse_held = false;
				}

				if (event.mouseButton.button == sf::Mouse::Right) {
					mouse_held_2 = false;
				}
				break;

            default: break;
        	}

            // Seleccion / Cambio de bloques
            if (mouse_held || mouse_held_2) {
                if (mouse_x < 340) {
                    for (unsigned int i = 0; i < bloques_paleta.size(); i++) {
                        if (bloques_paleta[i].clicked(mouse_x, mouse_y)) {
							if (mouse_held) { // Click Izquierdo - Cambiar seleccion primaria
								bloque_selec = i;
								selec_rect.setPosition(bloques_paleta[i].get_drawx(), bloques_paleta[i].get_drawy());
							}
							else if (mouse_held_2) { // Click Derecho - Cambiar seleccion secundaria
								bloque_selec_2 = i;
								selec_rect_2.setPosition(bloques_paleta[i].get_drawx(), bloques_paleta[i].get_drawy());
							}
                            break;
                        }
                    }
                } else {
                    for (unsigned int i = 0; i < bloques_nivel.size(); i++) {
                        if (bloques_nivel[i].clicked(mouse_x, mouse_y)) {
							int selec = 0;

							if (mouse_held) selec = bloque_selec;
							else if (mouse_held_2) selec = bloque_selec_2;

                            bloques_nivel[i].set_spriteid(bloques_paleta[selec].get_spriteid());

                            int xx = ((bloques_nivel[i].get_drawx() - niv_draw_xoff) / 32) % 32;
                            int yy = ((bloques_nivel[i].get_drawy() - niv_draw_yoff) / 32) % 32;

							matriz_nivel[xx + yy * nivel_tam_x] = bloques_paleta[selec].get_spriteid();
                            break;
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

		window.draw(selec_rect_2);
		window.draw(selec_rect);
		window.draw(level_outline_rect);

		text_savelvl.setString("Nivel: " + nombre_nivel);
		if (inputting) {
			text_savelvl.setFillColor((input_state == 0 ? sf::Color::Green : sf::Color::Cyan));
		} else {
			text_savelvl.setFillColor(sf::Color::White);
		}
		window.draw(text_savelvl);

        window.display();
    }

    return 0;
}
