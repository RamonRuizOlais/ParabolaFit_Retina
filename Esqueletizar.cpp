#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <filesystem>
#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <set>
#include <cmath>
#include <chrono>
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

int maxElement(const Image& img) {
    int largestElement = img.pixels[0][0]; // Suponemos que el primer elemento es el más grande

    // Iteramos sobre la matriz de píxeles para encontrar el elemento más grande
    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            if (img.pixels[y][x] > largestElement) {
                largestElement = img.pixels[y][x];
            }
        }
    }

    return largestElement;
}

int minElement(const Image& img){
    int smallestElement = img.pixels[0][0];  // Suponemos que el primer elemento es el más pequeño

    // Iteramos sobre la matriz para encontrar el elemento más pequeño
    for (int y = 0; y < img.height; ++y){
        for (int x = 0; x < img.width; ++x){
            if(img.pixels[y][x] < smallestElement){
                smallestElement = img.pixels[y][x];
            }
        }
    }

    return smallestElement;
}

// Función para aplicar un ajuste lineal a una imagen
Image applyLinearAdjustment(const Image& input, int y1, int y2, int x1, int x2) {
    Image output = input;

    // Iterar sobre cada píxel de la imagen de salida
    for (int y = 0; y < input.height; ++y) {
        for (int x = 0; x < input.width; ++x) {
            // Aplicar la fórmula de ajuste lineal a cada píxel
            output.pixels[y][x] = int(double(y2 - y1) / double(x2 - x1) * (output.pixels[y][x] - x1) + y1);

            // Asegurarse de que los valores estén en el rango [0, 255]
            if (output.pixels[y][x] < 0) {
                output.pixels[y][x] = 0;
            } else if (output.pixels[y][x] > 255) {
                output.pixels[y][x] = 255;
            }
        }
    }

    return output;
}

// Función para extender la matriz con ceros
vector<vector<double>> extenderMatriz(const vector<vector<double>>& matriz) {
    int rows = matriz.size();
    int cols = matriz[0].size();

    // Extender la matriz con ceros
    vector<vector<double>> extendedMatrix(rows + 6, vector<double>(cols + 6, 0));

    // Copiar la matriz original en el centro de la matriz extendida
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            extendedMatrix[i + 3][j + 3] = matriz[i][j];
        }
    }

    return extendedMatrix;
}

// Función para aplicar una rotación de 15 grados a la matriz
vector<vector<double>> rotarMatriz(const vector<vector<double>>& matriz, double theta) {
    int rows = matriz.size();
    int cols = matriz[0].size();

    // Definir el ángulo de rotación en radianes
    double angle = theta * M_PI / 180.0;

    // Inicializar la matriz rotada con ceros
    vector<vector<double>> rotatedMatrix(rows, vector<double>(cols, 0));

    // Calcular el centro de la matriz
    double centerX = cols / 2.0;
    double centerY = rows / 2.0;

    // Aplicar la rotación a cada punto de la matriz original
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            // Calcular la posición rotada del punto
            double rotatedX = centerX + (x - centerX) * cos(angle) - (y - centerY) * sin(angle);
            double rotatedY = centerY + (x - centerX) * sin(angle) + (y - centerY) * cos(angle);

            // Redondear a la posición entera más cercana
            int newX = round(rotatedX);
            int newY = round(rotatedY);

            // Asignar el valor del punto original a la posición rotada si está dentro de la matriz
            if (newX >= 0 && newX < cols && newY >= 0 && newY < rows) {
                rotatedMatrix[y][x] = matriz[newY][newX];
            }
        }
    }

    return rotatedMatrix;
}

double Gaussiana(int x, double sigma){
    return (1/(sigma*sqrt(2*M_PI)))*exp(-0.5*(pow(x,2)/pow(sigma,2)));
}

// Función para realizar la convolución de una imagen con un kernel dado
vector<vector<int>> convolucionar(const Image& input, const vector<vector<double>>& kernel, const Image& mask) {
    int kernelHeight = kernel.size();
    int kernelWidth = kernel[0].size();
    int imageHeight = input.height;
    int imageWidth = input.width;

    int r = int(float(kernelHeight/2));
    int c = int(float(kernelWidth/2));

    vector<vector<int>> result(imageHeight, vector<int>(imageWidth, 0));

    // Iterar sobre cada píxel de la imagen
    for (int y = r; y < imageHeight - r; y++) {
        for (int x = c; x < imageWidth - c; x++) {
            double sum = 0;

            // Iterar sobre cada píxel del kernel
            for (int ky = 0; ky < kernelHeight; ky++) {
                for (int kx = 0; kx < kernelWidth; kx++) {
                    // Coordenadas en la imagen
                    int imgX = x - (c) + kx;
                    int imgY = y - r + ky;

                    // Verificar los límites de la imagen
                    if (imgX >= 0 && imgX < imageWidth && imgY >= 0 && imgY < imageHeight) {
                        if(mask.pixels[y][x] == 255){
                            sum += input.pixels[imgY][imgX] * kernel[ky][kx];
                        }
                    }
                }
            }

            // Asignar el valor resultante al píxel correspondiente
            result[y][x] = int(sum);
        }
    }

    return result;
}

double calcularMedia(const vector<double>& vec) {
    double suma = 0.0;
    for (const auto& elemento : vec) {
        suma += elemento;
    }
    return suma / vec.size();
}

Image FiltroGaussiano(const Image& input, int T, int L, int sigma, const Image& mask){

    Image output = input;

    for(int y = 0; y < input.height; y++){
        for(int x = 0; x < input.width; x++){
            output.pixels[y][x] = 0;
        }
    }

    double theta;
    vector<double> temp;
    vector<vector<double>> filtro;
    vector<vector<int>> convolucion;
    vector<vector<double>> extension;
    vector<vector<double>> rotacion;
    

    for(int i=(-T/2); i<(T/2)+1; i++){
        temp.push_back(Gaussiana(0, sigma) - Gaussiana(i, sigma));
    }

    double media = calcularMedia(temp);

    // Restar la media al vector temp
    for (auto& elemento : temp) {
        elemento -= media;
    }

    for(int i=0; i<L; i++){
        filtro.push_back(temp);
    }
    
    extension = extenderMatriz(filtro);

    for(double theta = 0; theta < 180; theta+=15){
        rotacion = rotarMatriz(extension, theta);
       
        convolucion = convolucionar(input, rotacion, mask);
        if(theta == 0){

        }

        for(int y=0; y < output.height; ++y){
            for(int x=0; x < output.width; ++x){
                if(convolucion[y][x] > output.pixels[y][x]){
                    output.pixels[y][x] = convolucion[y][x];
                }
            }
        } 

        
    }

    return output;
}

Image RATS(const Image& input){
    Image dx = input;
    Image dy = input;
    Image dz = input;
    Image output = input;

    for(int y = 1; y<input.height - 1; y++){
        for(int x = 1; x<input.width - 1; x++){
            dx.pixels[y][x] = (input.pixels[y][x+1] - input.pixels[y][x-1]);
            dy.pixels[y][x] = (input.pixels[y+1][x] - input.pixels[y-1][x]);
        }
    }

    for(int y=0; y<input.height; y++){
        for(int x=0; x<input.width; x++){
            dz.pixels[y][x] = max(abs(dx.pixels[y][x]), abs(dy.pixels[y][x]));
        }
    }

    int sd = 0;
    for(int y = 0; y < input.height; y++){
        for(int x=0; x < input.width; x++){
            sd += dz.pixels[y][x];
        }
    }

    int sdf = 0;

    for(int y = 0; y < input.height; y++){
        for(int x = 0; x < input.width; x++){
            sdf += dz.pixels[y][x]*input.pixels[y][x];
        }
    }

    double t = sdf/sd;

    for(int y = 0; y < input.height; y++){
        for(int x = 0; x < input.width; x++){
            if(input.pixels[y][x] < t){
                output.pixels[y][x] = 0;
            }
            else{
                output.pixels[y][x] = 255;
            }
        }
    }

    return output;
}

Image MayorComponenteConexa(const Image& input){

    Image test = input;
    Image output = input;

    queue<pair<int,int>> cola;
    set<pair<int,int>> conjuntoPosicion;
    pair <int,int> frente;
    vector<set<pair<int,int>>> totalConjuntos;

    for(int y = 1; y < test.height - 1; ++y){

        for(int x = 1; x < test.width - 1; ++x){

            if(test.pixels[y][x] == 255){

                conjuntoPosicion.clear();
                cola.push(make_pair(y,x));

                while(!cola.empty()){
                    frente = cola.front();
                    if(frente.first > 1 && frente.first < test.height - 1 && frente.second > 1 && frente.second < test.width - 1){
                        if(test.pixels[frente.first][frente.second + 1]){
                            cola.push(make_pair(frente.first, frente.second + 1));
                            test.pixels[frente.first][frente.second + 1] = 0;
                        }
                        if(test.pixels[frente.first][frente.second - 1]){
                            cola.push(make_pair(frente.first, frente.second - 1));
                            test.pixels[frente.first][frente.second - 1] = 0;
                        }
                        if(test.pixels[frente.first + 1][frente.second]){
                            cola.push(make_pair(frente.first + 1, frente.second));
                            test.pixels[frente.first + 1][frente.second] = 0;
                        }
                        if(test.pixels[frente.first - 1][frente.second]){
                            cola.push(make_pair(frente.first - 1, frente.second));
                            test.pixels[frente.first - 1][frente.second] = 0;
                        }       
                    }

                    test.pixels[frente.first][frente.second] = 0;
                    conjuntoPosicion.insert(frente);
                    cola.pop();           
                }

                totalConjuntos.push_back(conjuntoPosicion);

            }
        }
    }

    int longitudMaxima = 0;
    set<pair<int,int>> conjuntoMaximo; 

    for(int i = 0; i < totalConjuntos.size(); i++){
        if(totalConjuntos[i].size() > longitudMaxima){
            conjuntoMaximo = totalConjuntos[i];
            longitudMaxima = totalConjuntos[i].size();
        }
    }

    for (int y = 0; y < output.height; ++y) {
        for (int x = 0; x < output.width; ++x) {
            output.pixels[y][x] = 0; // Establece el valor del píxel en negro en la imagen de salida
        }
    }

    // Después de encontrar la componente conexa más grande
    for (auto pix : conjuntoMaximo) {
        int y = pix.first;
        int x = pix.second;
        output.pixels[y][x] = 255; // Establece el valor del píxel en blanco en la imagen de salida
    }



    return output;

}

int neighbours2_6(const Image& input, int y, int x){
    int k = 0;
    
    if(input.pixels[y+1][x] == 255){
        k += 1;
    }
    if(input.pixels[y - 1][x] == 255){
        k += 1;
    }
    if(input.pixels[y][x + 1] == 255){
        k += 1;
    }
    if(input.pixels[y][x - 1] == 255){
        k += 1;
    }
    if(input.pixels[y+1][x+1] == 255){
        k += 1;
    }
    if(input.pixels[y-1][x-1] == 255){
        k += 1;
    }
    if(input.pixels[y+1][x-1] == 255){
        k += 1;
    }
    if(input.pixels[y-1][x+1] == 255){
        k += 1;
    } 

    return k;
}

int neighboursChanges(const Image& input, int y, int x){

    int neighboursValues[8];
    int changes = 0;
    neighboursValues[0] = (input.pixels[y-1][x]);
    neighboursValues[1] = (input.pixels[y-1][x+1]);
    neighboursValues[2] = (input.pixels[y][x+1]);
    neighboursValues[3] = (input.pixels[y+1][x+1]);
    neighboursValues[4] = (input.pixels[y+1][x]);
    neighboursValues[5] = (input.pixels[y+1][x-1]);
    neighboursValues[6] = (input.pixels[y][x-1]);
    neighboursValues[7] = (input.pixels[y-1][x-1]);

    for(int i = 0; i < 7; i++){
        if(neighboursValues[i] == 0 && neighboursValues[i+1] == 255){
            changes += 1;
        }
    }

    // Verificar el cambio entre el último y el primer vecino
    if (neighboursValues[7] == 0 && neighboursValues[0] == 255) {
        changes += 1;
    }

    return changes;
}

int condicionExtra(const Image&input, int y, int x){
    int sum = 0;

    sum = input.pixels[y-1][x]*pow(2,0) + input.pixels[y-1][x+1]*pow(2,1) + input.pixels[y][x+1]*pow(2,2) + input.pixels[y+1][x+1]*pow(2,3) + input.pixels[y+1][x]*pow(2,4) + input.pixels[y+1][x-1]*pow(2,5) + input.pixels[y][x-1]*pow(2,6) + input.pixels[y-1][x-1]*pow(2,7);

    return int(float(sum/255));
}

Image ZhangSuen(const Image& input){

    Image skeleton = input;
    vector<pair<int,int>> position;
    position.push_back(make_pair(1,1));

    while(position.size() > 0){

        position.clear();
        for(int y = 1; y < skeleton.height - 1; ++y){
            for(int x = 1; x < skeleton.width - 1 ; ++x){
                if(skeleton.pixels[y][x] == 255){
                    //cout << "vecinos = " << neighbours2_6(skeleton, y, x) << endl;
                    if(2 <= neighbours2_6(skeleton, y, x) && neighbours2_6(skeleton, y, x) <= 6){
                        //cout << "cambios = " << neighboursChanges(skeleton, y, x) << endl;
                        if(neighboursChanges(skeleton, y, x) == 1){
                            if(skeleton.pixels[y-1][x] == 0|| skeleton.pixels[y][x+1] == 0 || skeleton.pixels[y+1][x] == 0){
                                if(skeleton.pixels[y][x+1] == 0 || skeleton.pixels[y+1][x] == 0 || skeleton.pixels[y][x-1] == 0){
                                   position.push_back(make_pair(y,x)); 
                                }
                            }
                        }
                    }
                }
            }
        }

        for (const auto& pos : position) {
            int y = pos.first;
            int x = pos.second;
            skeleton.pixels[y][x] = 0; // Establecer el píxel en negro
        }
    

        
        position.clear();

        for(int y = 1; y < skeleton.height - 1; ++y){
            for(int x = 1; x < skeleton.width - 1; ++x){
                if(skeleton.pixels[y][x] == 255){
                    if(2 <= neighbours2_6(skeleton, y, x) && neighbours2_6(skeleton, y, x) <= 6){
                        if(neighboursChanges(skeleton, y, x) == 1){
                            if(skeleton.pixels[y - 1][x] == 0 || skeleton.pixels[y][x+1] == 0 || skeleton.pixels[y][x-1] == 0){
                                if(skeleton.pixels[y -1][x] == 0 || skeleton.pixels[y+1][x] == 0 || skeleton.pixels[y][x-1] == 0){
                                    position.push_back(make_pair(y,x));    
                                }    
                            }
                        }
                    }
                }
            }
        }

        for (const auto& pos : position) {
            int y = pos.first;
            int x = pos.second;
            skeleton.pixels[y][x] = 0; // Establecer el píxel en negro
        }

    }

    return skeleton;
    
}

Image ZhangSuen2(const Image& input){

    Image skeleton = input;
    int interruptor = 1;
    while(interruptor == 1){
        interruptor = 0;
        for(int y = 1; y < skeleton.height - 1; ++y){
            for(int x = 1; x < skeleton.width - 1 ; ++x){
                if(skeleton.pixels[y][x] == 255){
                    //cout << "vecinos = " << neighbours2_6(skeleton, y, x) << endl;
                    if(2 <= neighbours2_6(skeleton, y, x) && neighbours2_6(skeleton, y, x) <= 6){
                        //cout << "cambios = " << neighboursChanges(skeleton, y, x) << endl;
                        if(neighboursChanges(skeleton, y, x) == 1 || condicionExtra(skeleton, y, x) == 65 || condicionExtra(skeleton, y, x) == 5 || condicionExtra(skeleton, y, x) == 20 || condicionExtra(skeleton, y, x) == 80 || condicionExtra(skeleton, y, x) == 97 || condicionExtra (skeleton, y, x)== 133 || condicionExtra(skeleton, y, x) == 52 || condicionExtra(skeleton, y, x) == 208 || condicionExtra(skeleton, y, x) == 67 || condicionExtra(skeleton, y, x) == 13 || condicionExtra(skeleton, y, x) == 22 || condicionExtra(skeleton, y, x) == 88 || condicionExtra(skeleton, y, x) == 99 || condicionExtra(skeleton, y, x) == 141 || condicionExtra(skeleton, y, x) == 54 || condicionExtra(skeleton, y, x) ==216){
                            if(skeleton.pixels[y-1][x] == 0|| skeleton.pixels[y][x+1] == 0 || skeleton.pixels[y+1][x] == 0){
                                if(skeleton.pixels[y][x+1] == 0 || skeleton.pixels[y+1][x] == 0 || skeleton.pixels[y][x-1] == 0){
                                   skeleton.pixels[y][x] = 0;
                                   interruptor = 1; 
                                }
                            }
                        }
                    }
                }
            }
        }
    
        for(int y = 1; y < skeleton.height - 1; ++y){
            for(int x = 1; x < skeleton.width - 1; ++x){
                if(skeleton.pixels[y][x] == 255){
                    if(2 <= neighbours2_6(skeleton, y, x) && neighbours2_6(skeleton, y, x) <= 6){
                        if(neighboursChanges(skeleton, y, x) == 1 || condicionExtra(skeleton, y, x) == 65 || condicionExtra(skeleton, y, x) == 5 || condicionExtra(skeleton, y, x) == 20 || condicionExtra(skeleton, y, x) == 80 || condicionExtra(skeleton, y, x) == 97 || condicionExtra (skeleton, y, x)== 133 || condicionExtra(skeleton, y, x) == 52 || condicionExtra(skeleton, y, x) == 208 || condicionExtra(skeleton, y, x) == 67 || condicionExtra(skeleton, y, x) == 13 || condicionExtra(skeleton, y, x) == 22 || condicionExtra(skeleton, y, x) == 88 || condicionExtra(skeleton, y, x) == 99 || condicionExtra(skeleton, y, x) == 141 || condicionExtra(skeleton, y, x) == 54 || condicionExtra(skeleton, y, x) ==216){
                            if(skeleton.pixels[y - 1][x] == 0 || skeleton.pixels[y][x+1] == 0 || skeleton.pixels[y][x-1] == 0){
                                if(skeleton.pixels[y -1][x] == 0 || skeleton.pixels[y+1][x] == 0 || skeleton.pixels[y][x-1] == 0){
                                    skeleton.pixels[y][x] = 0;
                                    interruptor = 1;    
                                }    
                            }
                        }
                    }
                }
            }
        }

    }

    return skeleton;
    
}

int main(){

    string folderPath = "/ImagenesBio/DRIVE_pgm/";
    string outputFilename = "/ImagenesBio/resultados/";

    int numImages = 41;

    for(int i = 21; i < numImages; i++){
        stringstream ss;
        stringstream ss_mask;
        
        ss << folderPath << i << "_test.pgm";
        ss_mask << folderPath << i << "_mask.pgm";

        string imagePath = ss.str();
        string maskPath = ss_mask.str();
        
        Image img = loadPGM(imagePath);
        Image img_mask = loadPGM(maskPath);

        Image gaussiano = FiltroGaussiano(img, 13, 9, 2, img_mask);  

        int minValue = minElement(gaussiano);
        int maxValue = maxElement(gaussiano);

        Image gaussiano_ajustado = applyLinearAdjustment(gaussiano, 0, 255, minValue, maxValue);

        Image bin = RATS(gaussiano_ajustado);      

        Image conexo = MayorComponenteConexa(bin);

        Image ske = ZhangSuen(conexo);

        Image ske2 = ZhangSuen2(ske);

        string gaussianoFile = outputFilename + to_string(i) + "_gauss.pgm";

        string conFile = outputFilename + to_string(i) + "_con.pgm";

        string skeFile = outputFilename + to_string(i) + "_ske.pgm";
        
        savePGM(gaussiano_ajustado, gaussianoFile);
        savePGM(conexo, conFile);
        savePGM(ske2, skeFile);
        
        cout << "Transformacion " << to_string(i) << " guardada correctamente." << endl;

    }

    return 0;
}