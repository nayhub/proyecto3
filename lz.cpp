//#include <string>
//#include <vector>
//#include <unordered_map>

// aquí va el codigo del LZ. lo de arriba son las bibliotecas que recomiendo :D pero pueden usar lo que quieran !!!

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <bitset>
#include <chrono>

struct Token {
    int offset;
    int length;
    char nextChar;
};

// Función para leer el contenido de un archivo en una cadena
std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "No se pudo abrir el archivo: " << filename << std::endl;
        exit(1);
    }

    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    return content;
}

// Función para escribir tokens comprimidos en un archivo binario
void writeCompressedFile(const std::string& filename, const std::vector<Token>& compressed) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "No se pudo abrir el archivo: " << filename << std::endl;
        exit(1);
    }

    for (const auto& token : compressed) {
        uint16_t offset = static_cast<uint16_t>(token.offset);
        uint8_t length = static_cast<uint8_t>(token.length);
        file.write(reinterpret_cast<const char*>(&offset), sizeof(offset));
        file.write(reinterpret_cast<const char*>(&length), sizeof(length));
        file.write(&token.nextChar, sizeof(token.nextChar));
    }

    file.close();
}

// Función para leer tokens comprimidos desde un archivo binario
std::vector<Token> readCompressedFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "No se pudo abrir el archivo: " << filename << std::endl;
        exit(1);
    }

    std::vector<Token> compressed;
    while (file.peek() != EOF) {
        Token token;
        uint16_t offset;
        uint8_t length;
        file.read(reinterpret_cast<char*>(&offset), sizeof(offset));
        file.read(reinterpret_cast<char*>(&length), sizeof(length));
        file.read(&token.nextChar, sizeof(token.nextChar));
        token.offset = static_cast<int>(offset);
        token.length = static_cast<int>(length);
        compressed.push_back(token);
    }

    file.close();
    return compressed;
}

// Función para realizar la compresión LZ77
std::vector<Token> compressLZ77(const std::string& input, int searchBufferSize) {
    std::vector<Token> compressed;
    int inputSize = input.size();

    for (int searchBufferEnd = 0; searchBufferEnd < inputSize; ) {
        int matchLength = 0;
        int matchDistance = 0;

        // Buscar la mejor coincidencia en el búfer de búsqueda
        for (int i = std::max(0, searchBufferEnd - searchBufferSize); i < searchBufferEnd; ++i) {
            int j = 0;
            while (j < inputSize - searchBufferEnd && input[i + j] == input[searchBufferEnd + j]) {
                ++j;
            }
            if (j > matchLength) {
                matchLength = j;
                matchDistance = searchBufferEnd - i;
            }
        }

        char nextChar = (searchBufferEnd + matchLength < inputSize) ? input[searchBufferEnd + matchLength] : '\0';
        compressed.push_back({matchDistance, matchLength, nextChar});

        searchBufferEnd += matchLength + 1;
    }

    return compressed;
}

// Función para realizar la descompresión LZ77
std::string decompressLZ77(const std::vector<Token>& compressed) {
    std::string decompressed;

    for (const auto& token : compressed) {
        int start = decompressed.size() - token.offset;
        for (int i = 0; i < token.length; ++i) {
            decompressed += decompressed[start + i];
        }
        if (token.nextChar != '\0') {
            decompressed += token.nextChar;
        }
    }

    return decompressed;
}

int main() {
    std::string nombreArchivo = "texto.txt";

    // Leer el archivo
    std::string texto = readFile(nombreArchivo);

    // Inicializar el reloj
    auto inicio = std::chrono::high_resolution_clock::now();

    // Realizar la compresión
    std::vector<Token> compressed = compressLZ77(texto, 4096);

    // Detener el reloj
    auto fin = std::chrono::high_resolution_clock::now();

    // Calcular tiempo de compresión
    auto duracion = std::chrono::duration_cast<std::chrono::milliseconds>(fin - inicio);
    std::cout << "Tiempo de compresion: " << duracion.count() << " ms" << std::endl;

    // Guardar el texto comprimido como binario
    writeCompressedFile("archivo_comprimido.txt", compressed);

    // Leer el archivo comprimido
    std::vector<Token> compressedFromFile = readCompressedFile("archivo_comprimido.txt");

    // Descomprimir para verificar
    std::string textoDescomprimido = decompressLZ77(compressedFromFile);
    std::ofstream archivoDescomprimido("decompressed.txt");
    archivoDescomprimido << textoDescomprimido;
    archivoDescomprimido.close();

    return 0;
}
