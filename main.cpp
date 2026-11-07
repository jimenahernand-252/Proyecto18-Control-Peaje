#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <limits>
#include <cstdlib>

using namespace std;

const int MAX_CARRILES = 4;
const int MAX_COLA = 100;
const int MAX_REGISTROS = 500;
const int CATEGORIAS = 3;

struct Vehiculo {
    int id;
    string placa;
    int categoria;       // 1 motocicleta, 2 automovil, 3 transporte pesado
    int cicloIngreso;
    int ciclosEspera;
    bool activo;
};

struct Carril {
    int numero;
    bool habilitado;
    Vehiculo cola[MAX_COLA];
    int frente;
    int final;
    int cantidad;
    int atendidos;
    double recaudacion;
};

struct Registro {
    int id;
    string placa;
    string categoria;
    int carril;
    int cicloIngreso;
    int cicloAtencion;
    int espera;
    double tarifa;
};

Vehiculo pendientes[MAX_REGISTROS];
int totalPendientes = 0;
Registro historial[MAX_REGISTROS];
int totalHistorial = 0;
Carril carriles[MAX_CARRILES];
int cantidadCarriles = 3;
int siguienteId = 1;
int cicloActual = 0;
double tarifas[CATEGORIAS] = {10.00, 20.00, 35.00};
long totalVehiculos = 0;
double totalRecaudado = 0.0;
long totalCiclosEspera = 0;

string nombreCategoria(int categoria) {
    if (categoria == 1) return "Motocicleta";
    if (categoria == 2) return "Automovil";
    if (categoria == 3) return "Transporte pesado";
    return "Desconocida";
}

void inicializarCarriles() {
    for (int i = 0; i < MAX_CARRILES; i++) {
        carriles[i].numero = i + 1;
        carriles[i].habilitado = (i < cantidadCarriles);
        carriles[i].frente = 0;
        carriles[i].final = -1;
        carriles[i].cantidad = 0;
        carriles[i].atendidos = 0;
        carriles[i].recaudacion = 0.0;
    }
}

void limpiarEntrada() {
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

int leerEntero(string mensaje, int minimo, int maximo) {
    int valor;
    while (true) {
        cout << mensaje;
        if (cin >> valor && valor >= minimo && valor <= maximo) {
            limpiarEntrada();
            return valor;
        }
        cout << "Entrada invalida. Ingrese un numero entre " << minimo << " y " << maximo << ".\n";
        limpiarEntrada();
    }
}

double leerTarifa(string mensaje) {
    double valor;
    while (true) {
        cout << mensaje;
        if (cin >> valor && valor >= 0.01 && valor <= 10000) {
            limpiarEntrada();
            return valor;
        }
        cout << "Tarifa invalida. Debe ser mayor que cero.\n";
        limpiarEntrada();
    }
}

int buscarCarrilDisponible() {
    int elegido = -1;
    int menorCola = MAX_COLA + 1;
    for (int i = 0; i < cantidadCarriles; i++) {
        if (carriles[i].habilitado && carriles[i].cantidad < MAX_COLA &&
            carriles[i].cantidad < menorCola) {
            menorCola = carriles[i].cantidad;
            elegido = i;
        }
    }
    return elegido;
}

bool placaEnSistema(const string& placa) {
    for (int i = 0; i < totalPendientes; i++)
        if (pendientes[i].activo && pendientes[i].placa == placa) return true;
    for (int i = 0; i < totalHistorial; i++)
        if (historial[i].placa == placa && historial[i].cicloAtencion == -1) return true;
    return false;
}

void agregarVehiculo() {
    if (totalPendientes >= MAX_REGISTROS) {
        cout << "Capacidad de registros alcanzada.\n";
        return;
    }
    if (totalPendientes >= MAX_REGISTROS - 1) {
        cout << "No hay espacio para nuevos ingresos.\n";
        return;
    }
    string placa;
    cout << "Ingrese placa (maximo 12 caracteres, sin espacios): ";
    cin >> placa;
    if (placa.empty() || placa.length() > 12 || placaEnSistema(placa)) {
        cout << "Placa invalida o ya registrada en la cola.\n";
        return;
    }
    cout << "\nCategorias:\n1. Motocicleta (Q" << tarifas[0] << ")\n2. Automovil (Q" << tarifas[1]
         << ")\n3. Transporte pesado (Q" << tarifas[2] << ")\n";
    int categoria = leerEntero("Seleccione categoria: ", 1, 3);
    int carril = buscarCarrilDisponible();
    if (carril == -1) {
        cout << "Todos los carriles estan llenos o deshabilitados.\n";
        return;
    }
    Vehiculo v;
    v.id = siguienteId++;
    v.placa = placa;
    v.categoria = categoria;
    v.cicloIngreso = cicloActual;
    v.ciclosEspera = 0;
    v.activo = true;

    Carril &c = carriles[carril];
    if (c.cantidad == 0) c.frente = 0;
    c.final = (c.final + 1) % MAX_COLA;
    c.cola[c.final] = v;
    c.cantidad++;
    pendientes[totalPendientes++] = v;

    cout << "Vehiculo registrado. ID: " << v.id << ", asignado al carril " << c.numero << ".\n";
}

void registrarAtencion(int indiceCarril, const Vehiculo& v) {
    if (totalHistorial >= MAX_REGISTROS) {
        cout << "Historial lleno: no se puede completar la atencion.\n";
        return;
    }
    Registro r;
    r.id = v.id;
    r.placa = v.placa;
    r.categoria = nombreCategoria(v.categoria);
    r.carril = carriles[indiceCarril].numero;
    r.cicloIngreso = v.cicloIngreso;
    r.cicloAtencion = cicloActual;
    r.espera = cicloActual - v.cicloIngreso;
    r.tarifa = tarifas[v.categoria - 1];
    historial[totalHistorial++] = r;
    carriles[indiceCarril].atendidos++;
    carriles[indiceCarril].recaudacion += r.tarifa;
    totalVehiculos++;
    totalRecaudado += r.tarifa;
    totalCiclosEspera += r.espera;
}

void procesarCiclo() {
    cicloActual++;
    int atendidosEsteCiclo = 0;
    for (int i = 0; i < cantidadCarriles; i++) {
        if (totalHistorial >= MAX_REGISTROS) {
            cout << "Historial lleno; los vehiculos restantes permanecen en sus colas.\\n";
            break;
        }
        Carril &c = carriles[i];
        if (c.habilitado && c.cantidad > 0) {
            Vehiculo v = c.cola[c.frente];
            c.frente = (c.frente + 1) % MAX_COLA;
            c.cantidad--;
            registrarAtencion(i, v);
            atendidosEsteCiclo++;
            for (int j = 0; j < totalPendientes; j++)
                if (pendientes[j].id == v.id) pendientes[j].activo = false;
        }
    }
    cout << "Ciclo " << cicloActual << " procesado. Vehiculos atendidos: " << atendidosEsteCiclo << ".\n";
}

void mostrarColas() {
    cout << "\n--- ESTADO DE CARRILES ---\n";
    for (int i = 0; i < cantidadCarriles; i++) {
        Carril &c = carriles[i];
        cout << "Carril " << c.numero << " | " << (c.habilitado ? "Habilitado" : "Deshabilitado")
             << " | En cola: " << c.cantidad << " | Atendidos: " << c.atendidos
             << " | Recaudado: Q" << fixed << setprecision(2) << c.recaudacion << "\n";
        for (int j = 0; j < c.cantidad; j++) {
            int pos = (c.frente + j) % MAX_COLA;
            cout << "   ID " << c.cola[pos].id << " - " << c.cola[pos].placa
                 << " - " << nombreCategoria(c.cola[pos].categoria) << "\n";
        }
    }
}

void buscarPorPlaca() {
    string placa;
    cout << "Ingrese placa a buscar: ";
    cin >> placa;
    bool encontrado = false;
    for (int i = 0; i < totalPendientes; i++) {
        if (pendientes[i].activo && pendientes[i].placa == placa) {
            cout << "Vehiculo en espera. ID: " << pendientes[i].id
                 << ", categoria: " << nombreCategoria(pendientes[i].categoria) << "\n";
            encontrado = true;
        }
    }
    for (int i = 0; i < totalHistorial; i++) {
        if (historial[i].placa == placa) {
            cout << "Registro atendido. ID: " << historial[i].id << ", carril: " << historial[i].carril
                 << ", tarifa: Q" << fixed << setprecision(2) << historial[i].tarifa
                 << ", espera: " << historial[i].espera << " ciclos.\n";
            encontrado = true;
        }
    }
    if (!encontrado) cout << "No se encontro la placa.\n";
}

void buscarPorId() {
    int id = leerEntero("Ingrese ID: ", 1, 1000000);
    bool encontrado = false;
    for (int i = 0; i < totalPendientes; i++)
        if (pendientes[i].activo && pendientes[i].id == id) {
            cout << "ID " << id << " esta en espera: " << pendientes[i].placa << ".\n";
            encontrado = true;
        }
    for (int i = 0; i < totalHistorial; i++)
        if (historial[i].id == id) {
            cout << "ID " << id << " fue atendido; tarifa Q" << historial[i].tarifa << ".\n";
            encontrado = true;
        }
    if (!encontrado) cout << "ID no encontrado.\n";
}

void cambiarTarifas() {
    cout << "\n--- CONFIGURAR TARIFAS ---\n";
    for (int i = 0; i < CATEGORIAS; i++)
        tarifas[i] = leerTarifa("Nueva tarifa para " + nombreCategoria(i + 1) + ": Q");
    cout << "Tarifas actualizadas. Se aplican a los vehiculos atendidos posteriormente.\n";
}

void ordenarHistorialPorEspera() {
    for (int i = 0; i < totalHistorial - 1; i++) {
        for (int j = 0; j < totalHistorial - i - 1; j++) {
            if (historial[j].espera > historial[j + 1].espera) {
                Registro temp = historial[j];
                historial[j] = historial[j + 1];
                historial[j + 1] = temp;
            }
        }
    }
    cout << "Historial ordenado manualmente por tiempo de espera (menor a mayor).\n";
    for (int i = 0; i < totalHistorial; i++)
        cout << historial[i].placa << " | " << historial[i].espera << " ciclos | Q"
             << fixed << setprecision(2) << historial[i].tarifa << "\n";
}

void reporteGeneral() {
    cout << "\n--- REPORTE GENERAL ---\n";
    cout << "Ciclo actual: " << cicloActual << "\nVehiculos atendidos: " << totalVehiculos
         << "\nRecaudacion total: Q" << fixed << setprecision(2) << totalRecaudado << "\n";
    cout << "Vehiculos en espera: ";
    int enEspera = 0;
    for (int i = 0; i < totalPendientes; i++) if (pendientes[i].activo) enEspera++;
    cout << enEspera << "\n";
    double promedio = totalVehiculos > 0 ? (double)totalCiclosEspera / totalVehiculos : 0.0;
    cout << "Espera promedio: " << promedio << " ciclos.\n";
}

void reportePorCategoria() {
    int conteo[CATEGORIAS] = {0, 0, 0};
    double ingresos[CATEGORIAS] = {0.0, 0.0, 0.0};
    for (int i = 0; i < totalHistorial; i++) {
        for (int c = 0; c < CATEGORIAS; c++) {
            if (historial[i].categoria == nombreCategoria(c + 1)) {
                conteo[c]++;
                ingresos[c] += historial[i].tarifa;
            }
        }
    }
    cout << "\n--- REPORTE POR CATEGORIA ---\n";
    for (int c = 0; c < CATEGORIAS; c++)
        cout << nombreCategoria(c + 1) << ": " << conteo[c] << " vehiculos, Q"
             << fixed << setprecision(2) << ingresos[c] << "\n";
}

void reportePorCarril() {
    cout << "\n--- REPORTE POR CARRIL ---\n";
    for (int i = 0; i < cantidadCarriles; i++)
        cout << "Carril " << carriles[i].numero << ": " << carriles[i].atendidos
             << " atendidos, Q" << fixed << setprecision(2) << carriles[i].recaudacion
             << " recaudados, " << carriles[i].cantidad << " en espera.\n";
}

void guardarDatos() {
    ofstream archivo("peaje_datos.txt");
    if (!archivo) {
        cout << "No se pudo abrir el archivo para guardar.\n";
        return;
    }
    archivo << cicloActual << "\n" << siguienteId << "\n" << totalHistorial << "\n";
    for (int i = 0; i < totalHistorial; i++)
        archivo << historial[i].id << "|" << historial[i].placa << "|" << historial[i].categoria
                << "|" << historial[i].carril << "|" << historial[i].cicloIngreso << "|"
                << historial[i].cicloAtencion << "|" << historial[i].espera << "|"
                << historial[i].tarifa << "\n";
    archivo.close();
    cout << "Historial guardado en peaje_datos.txt.\n";
}

void cargarDatos() {
    ifstream archivo("peaje_datos.txt");
    if (!archivo) {
        cout << "No existe archivo previo. Se iniciara una sesion nueva.\n";
        return;
    }
    // Se carga el historial. Las colas pendientes comienzan vacias para evitar
    // reconstruir incorrectamente estados que no fueron guardados.
    int ciclo, id, total;
    if (!(archivo >> ciclo >> id >> total) || total < 0 || total > MAX_REGISTROS) {
        cout << "Archivo con formato invalido. No se cargaron datos.\n";
        return;
    }
    archivo.ignore(numeric_limits<streamsize>::max(), '\n');
    Registro temporal[MAX_REGISTROS];
    bool correcto = true;
    for (int i = 0; i < total; i++) {
        string linea;
        if (!getline(archivo, linea)) { correcto = false; break; }
        string campos[8];
        int inicio = 0, campo = 0;
        for (int p = 0; p <= (int)linea.size(); p++) {
            if (p == (int)linea.size() || linea[p] == '|') {
                if (campo < 8) campos[campo++] = linea.substr(inicio, p - inicio);
                inicio = p + 1;
            }
        }
        if (campo != 8) { correcto = false; break; }
        try {
            temporal[i].id = stoi(campos[0]);
            temporal[i].placa = campos[1];
            temporal[i].categoria = campos[2];
            temporal[i].carril = stoi(campos[3]);
            temporal[i].cicloIngreso = stoi(campos[4]);
            temporal[i].cicloAtencion = stoi(campos[5]);
            temporal[i].espera = stoi(campos[6]);
            temporal[i].tarifa = stod(campos[7]);
        } catch (...) { correcto = false; break; }
    }
    if (!correcto) {
        cout << "Error leyendo registros. No se modifico la informacion actual.\n";
        return;
    }
    totalHistorial = total;
    for (int i = 0; i < total; i++) historial[i] = temporal[i];
    cicloActual = ciclo;
    siguienteId = id;
    totalVehiculos = totalHistorial;
    totalRecaudado = 0;
    totalCiclosEspera = 0;
    for (int i = 0; i < totalHistorial; i++) {
        totalRecaudado += historial[i].tarifa;
        totalCiclosEspera += historial[i].espera;
    }
    cout << "Se cargaron " << totalHistorial << " registros historicos.\n";
}

void menuReportes() {
    int opcion;
    do {
        cout << "\n--- REPORTES ---\n1. General\n2. Por categoria\n3. Por carril\n0. Regresar\n";
        opcion = leerEntero("Opcion: ", 0, 3);
        if (opcion == 1) reporteGeneral();
        else if (opcion == 2) reportePorCategoria();
        else if (opcion == 3) reportePorCarril();
    } while (opcion != 0);
}

int main() {
    inicializarCarriles();
    cargarDatos();
    int opcion;
    do {
        cout << "\n====================================\n";
        cout << "       SISTEMA DE CONTROL DE PEAJE\n";
        cout << "====================================\n";
        cout << "Ciclo actual: " << cicloActual << "\n";
        cout << "1. Registrar ingreso de vehiculo\n";
        cout << "2. Procesar siguiente ciclo\n";
        cout << "3. Consultar carriles y colas\n";
        cout << "4. Buscar por placa\n";
        cout << "5. Buscar por ID\n";
        cout << "6. Configurar tarifas\n";
        cout << "7. Ordenar historial por espera\n";
        cout << "8. Consultar reportes\n";
        cout << "9. Guardar datos\n";
        cout << "0. Guardar y salir\n";
        opcion = leerEntero("Seleccione una opcion: ", 0, 9);
        switch (opcion) {
            case 1: agregarVehiculo(); break;
            case 2: procesarCiclo(); break;
            case 3: mostrarColas(); break;
            case 4: buscarPorPlaca(); break;
            case 5: buscarPorId(); break;
            case 6: cambiarTarifas(); break;
            case 7: ordenarHistorialPorEspera(); break;
            case 8: menuReportes(); break;
            case 9: guardarDatos(); break;
            case 0: guardarDatos(); cout << "Saliendo del sistema.\n"; break;
        }
    } while (opcion != 0);
    return 0;
}
