#include <iostream>
#include <fstream>
#include <queue>
#include <unordered_map>
#include <vector>
#include <bitset>
#include <chrono>

// este es el código de huffman !!! funciona bien con archivos txt :D
// para compilar g++ huffman.cpp -o huffman en la terminal. el nombre del archivo se edita desde el main, no se pone en la terminal.

struct Nodo {
    char simbolo;
    int frecuencia;
    Nodo* izquierdo;
    Nodo* derecho;
};

Nodo* nuevoNodo(char simbolo, int frecuencia, Nodo* izquierdo, Nodo* derecho) {
    Nodo* nodo = new Nodo();
    nodo->simbolo = simbolo;
    nodo->frecuencia = frecuencia;
    nodo->izquierdo = izquierdo;
    nodo->derecho = derecho;
    return nodo;
}

struct comparar {
    bool operator()(Nodo* izquierdo, Nodo* derecho) {
        return izquierdo->frecuencia > derecho->frecuencia;
    }
};

void codificarAux(Nodo* nodo, std::string str, std::unordered_map<char, std::string> &codigoHuffman) {
    if (nodo == nullptr)
        return;

    if (!nodo->izquierdo && !nodo->derecho) {
        codigoHuffman[nodo->simbolo] = str;
    }

    codificarAux(nodo->izquierdo, str + "0", codigoHuffman);
    codificarAux(nodo->derecho, str + "1", codigoHuffman);
}

std::string codificar(Nodo* &raiz, std::string texto) {
    std::unordered_map<char, int> frecuencias;
    for (char simbolo : texto) {
        frecuencias[simbolo]++;
    }

    std::priority_queue<Nodo*, std::vector<Nodo*>, comparar> colaPrioridad;

    for (auto pair : frecuencias) {
        colaPrioridad.push(nuevoNodo(pair.first, pair.second, nullptr, nullptr));
    }

    while (colaPrioridad.size() != 1) {
        Nodo* izquierdo = colaPrioridad.top(); colaPrioridad.pop();
        Nodo* derecho = colaPrioridad.top(); colaPrioridad.pop();

        int suma = izquierdo->frecuencia + derecho->frecuencia;
        colaPrioridad.push(nuevoNodo('\0', suma, izquierdo, derecho));
    }

    raiz = colaPrioridad.top();

    std::unordered_map<char, std::string> codigoHuffman;
    codificarAux(raiz, "", codigoHuffman);

    std::string textoCodificado = "";
    for (char simbolo : texto) {
        textoCodificado += codigoHuffman[simbolo];
    }

    return textoCodificado;
}

std::string decodificar(Nodo* raiz, std::string textoCodificado) {
    std::string textoDecodificado = "";
    Nodo* nodo = raiz;
    for (char bit : textoCodificado) {
        if (bit == '0')
            nodo = nodo->izquierdo;
        else
            nodo = nodo->derecho;

        if (!nodo->izquierdo && !nodo->derecho) {
            textoDecodificado += nodo->simbolo;
            nodo = raiz;
        }
    }

    return textoDecodificado;
}

void arbolACadena(Nodo* nodo, std::string& cadena) {
    if (!nodo)
        return;

    if (!nodo->izquierdo && !nodo->derecho) {
        cadena += "1";
        cadena += nodo->simbolo;
    } else {
        cadena += "0";
    }

    arbolACadena(nodo->izquierdo, cadena);
    arbolACadena(nodo->derecho, cadena);
}

Nodo* cadenaAArbol(std::string& cadena, int& indice) {
    if (indice >= cadena.size())
        return nullptr;

    char bit = cadena[indice];
    indice++;

    if (bit == '1') {
        char simbolo = cadena[indice];
        indice++;
        return nuevoNodo(simbolo, 0, nullptr, nullptr);
    } else {
        Nodo* izquierdo = cadenaAArbol(cadena, indice);
        Nodo* derecho = cadenaAArbol(cadena, indice);
        return nuevoNodo('\0', 0, izquierdo, derecho);
    }
}

int main() {
    std::string nombreArchivo = "texto.txt";
    std::ifstream archivo(nombreArchivo);
    std::string texto((std::istreambuf_iterator<char>(archivo)), std::istreambuf_iterator<char>());
    archivo.close();

    Nodo* raiz;

    // inicializar el reloj
    auto inicio = std::chrono::high_resolution_clock::now();

    std::string textoCodificado = codificar(raiz, texto);

    // detener el reloj
    auto fin = std::chrono::high_resolution_clock::now();

    // calcular tiempo de codificación
    auto duracion = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio);
    std::cout << "Tiempo de codificacion: " << duracion.count() << " ms" << std::endl;

    // guarda el texto codificado como binarios
    std::ofstream archivoCodificado("archivo_codificado.txt", std::ios::binary);
    for (int i = 0; i < textoCodificado.size(); i += 8) {
        std::bitset<8> bits(textoCodificado.substr(i, 8));
        char byte = static_cast<char>(bits.to_ulong());
        archivoCodificado.write(&byte, sizeof(byte));
    }
    archivoCodificado.close();

    // guarla el huffman como archivo 
    std::string cadenaArbol;
    arbolACadena(raiz, cadenaArbol);
    std::ofstream archivoArbol("arbol_huffman.txt");
    archivoArbol << cadenaArbol;
    archivoArbol.close();

    // carga el huffman desde un archivo 
    std::ifstream archivoArbolIn("arbol_huffman.txt");
    std::string cadenaArbolIn((std::istreambuf_iterator<char>(archivoArbolIn)), std::istreambuf_iterator<char>());
    archivoArbolIn.close();
    int indice = 0;
    Nodo* raizIn = cadenaAArbol(cadenaArbolIn, indice);

    // carga el texto como datos binarios
    std::ifstream archivoCodificadoIn("archivo_codificado.txt", std::ios::binary);
    std::string textoCodificadoIn;
    char byte;
    while (archivoCodificadoIn.read(&byte, sizeof(byte))) {
        textoCodificadoIn += std::bitset<8>(byte).to_string();
    }
    archivoCodificadoIn.close();

    std::string textoDecodificado = decodificar(raizIn, textoCodificadoIn);

    std::ofstream archivoDecodificado("archivo_decodificado.txt");
    archivoDecodificado << textoDecodificado;
    archivoDecodificado.close();

    return 0;
}
