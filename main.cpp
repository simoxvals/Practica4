#include <iostream>
#include <map>
#include <vector>
#include <limits>
#include <queue>
#include <string>
#include <fstream>

using namespace std;

class Enrutador
{
public:
    string nombre;
    map<string, int> tablaEnrutamiento;
    map<string, string> siguienteSalto;
    Enrutador() = default;
    Enrutador(const string& nombre) : nombre(nombre) {}

    void actualizarTablaDeEnrutamiento(const string& destino, int costo, const string& salto) {
        tablaEnrutamiento[destino] = costo;
        siguienteSalto[destino] = salto;
    }
};

class Red
{
    map<string, Enrutador> enrutadores;
    map<pair<string, string>, int> enlaces;

public:

    void agregarEnrutador(const string& nombre) {
        if (enrutadores.find(nombre) == enrutadores.end()) {
            enrutadores.emplace(nombre, Enrutador(nombre));
            cout << "Enrutador " << nombre << " agregado exitosamente.\n";
        } else {
            cout << "El enrutador " << nombre << " ya existe.\n";
        }
    }

    void removerEnrutador(const string& nombre) {
        auto it = enrutadores.find(nombre);
        if (it != enrutadores.end()) {
            for (auto enlace = enlaces.begin(); enlace != enlaces.end(); ) {
                if (enlace->first.first == nombre || enlace->first.second == nombre) {
                    enlace = enlaces.erase(enlace);
                } else {
                    ++enlace;
                }
            }
            enrutadores.erase(it);
            cout << "Enrutador " << nombre << " removido exitosamente.\n";
        } else {
            cout << "El enrutador " << nombre << " no se encuentra en la red.\n";
        }
    }

    void agregarEnlace(const string& r1, const string& r2, int costo) {
        enlaces[{r1, r2}] = costo;
        enlaces[{r2, r1}] = costo;
    }

    void removerEnlace(const string& r1, const string& r2) {
        enlaces.erase({r1, r2});
        enlaces.erase({r2, r1});
    }

    void cargarTopologia(const std::string& archivo) {
        ifstream archivoEntrada(archivo);
        string r1, r2;
        int costo;
        while (archivoEntrada >> r1 >> r2 >> costo) {
            agregarEnrutador(r1);
            agregarEnrutador(r2);
            agregarEnlace(r1, r2, costo);
        }
        archivoEntrada.close();
        actualizarTablasDeEnrutamiento();
    }

    void actualizarTablasDeEnrutamiento() {
        for (auto& [nombre, enrutador] : enrutadores) {
            ejecutarBusqueda(nombre);
        }
    }

    void ejecutarBusqueda(const std::string& origen) {
        map<string, int> distancias;
        map<string, string> anteriores;
        for (auto& [nombre, _] : enrutadores) {
            distancias[nombre] = numeric_limits<int>::max();
            anteriores[nombre] = "";
        }
        distancias[origen] = 0;

        using pii = pair<int, string>;
        priority_queue<pii, vector<pii>, greater<pii>> colaDePrioridad;
        colaDePrioridad.push({0, origen});

        while (!colaDePrioridad.empty()) {
            auto [d, u] = colaDePrioridad.top();
            colaDePrioridad.pop();

            if (d > distancias[u]) continue;

            for (auto& [enlace, costo] : enlaces) {
                if (enlace.first == u) {
                    string v = enlace.second;
                    int alternativa = distancias[u] + costo;
                    if (alternativa < distancias[v]) {
                        distancias[v] = alternativa;
                        anteriores[v] = u;
                        colaDePrioridad.push({alternativa, v});
                    }
                }
            }
        }

        for (auto& [destino, d] : distancias) {
            if (d == numeric_limits<int>::max()) continue;
            string siguiente = destino;
            while (anteriores[siguiente] != origen && !anteriores[siguiente].empty()) {
                siguiente = anteriores[siguiente];
            }
            enrutadores[origen].actualizarTablaDeEnrutamiento(destino, d, siguiente);
        }
    }

    int obtenerCosto(const string& origen, const string& destino) {
        if (enrutadores[origen].tablaEnrutamiento.find(destino) != enrutadores[origen].tablaEnrutamiento.end()) {
            return enrutadores[origen].tablaEnrutamiento[destino];
        }
        return -1;
    }

    vector<string> obtenerCamino(const string& origen, const string& destino) {
        vector<string> camino;
        string actual = origen;
        while (actual != destino && !actual.empty()) {
            camino.push_back(actual);
            actual = enrutadores[actual].siguienteSalto[destino];
        }
        if (actual == destino) camino.push_back(destino);
        return camino;
    }
};

void crearArchivoDeTopologia(const string& nombreArchivo) {
    ofstream archivo(nombreArchivo);
    archivo << "A B 5\n";
    archivo << "A C 10\n";
    archivo << "B C 2\n";
    archivo << "B D 3\n";
    archivo << "C D 1\n";
    archivo.close();
}

int main() {
    Red red;

    const string archivoTopologia = "topologia.txt";
    ifstream file(archivoTopologia);
    if (!file) {
        crearArchivoDeTopologia(archivoTopologia);
    }
    file.close();

    red.cargarTopologia(archivoTopologia);

    int opcion;
    do {
        cout << "\nMenu de opciones:\n";
        cout << "1. Agregar enrutador\n";
        cout << "2. Remover enrutador\n";
        cout << "3. Obtener costo de paquete\n";
        cout << "4. Obtener camino\n";
        cout << "5. Salir\n";
        cout << "Ingrese una opcion: ";
        cin >> opcion;

        switch (opcion) {
        case 1: {
            string nombreEnrutador;
            cout << "Ingrese el nombre del enrutador a agregar: ";
            cin >> nombreEnrutador;
            red.agregarEnrutador(nombreEnrutador);
            break;
        }
        case 2: {
            string nombreEnrutador;
            cout << "Ingrese el nombre del enrutador a remover: ";
            cin >> nombreEnrutador;
            red.removerEnrutador(nombreEnrutador);
            break;
        }
        case 3: {
            string origen, destino;
            cout << "Ingrese el enrutador origen: ";
            cin >> origen;
            cout << "Ingrese el enrutador destino: ";
            cin >> destino;
            int costo = red.obtenerCosto(origen, destino);
            if (costo != -1) {
                cout << "Costo de enviar paquete de " << origen << " a " << destino << ": " << costo << endl;
            } else {
                cout << "No hay ruta disponible entre " << origen << " y " << destino << "." << endl;
            }
            break;
        }
        case 4: {
            string origen, destino;
            cout << "Ingrese el enrutador origen: ";
            cin >> origen;
            cout << "Ingrese el enrutador destino: ";
            cin >> destino;
            auto camino = red.obtenerCamino(origen, destino);
            if (camino.empty()) {
                cout << "No hay ruta disponible entre " << origen << " y " << destino << "." <<endl;
            } else {
                cout << "Camino desde " << origen << " a " << destino << ": ";
                for (const auto& enrutador : camino) {
                    cout << enrutador << " ";
                }
                cout << endl;
            }
            break;
        }
        case 5:
            cout << "Saliendo del programa...\n";
            break;
        default:
            cout << "Opción no válida. Intente nuevamente.\n";
        }
    } while (opcion != 5);

    return 0;
}
