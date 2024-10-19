#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <cmath>
# define M_PI   3.14159265358979323846

using namespace std;

// Estructura para representar una imagen PGM
struct Image {
    int width;
    int height;
    vector<vector<int>> pixels;
};

// Función para cargar una imagen PGM desde un archivo
Image loadPGM(const string& filename) {
    Image img;
    ifstream file(filename);

    if (!file) {
        cerr << "No se pudo abrir el archivo " << filename << endl;
        exit(1);
    }

    string format;
    string comment;
    getline(file, format);
    getline(file, comment);

    if (format != "P2") {
        cout << "El formato es: " << format << endl;
        cerr << "Formato de archivo no válido. Se esperaba P2." << endl;
        exit(1);
    }

    file >> img.width >> img.height;
    int max_val;
    file >> max_val;

    img.pixels.resize(img.height, vector<int>(img.width));

    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            file >> img.pixels[y][x];
        }
    }

    file.close();

    return img;
}

// Función para guardar una imagen PGM en un archivo
void savePGM(const Image& img, const string& filename) {
    ofstream file(filename);

    if (!file) {
        cerr << "No se pudo crear el archivo " << filename << endl;
        exit(1);
    }

    file << "P2" << endl;
    file << "#aaa" << endl;
    file << img.width << " " << img.height << endl;
    file << "255" << endl;

    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            file << img.pixels[y][x] << " ";
        }
        file << endl;
    }

    file.close();
}

vector<pair<int,int>> randomPoints(const Image& input){
    vector<pair<int,int>> whitePixels;

    for(int y = 0; y < input.height; y++){
        for(int x = 0; x < input.width; x++){
            if(input.pixels[y][x] == 255){
                whitePixels.push_back(make_pair(x,y));
            }
        }
    }
    if(whitePixels.size() < 3){
        cerr << "No hay suficientes pixeles blancos." << endl;
        exit(1);
    }

    vector<pair<int,int>> randomP;
    for(int i = 0; i<3; i++){
        int k = rand() % whitePixels.size();
        randomP.push_back(whitePixels[k]);
        whitePixels.erase(whitePixels.begin() + k);
    }

    return randomP;
}

double coeficienteA(vector<pair<int,int>> puntos){

    double a1 = puntos[2].second*(puntos[1].first - puntos[0].first);
    double a2 = puntos[1].second*(puntos[0].first - puntos[2].first);
    double a3 = puntos[0].second*(puntos[2].first - puntos[1].first);

    double den = (puntos[0].second - puntos[1].second)*(puntos[0].second - puntos[2].second)*(puntos[1].second - puntos[2].second);

    double A = (a1 + a2 + a3)/(den);

    return A;
}

double coeficienteB(vector<pair<int,int>> puntos){

    double b1 = pow(puntos[2].second,2)*(puntos[0].first - puntos[1].first);
    double b2 = pow(puntos[1].second,2)*(puntos[2].first - puntos[0].first);
    double b3 = pow(puntos[0].second,2)*(puntos[1].first - puntos[2].first);

    double den = (puntos[0].second - puntos[1].second)*(puntos[0].second - puntos[2].second)*(puntos[1].second - puntos[2].second);

    return (b1 + b2 + b3)/(den);
}

double coeficienteC(vector<pair<int,int>> puntos){

    double c1 = (puntos[1].second*puntos[2].second)*(puntos[1].second - puntos[2].second)*puntos[0].first;
    double c2 = (puntos[2].second*puntos[0].second)*(puntos[2].second - puntos[0].second)*puntos[1].first;
    double c3 = (puntos[0].second*puntos[1].second)*(puntos[0].second - puntos[1].second)*puntos[2].first;

    double den = (puntos[0].second - puntos[1].second)*(puntos[0].second - puntos[2].second)*(puntos[1].second - puntos[2].second);

    return (c1 + c2 + c3)/(den);
} 

int encontrarMaximo(const vector<int>& vec) {
    int k = 0;

    if (vec.empty()) {
        // Manejar el caso de un vector vacío
        cerr << "El vector está vacío." << endl;
        return -1; 
    }

    int maximo = vec[0]; // Suponemos que el primer elemento es el máximo inicialmente

    // Iterar sobre los elementos restantes del vector
    for (int i = 1; i < vec.size(); ++i) {
        if (vec[i] > maximo) {
            maximo = vec[i]; // Actualizar el máximo si encontramos un elemento mayor
            k = i;
        }
    }

    return k;
}

void guardarCoordenadas(const vector<pair<int,int>>& coordenadas, const string& filename) {
    ofstream file(filename);

    if (!file) {
        cerr << "No se pudo crear el archivo " << filename << endl;
        exit(1);
    }

    for (const auto& punto : coordenadas) {
        file << punto.first << " " << punto.second << endl;
    }

    file.close();
}

void guardarUltimosCoeficientes(double A, double B, double C, const string& filename) {
    ofstream file(filename);

    if (!file) {
        cerr << "No se pudo crear el archivo " << filename << endl;
        exit(1);
    }

    file << A << endl;
    file << B << endl;
    file << C << endl;

    file.close();
}

int main(){
    srand(time(nullptr)); // Inicializar la semilla aleatoria
    string folderPath = "/ImagenesBio/resultados/26_ske.pgm";
    string gtPath = "/ImagenesBio/26_gt.pgm";

    Image ske = loadPGM(folderPath);
    Image test = loadPGM(gtPath);

    vector<pair<int,int>> puntos;
    vector<pair<int,int>> porRevisar;
    vector<pair<int,int>> necesarios;
    double A, B, C;
    int iterations = 0;

    for(int y = 0; y < ske.height; y++){
        for(int x = 0; x < ske.width; x++){
            if(ske.pixels[y][x] == 255){
                porRevisar.push_back(make_pair(x,y));
            }
            if(test.pixels[y][x] == 255){
                necesarios.push_back(make_pair(x,y));
            }
        }
    }

    int n = porRevisar.size();

    //vector<int> cantidad;
    //vector<pair<int,int>> revisados;

    vector<int> cantidad(porRevisar.size(), 0); // Inicializa cantidad con el tamaño de porRevisar y valores 0
    vector<vector<pair<int,int>>> revisados(porRevisar.size());
    int k = 0;
    int coincidencias = 0;
    while(iterations < 1000){

        int contador = 0;
        puntos = randomPoints(ske);

        A = coeficienteA(puntos);
        B = coeficienteB(puntos);
        C = coeficienteC(puntos);

        for(int i = 0; i < n; i++){
            if(porRevisar[i].first == int(A*pow(porRevisar[i].second,2) + B*porRevisar[i].second + C)){
                contador++;
            }
            if(necesarios[i].first == int(A*pow(necesarios[i].second,2) + B*necesarios[i].second + C)){
                coincidencias++;
            }
        }

        cantidad[k] = contador;
        revisados[k] = puntos;
        k++;
        iterations++;

    }

    int posicionMaxima = encontrarMaximo(cantidad);
    // Después de encontrar la posición máxima
    guardarCoordenadas(revisados[posicionMaxima], "coordenadas_maximas.txt");

    cout << "La posición de mayor coincidencia es: " << posicionMaxima << endl;
    cout << " " << endl;

    cout << "La cantidad de puntos coincidentes es " << cantidad[posicionMaxima] << endl;
    // Imprimir el contenido de revisados en la posición máxima
    cout << "Puntos que generan la parábola con mayor coincidencias:" << endl;
    for (const auto& point : revisados[posicionMaxima]) {
        cout << "(" << point.first << ", " << point.second << ")" << endl;

    }

    cout << " " << endl;

    cout << "A = " << coeficienteA(revisados[posicionMaxima]) << endl;
    cout << "B = " << coeficienteB(revisados[posicionMaxima]) << endl;
    cout << "C = " << coeficienteC(revisados[posicionMaxima]) << endl;

    // Después de calcular A, B y C
    guardarUltimosCoeficientes(coeficienteA(revisados[posicionMaxima]), coeficienteB(revisados[posicionMaxima]), coeficienteC(revisados[posicionMaxima]), "ultimos_coeficientes.txt");
    int den = 0;
    for(int y=0; y < ske.height; y++){
        for(int x=0; x < ske.width; x++){
            if(test.pixels[y][x] == 255){
                den++;
            }
        }
    }

    double ratio = double(coincidencias)/double(den);
    cout << "Coincidencias con la vena temporal superior: " << coincidencias << endl;
    cout << "Ratio de coincidencias con número total de pixeles: " << ratio << endl;

    return 0;
}